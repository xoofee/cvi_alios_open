/*
 * Face Detection Module for USB Camera
 * Detects faces in video stream and prints messages
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <aos/kernel.h>
#include "cvi_tdl.h"
#include "cvi_tpu_interface.h"
#include "cvi_vpss.h"
#include "cvi_param.h"

#define TAG "face_detection"

// Face detection configuration
#define FACE_DETECTION_VPSS_GRP 15
#define FACE_DETECTION_VPSS_CHN 2
#define FACE_DETECTION_TIMEOUT 2000
// Model paths (try multiple locations in priority order)
#define RETINA_FACE_MODEL_PATH_WEIGHT "/mnt/flash/weight/retinaface_mnet0.25_342_608.cvimodel"
#define RETINA_FACE_MODEL_PATH_SD "/mnt/sd/retinaface_mnet0.25_342_608.cvimodel"
#define RETINA_FACE_MODEL_PATH_DEFAULT RETINA_FACE_MODEL_PATH_WEIGHT

// Weight partition is mounted at /mnt/flash/weight/ during runtime
// The model gets burned into the weight partition during firmware flashing

// Global variables
static cvitdl_handle_t g_face_detection_handle = NULL;
static pthread_t g_face_detection_thread;
static int g_face_detection_running = 0;
static pthread_mutex_t g_face_detection_mutex;

// Face detection statistics
static int g_total_faces_detected = 0;
static int g_detection_frames = 0;

/**
 * Initialize face detection system
 */
int face_detection_init(void)
{
    CVI_S32 ret = CVI_SUCCESS;
    
    printf("[%s] Initializing face detection system...\n", TAG);
    
    // Initialize TPU
    ret = cvi_tpu_init();
    if (ret != CVI_SUCCESS) {
        printf("[%s] TPU initialization failed: %d\n", TAG, ret);
        return ret;
    }
    
    // Wait for TPU to be ready
    aos_msleep(1000);
    
    // Create TDL handle
    ret = CVI_TDL_CreateHandle2(&g_face_detection_handle, FACE_DETECTION_VPSS_GRP, 0);
    if (ret != CVI_SUCCESS) {
        printf("[%s] TDL handle creation failed: %d\n", TAG, ret);
        cvi_tpu_deinit();
        return ret;
    }
    
    // Try to load RetinaFace model from multiple locations
    const char* model_paths[] = {
        RETINA_FACE_MODEL_PATH_WEIGHT,  // Weight partition (preferred)
        RETINA_FACE_MODEL_PATH_SD,      // SD card (fallback)
        NULL
    };
    
    ret = CVI_FAILURE;
    for (int i = 0; model_paths[i] != NULL; i++) {
        printf("[%s] Trying to load model from: %s\n", TAG, model_paths[i]);
        ret = CVI_TDL_OpenModel(g_face_detection_handle, 
                               CVI_TDL_SUPPORTED_MODEL_RETINAFACE, 
                               model_paths[i]);
        if (ret == CVI_SUCCESS) {
            printf("[%s] ✅ Model loaded successfully from: %s\n", TAG, model_paths[i]);
            break;
        } else {
            printf("[%s] ❌ Failed to load from: %s (error: %d)\n", TAG, model_paths[i], ret);
        }
    }
    
    if (ret != CVI_SUCCESS) {
        printf("[%s] ❌ RetinaFace model loading failed from all locations!\n", TAG);
        printf("[%s] Please ensure model file exists in one of these locations:\n", TAG);
        printf("[%s]   - %s (RECOMMENDED - weight partition)\n", TAG, RETINA_FACE_MODEL_PATH_WEIGHT);
        printf("[%s]   - %s (SD card fallback)\n", TAG, RETINA_FACE_MODEL_PATH_SD);
        CVI_TDL_DestroyHandle(g_face_detection_handle);
        cvi_tpu_deinit();
        return ret;
    }
    
    // Initialize mutex
    pthread_mutex_init(&g_face_detection_mutex, NULL);
    
    printf("[%s] Face detection system initialized successfully!\n", TAG);
    return CVI_SUCCESS;
}

/**
 * Deinitialize face detection system
 */
void face_detection_deinit(void)
{
    printf("[%s] Deinitializing face detection system...\n", TAG);
    
    // Stop detection thread
    g_face_detection_running = 0;
    if (g_face_detection_thread) {
        pthread_join(g_face_detection_thread, NULL);
    }
    
    // Cleanup TDL handle
    if (g_face_detection_handle) {
        CVI_TDL_CloseAllModel(g_face_detection_handle);
        CVI_TDL_DestroyHandle(g_face_detection_handle);
        g_face_detection_handle = NULL;
    }
    
    // Deinitialize TPU
    cvi_tpu_deinit();
    
    // Cleanup mutex
    pthread_mutex_destroy(&g_face_detection_mutex);
    
    printf("[%s] Face detection system deinitialized!\n", TAG);
}

/**
 * Process detected faces and print messages
 */
