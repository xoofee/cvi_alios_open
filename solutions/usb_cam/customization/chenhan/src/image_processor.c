/*
 * Real-time Image Processor for USB Camera
 * Acquires images from VPSS and calculates brightness statistics
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <aos/kernel.h>
#include "cvi_vpss.h"
#include "cvi_param.h"
#include "cvi_type.h"
#include "cvi_comm_vpss.h"
#include "common_vi.h"

#define TAG "image_processor"

// Image processing configuration
#define IMAGE_PROCESSOR_VPSS_GRP 0    // Use main VPSS group    change to 1 if you want to use the IR camera
#define IMAGE_PROCESSOR_VPSS_CHN 0    // Use main VPSS channel
#define IMAGE_PROCESSOR_TIMEOUT 1000  // 1 second timeout

// Global variables
static pthread_t g_image_processor_thread;
static int g_image_processor_running = 0;
static pthread_mutex_t g_image_processor_mutex;

// Statistics
static int g_total_frames_processed = 0;
static float g_total_brightness_sum = 0.0f;
static float g_min_brightness = 255.0f;
static float g_max_brightness = 0.0f;

/**
 * Calculate mean brightness of Y channel in YUYV format
 */
float calculate_y_brightness(uint8_t *yuyv_data, int width, int height)
{
    uint64_t brightness_sum = 0;
    int pixel_count = 0;
    
    // Process YUYV format: Y0 U0 Y1 V0 Y2 U1 Y3 V1 ...
    // We only care about Y (luminance) values at even indices
    for (int i = 0; i < width * height * 2; i += 2) {
        brightness_sum += yuyv_data[i];  // Y value
        pixel_count++;
    }
    
    return (float)brightness_sum / pixel_count;
}

/**
 * Calculate mean brightness of Y channel in NV21 format
 */
float calculate_y_brightness_nv21(uint8_t *nv21_data, int width, int height)
{
    uint64_t brightness_sum = 0;
    int pixel_count = width * height;
    
    // NV21 format: Y plane first, then UV plane
    // Y values are in the first width*height bytes
    for (int i = 0; i < pixel_count; i++) {
        brightness_sum += nv21_data[i];
    }
    
    return (float)brightness_sum / pixel_count;
}

/**
 * Process a single frame and calculate brightness
 */
void process_frame(VIDEO_FRAME_INFO_S *video_frame, VPSS_CHN_ATTR_S *chn_attr)
{
    float brightness = 0.0f;
    
    // Map physical address to virtual address
    video_frame->stVFrame.pu8VirAddr[0] = (uint8_t*)video_frame->stVFrame.u64PhyAddr[0];
    
    // Calculate brightness based on pixel format
    switch (chn_attr->enPixelFormat) {
        case PIXEL_FORMAT_YUYV:
            brightness = calculate_y_brightness(
                video_frame->stVFrame.pu8VirAddr[0],
                chn_attr->u32Width,
                chn_attr->u32Height
            );
            break;
            
        case PIXEL_FORMAT_NV21:
            brightness = calculate_y_brightness_nv21(
                video_frame->stVFrame.pu8VirAddr[0],
                chn_attr->u32Width,
                chn_attr->u32Height
            );
            break;
            
        default:
            printf("[%s] Unsupported pixel format: %d\n", TAG, chn_attr->enPixelFormat);
            return;
    }
    
    // Update statistics
    g_total_frames_processed++;
    g_total_brightness_sum += brightness;
    
    if (brightness < g_min_brightness) {
        g_min_brightness = brightness;
    }
    if (brightness > g_max_brightness) {
        g_max_brightness = brightness;
    }
    
    // Print brightness information
    printf("[%s] Frame %d: Brightness=%.2f, Avg=%.2f, Min=%.2f, Max=%.2f\n",
           TAG, g_total_frames_processed, brightness,
           g_total_brightness_sum / g_total_frames_processed,
           g_min_brightness, g_max_brightness);
    
    // Unmap virtual address
    video_frame->stVFrame.pu8VirAddr[0] = NULL;
}

/**
 * Main image processing thread
 */
