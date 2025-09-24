/*
 * Brightness Analyzer Module
 * Processes frames and calculates brightness statistics
 */

#include <stdio.h>
#include <stdint.h>
#include "cvi_type.h"
#include "cvi_comm_vpss.h"

#define TAG "brightness_analyzer"

// Statistics
int g_total_frames_processed = 0;
float g_total_brightness_sum = 0.0f;
float g_min_brightness = 255.0f;
float g_max_brightness = 0.0f;

/**
 * Calculate mean brightness of Y channel in YUYV format
 */
float calculate_y_brightness_yuyv(uint8_t *yuyv_data, int width, int height)
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
 * Update brightness statistics and print
 */
void update_brightness_stats(float brightness)
{
    g_total_frames_processed++;
    g_total_brightness_sum += brightness;
    
    if (brightness < g_min_brightness) {
        g_min_brightness = brightness;
    }
    if (brightness > g_max_brightness) {
        g_max_brightness = brightness;
    }
    
    // Print brightness information every 30 frames to reduce spam
    if (g_total_frames_processed % 30 == 0) {
        printf("[%s] Frame %d: Brightness=%.2f, Avg=%.2f, Min=%.2f, Max=%.2f\n",
               TAG, g_total_frames_processed, brightness,
               g_total_brightness_sum / g_total_frames_processed,
               g_min_brightness, g_max_brightness);
    }
}

/**
 * Process frame directly - called by UVC adapter
 */
void brightness_analyzer_process_frame(VIDEO_FRAME_INFO_S *frame, VPSS_CHN_ATTR_S *chn_attr)
{
    printf("[%s] brightness_analyzer_process_frame\n", TAG);
    return;
    float brightness = 0.0f;
    
    // Map physical address to virtual address
    // frame->stVFrame.pu8VirAddr[0] = (uint8_t*)frame->stVFrame.u64PhyAddr[0];
    
    // Calculate brightness based on pixel format
    switch (chn_attr->enPixelFormat) {
        case PIXEL_FORMAT_YUYV:
            brightness = calculate_y_brightness_yuyv(
                (uint8_t*)frame->stVFrame.u64PhyAddr[0],
                chn_attr->u32Width,
                chn_attr->u32Height
            );
            break;
            
        case PIXEL_FORMAT_NV21:
            brightness = calculate_y_brightness_nv21(
                (uint8_t*)frame->stVFrame.u64PhyAddr[0],
                chn_attr->u32Width,
                chn_attr->u32Height
            );
            break;
            
        default:
            printf("[%s] Unsupported pixel format: %d\n", TAG, chn_attr->enPixelFormat);
            // frame->stVFrame.pu8VirAddr[0] = NULL;
            return;
    }
    
    // Update statistics
    update_brightness_stats(brightness);
    
    // Unmap virtual address
    // frame->stVFrame.pu8VirAddr[0] = NULL;
}

/**
 * Initialize brightness analyzer
 */
int brightness_analyzer_init(void)
{
    printf("[%s] Initializing brightness analyzer...\n", TAG);
    
    // Reset statistics
    // g_total_frames_processed = 0;
    // g_total_brightness_sum = 0.0f;
    // g_min_brightness = 255.0f;
    // g_max_brightness = 0.0f;
    
    // No need to register callback - will be called directly by UVC adapter
    
    printf("[%s] Brightness analyzer initialized successfully!\n", TAG);
    return CVI_SUCCESS;
}

/**
 * Deinitialize brightness analyzer
 */
void brightness_analyzer_deinit(void)
{
    printf("[%s] Deinitializing brightness analyzer...\n", TAG);
    
    // No need to unregister callback - was called directly by UVC adapter
    
    printf("[%s] Final statistics: %d frames processed\n", TAG, g_total_frames_processed);
    if (g_total_frames_processed > 0) {
        printf("[%s] Average brightness: %.2f\n", TAG, 
               g_total_brightness_sum / g_total_frames_processed);
        printf("[%s] Brightness range: %.2f - %.2f\n", TAG, 
               g_min_brightness, g_max_brightness);
    }
    
    printf("[%s] Brightness analyzer deinitialized!\n", TAG);
}

/**
 * Get brightness analyzer statistics
 */
void brightness_analyzer_get_stats(int *total_frames, float *avg_brightness, 
                                  float *min_brightness, float *max_brightness)
{
    *total_frames = g_total_frames_processed;
    if (g_total_frames_processed > 0) {
        *avg_brightness = g_total_brightness_sum / g_total_frames_processed;
    } else {
        *avg_brightness = 0.0f;
    }
    *min_brightness = g_min_brightness;
    *max_brightness = g_max_brightness;
}