void process_detected_faces(cvtdl_face_t *face_meta)
{
    if (face_meta->size > 0) {
        g_total_faces_detected += face_meta->size;
        
        printf("\n🎯 [FACE DETECTED] %d face(s) found in frame!\n", face_meta->size);
        
        for (uint32_t i = 0; i < face_meta->size; i++) {
            cvtdl_face_info_t *face_info = &face_meta->info[i];
            
            printf("   Face %d: Position=(%.1f,%.1f,%.1f,%.1f) Confidence=%.3f\n",
                   i + 1,
                   face_info->bbox.x1, face_info->bbox.y1,
                   face_info->bbox.x2, face_info->bbox.y2,
                   face_info->bbox.score);
        }
        
        printf("   📊 Total faces detected so far: %d\n", g_total_faces_detected);
        printf("   📈 Detection rate: %.1f%% (%d/%d frames)\n", 
               (float)g_detection_frames * 100.0f / (g_detection_frames + 1),
               g_detection_frames, g_detection_frames + 1);
        printf("\n");
    }
}

/**
 * Main face detection thread
 */
void* face_detection_thread(void* arg)
{
    CVI_S32 ret = CVI_SUCCESS;
    cvtdl_face_t face_meta = {0};
    VIDEO_FRAME_INFO_S video_frame;
    
    printf("[%s] Face detection thread started!\n", TAG);
    
    while (g_face_detection_running) {
        pthread_mutex_lock(&g_face_detection_mutex);
        
        // Get frame from VPSS
        ret = CVI_VPSS_GetChnFrame(FACE_DETECTION_VPSS_GRP, FACE_DETECTION_VPSS_CHN,
                                  &video_frame, FACE_DETECTION_TIMEOUT);
        
        if (ret != CVI_SUCCESS) {
            pthread_mutex_unlock(&g_face_detection_mutex);
            aos_msleep(100);
            continue;
        }
        
        // Clear face metadata
        memset(&face_meta, 0, sizeof(cvtdl_face_t));
        
        // Run face detection
        ret = CVI_TDL_FaceDetection(g_face_detection_handle, &video_frame, 
                                   CVI_TDL_SUPPORTED_MODEL_RETINAFACE, &face_meta);
        
        if (ret == CVI_SUCCESS) {
            g_detection_frames++;
            process_detected_faces(&face_meta);
        } else {
            printf("[%s] Face detection failed: %d\n", TAG, ret);
        }
        
        // Release frame
        CVI_VPSS_ReleaseChnFrame(FACE_DETECTION_VPSS_GRP, FACE_DETECTION_VPSS_CHN, &video_frame);
        
        // Free face metadata
        CVI_TDL_Free(&face_meta);
        
        pthread_mutex_unlock(&g_face_detection_mutex);
        
        // Small delay to prevent excessive CPU usage
        aos_msleep(33); // ~30 FPS
    }
    
    printf("[%s] Face detection thread stopped!\n", TAG);
    return NULL;
}

/**
 * Start face detection
 */
int face_detection_start(void)
{
    int ret = 0;
    pthread_attr_t thread_attr;
    
    if (g_face_detection_running) {
        printf("[%s] Face detection already running!\n", TAG);
        return CVI_SUCCESS;
    }
    
    printf("[%s] Starting face detection...\n", TAG);
    
    // Initialize face detection system
    ret = face_detection_init();
    if (ret != CVI_SUCCESS) {
        printf("[%s] Face detection initialization failed: %d\n", TAG, ret);
        return ret;
    }
    
    // Set thread attributes
    pthread_attr_init(&thread_attr);
    pthread_attr_setstacksize(&thread_attr, 8192);
    
    // Start detection thread
    g_face_detection_running = 1;
    ret = pthread_create(&g_face_detection_thread, &thread_attr, 
                        face_detection_thread, NULL);
    
    if (ret != 0) {
        printf("[%s] Failed to create face detection thread: %d\n", TAG, ret);
        g_face_detection_running = 0;
        face_detection_deinit();
        return CVI_FAILURE;
    }
    
    pthread_setname_np(g_face_detection_thread, "face_detection");
    
    printf("[%s] Face detection started successfully!\n", TAG);
    printf("[%s] Monitoring video stream for faces...\n", TAG);
    
    return CVI_SUCCESS;
}

/**
 * Stop face detection
 */
void face_detection_stop(void)
{
    printf("[%s] Stopping face detection...\n", TAG);
    
    g_face_detection_running = 0;
    
    if (g_face_detection_thread) {
        pthread_join(g_face_detection_thread, NULL);
        g_face_detection_thread = 0;
    }
    
    face_detection_deinit();
    
    printf("[%s] Face detection stopped!\n", TAG);
    printf("[%s] Final statistics: %d faces detected in %d frames\n", 
           TAG, g_total_faces_detected, g_detection_frames);
}

/**
 * Get face detection statistics
 */
void face_detection_get_stats(int *total_faces, int *total_frames)
{
    pthread_mutex_lock(&g_face_detection_mutex);
    *total_faces = g_total_faces_detected;
    *total_frames = g_detection_frames;
    pthread_mutex_unlock(&g_face_detection_mutex);
}