void* image_processor_thread(void* arg)
{
    CVI_S32 ret = CVI_SUCCESS;
    VIDEO_FRAME_INFO_S video_frame;
    VPSS_CHN_ATTR_S chn_attr;
    
    printf("[%s] Image processor thread started!\n", TAG);
    
    // Get VPSS channel attributes
    ret = CVI_VPSS_GetChnAttr(IMAGE_PROCESSOR_VPSS_GRP, IMAGE_PROCESSOR_VPSS_CHN, &chn_attr);
    if (ret != CVI_SUCCESS) {
        printf("[%s] Failed to get VPSS channel attributes: %d\n", TAG, ret);
        return NULL;
    }
    
    printf("[%s] VPSS Channel: %dx%d, Format: %d\n", TAG, 
           chn_attr.u32Width, chn_attr.u32Height, chn_attr.enPixelFormat);
    
    while (g_image_processor_running) {
        pthread_mutex_lock(&g_image_processor_mutex);
        
        // Get frame from VPSS
        ret = CVI_VPSS_GetChnFrame(IMAGE_PROCESSOR_VPSS_GRP, IMAGE_PROCESSOR_VPSS_CHN,
                                  &video_frame, IMAGE_PROCESSOR_TIMEOUT);
        
        if (ret != CVI_SUCCESS) {
            pthread_mutex_unlock(&g_image_processor_mutex);
            aos_msleep(100);
            continue;
        }
        
        // Process the frame
        process_frame(&video_frame, &chn_attr);
        
        // Release frame
        ret = CVI_VPSS_ReleaseChnFrame(IMAGE_PROCESSOR_VPSS_GRP, IMAGE_PROCESSOR_VPSS_CHN, &video_frame);
        if (ret != CVI_SUCCESS) {
            printf("[%s] Failed to release VPSS frame: %d\n", TAG, ret);
        }
        
        pthread_mutex_unlock(&g_image_processor_mutex);
        
        // Small delay to prevent excessive CPU usage
        aos_msleep(33); // ~30 FPS
    }
    
    printf("[%s] Image processor thread stopped!\n", TAG);
    return NULL;
}

/**
 * Initialize image processor
 */
int image_processor_init(void)
{
    printf("[%s] Initializing image processor...\n", TAG);
    
    // Initialize mutex
    pthread_mutex_init(&g_image_processor_mutex, NULL);
    
    // Reset statistics
    g_total_frames_processed = 0;
    g_total_brightness_sum = 0.0f;
    g_min_brightness = 255.0f;
    g_max_brightness = 0.0f;
    
    printf("[%s] Image processor initialized successfully!\n", TAG);
    return CVI_SUCCESS;
}

/**
 * Start image processor thread
 */
int image_processor_start(void)
{
    int ret = 0;
    pthread_attr_t thread_attr;
    
    if (g_image_processor_running) {
        printf("[%s] Image processor already running!\n", TAG);
        return CVI_SUCCESS;
    }
    
    printf("[%s] Starting image processor...\n", TAG);
    
    // Initialize image processor
    ret = image_processor_init();
    if (ret != CVI_SUCCESS) {
        printf("[%s] Image processor initialization failed: %d\n", TAG, ret);
        return ret;
    }
    
    // Set thread attributes
    pthread_attr_init(&thread_attr);
    pthread_attr_setstacksize(&thread_attr, 8192);
    
    // Start processing thread
    g_image_processor_running = 1;
    ret = pthread_create(&g_image_processor_thread, &thread_attr, 
                        image_processor_thread, NULL);
    
    if (ret != 0) {
        printf("[%s] Failed to create image processor thread: %d\n", TAG, ret);
        g_image_processor_running = 0;
        return CVI_FAILURE;
    }
    
    pthread_setname_np(g_image_processor_thread, "image_processor");
    
    printf("[%s] Image processor started successfully!\n", TAG);
    printf("[%s] Monitoring video stream for brightness analysis...\n", TAG);
    
    return CVI_SUCCESS;
}

/**
 * Stop image processor
 */
void image_processor_stop(void)
{
    printf("[%s] Stopping image processor...\n", TAG);
    
    g_image_processor_running = 0;
    
    if (g_image_processor_thread) {
        pthread_join(g_image_processor_thread, NULL);
        g_image_processor_thread = 0;
    }
    
    // Cleanup mutex
    pthread_mutex_destroy(&g_image_processor_mutex);
    
    printf("[%s] Image processor stopped!\n", TAG);
    printf("[%s] Final statistics: %d frames processed\n", TAG, g_total_frames_processed);
    if (g_total_frames_processed > 0) {
        printf("[%s] Average brightness: %.2f\n", TAG, 
               g_total_brightness_sum / g_total_frames_processed);
        printf("[%s] Brightness range: %.2f - %.2f\n", TAG, 
               g_min_brightness, g_max_brightness);
    }
}

/**
 * Get image processor statistics
 */
void image_processor_get_stats(int *total_frames, float *avg_brightness, 
                              float *min_brightness, float *max_brightness)
{
    pthread_mutex_lock(&g_image_processor_mutex);
    *total_frames = g_total_frames_processed;
    if (g_total_frames_processed > 0) {
        *avg_brightness = g_total_brightness_sum / g_total_frames_processed;
    } else {
        *avg_brightness = 0.0f;
    }
    *min_brightness = g_min_brightness;
    *max_brightness = g_max_brightness;
    pthread_mutex_unlock(&g_image_processor_mutex);
}
