/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: face_detection.c
 * Description: Face detection module for USB camera solution
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "cvi_tdl.h"
#include "cvi_tdl_core.h"
#include "cvi_tpu_interface.h"
#include "cvi_vpss.h"
#include "cvi_param.h"
#include "drv/spiflash.h"
#include "face_detection.h"

#define TAG "face_detection"

// Global face detection context
static face_detection_context_t g_face_ctx = {0};

// Weight partition configuration
#define WEIGHT_PARTITION_ADDR 0x4DE000
#define MODEL_SIZE 551576

// Spiflash handle for model loading
static csi_spiflash_t g_spiflash_handle;
static int8_t* g_model_buffer = NULL;

// already done in param/custom_vpssparam.c???
// /**
//  * Initialize VPSS for face detection preprocessing
//  */
// static CVI_S32 init_face_detection_vpss(void)
// {
//     CVI_S32 ret = CVI_SUCCESS;
    
//     if (g_face_ctx.vpss_initialized) {
//         return CVI_SUCCESS;
//     }
    
//     printf("[%s] Initializing VPSS for face detection preprocessing...\n", TAG);
    
//     // Configure VPSS group attributes
//     memset(&g_face_ctx.vpss_grp_attr, 0, sizeof(VPSS_GRP_ATTR_S));
//     g_face_ctx.vpss_grp_attr.u8VpssDev = 0;
//     g_face_ctx.vpss_grp_attr.u32MaxW = 1920;  // Max input width
//     g_face_ctx.vpss_grp_attr.u32MaxH = 1080;  // Max input height
//     g_face_ctx.vpss_grp_attr.enPixelFormat = PIXEL_FORMAT_NV21;
//     g_face_ctx.vpss_grp_attr.stFrameRate.s32SrcFrameRate = -1;
//     g_face_ctx.vpss_grp_attr.stFrameRate.s32DstFrameRate = -1;
    
//     // Configure VPSS channel attributes for face detection
//     memset(&g_face_ctx.vpss_chn_attr, 0, sizeof(VPSS_CHN_ATTR_S));
//     g_face_ctx.vpss_chn_attr.u32Width = FACE_DETECTION_MODEL_WIDTH;
//     g_face_ctx.vpss_chn_attr.u32Height = FACE_DETECTION_MODEL_HEIGHT;
//     g_face_ctx.vpss_chn_attr.enVideoFormat = VIDEO_FORMAT_LINEAR;
//     g_face_ctx.vpss_chn_attr.enPixelFormat = PIXEL_FORMAT_RGB_888_PLANAR;  // RGB for AI model
//     g_face_ctx.vpss_chn_attr.stFrameRate.s32SrcFrameRate = -1;
//     g_face_ctx.vpss_chn_attr.stFrameRate.s32DstFrameRate = -1;
//     g_face_ctx.vpss_chn_attr.bFlip = CVI_FALSE;
//     g_face_ctx.vpss_chn_attr.bMirror = CVI_FALSE;
//     g_face_ctx.vpss_chn_attr.u32Depth = 1;
    
//     // Configure aspect ratio (stretch to exact dimensions)
//     g_face_ctx.vpss_chn_attr.stAspectRatio.enMode = ASPECT_RATIO_NONE;
//     g_face_ctx.vpss_chn_attr.stAspectRatio.bEnableBgColor = CVI_TRUE;
//     g_face_ctx.vpss_chn_attr.stAspectRatio.u32BgColor = COLOR_RGB_BLACK;
    
//     // Configure normalization (disable for raw data)
//     g_face_ctx.vpss_chn_attr.stNormalize.bEnable = CVI_FALSE;
    
//     // Create VPSS group
//     ret = CVI_VPSS_CreateGrp(FACE_DETECTION_VPSS_GRP, &g_face_ctx.vpss_grp_attr);
//     if (ret != CVI_SUCCESS) {
//         printf("[%s] Failed to create VPSS group: %d\n", TAG, ret);
//         return ret;
//     }
    
//     // Set VPSS channel attributes
//     ret = CVI_VPSS_SetChnAttr(FACE_DETECTION_VPSS_GRP, FACE_DETECTION_VPSS_CHN, &g_face_ctx.vpss_chn_attr);
//     if (ret != CVI_SUCCESS) {
//         printf("[%s] Failed to set VPSS channel attributes: %d\n", TAG, ret);
//         CVI_VPSS_DestroyGrp(FACE_DETECTION_VPSS_GRP);
//         return ret;
//     }
    
//     // Enable VPSS channel
//     ret = CVI_VPSS_EnableChn(FACE_DETECTION_VPSS_GRP, FACE_DETECTION_VPSS_CHN);
//     if (ret != CVI_SUCCESS) {
//         printf("[%s] Failed to enable VPSS channel: %d\n", TAG, ret);
//         CVI_VPSS_DestroyGrp(FACE_DETECTION_VPSS_GRP);
//         return ret;
//     }
    
//     // Start VPSS group
//     ret = CVI_VPSS_StartGrp(FACE_DETECTION_VPSS_GRP);
//     if (ret != CVI_SUCCESS) {
//         printf("[%s] Failed to start VPSS group: %d\n", TAG, ret);
//         CVI_VPSS_DisableChn(FACE_DETECTION_VPSS_GRP, FACE_DETECTION_VPSS_CHN);
//         CVI_VPSS_DestroyGrp(FACE_DETECTION_VPSS_GRP);
//         return ret;
//     }
    
//     g_face_ctx.vpss_initialized = CVI_TRUE;
//     printf("[%s] VPSS initialized for face detection: %dx%d -> %dx%d\n", 
//            TAG, g_face_ctx.vpss_grp_attr.u32MaxW, g_face_ctx.vpss_grp_attr.u32MaxH, 
//            FACE_DETECTION_MODEL_WIDTH, FACE_DETECTION_MODEL_HEIGHT);
    
//     return CVI_SUCCESS;
// }

// /**
//  * Deinitialize VPSS for face detection
//  */
// static void deinit_face_detection_vpss(void)
// {
//     if (!g_face_ctx.vpss_initialized) {
//         return;
//     }
    
//     printf("[%s] Deinitializing VPSS for face detection...\n", TAG);
    
//     CVI_VPSS_StopGrp(FACE_DETECTION_VPSS_GRP);
//     CVI_VPSS_DisableChn(FACE_DETECTION_VPSS_GRP, FACE_DETECTION_VPSS_CHN);
//     CVI_VPSS_DestroyGrp(FACE_DETECTION_VPSS_GRP);
    
//     g_face_ctx.vpss_initialized = CVI_FALSE;
//     printf("[%s] VPSS deinitialized for face detection\n", TAG);
// }

/**
 * Initialize spiflash for weight partition access
 */
static CVI_S32 init_weight_partition_access(void)
{
    CVI_S32 ret = CVI_SUCCESS;
    
    printf("[%s] Initializing spiflash for weight partition access...\n", TAG);
    
    // Initialize spiflash (assuming spiflash index 0)
    ret = csi_spiflash_spi_init(&g_spiflash_handle, 0, NULL);
    if (ret != 0) {
        printf("[%s] Failed to initialize spiflash: %d\n", TAG, ret);
        return CVI_FAILURE;
    }
    
    printf("[%s] ✅ Spiflash initialized for weight partition access\n", TAG);
    return CVI_SUCCESS;
}

/**
 * Load face detection model from weight partition
 */
static CVI_S32 load_face_detection_model(void)
{
    CVI_S32 ret = CVI_SUCCESS;
    
    printf("[%s] Loading face detection model from weight partition...\n", TAG);
    
    // Allocate buffer for model
    g_model_buffer = (int8_t*)malloc(MODEL_SIZE);
    if (!g_model_buffer) {
        printf("[%s] Failed to allocate model buffer (%d bytes)\n", TAG, MODEL_SIZE);
        return CVI_FAILURE;
    }
    
    // Read model data from weight partition
    ret = csi_spiflash_read(&g_spiflash_handle, WEIGHT_PARTITION_ADDR, 
                           g_model_buffer, MODEL_SIZE);
    if (ret < 0) {
        printf("[%s] Failed to read model data: %d\n", TAG, ret);
        free(g_model_buffer);
        g_model_buffer = NULL;
        return CVI_FAILURE;
    }
    
    printf("[%s] ✅ Model data read from weight partition (%d bytes)\n", TAG, MODEL_SIZE);
    
    // Initialize TDL handle
    ret = CVI_TDL_CreateHandle(&g_face_ctx.tdl_handle);
    if (ret != CVI_SUCCESS) {
        printf("[%s] Failed to create TDL handle: %d\n", TAG, ret);
        free(g_model_buffer);
        g_model_buffer = NULL;
        return CVI_FAILURE;
    }
    
    // Load model from buffer
    ret = CVI_TDL_OpenModel_FromBuffer(g_face_ctx.tdl_handle, 
                                      CVI_TDL_SUPPORTED_MODEL_RETINAFACE, 
                                      g_model_buffer,
                                      MODEL_SIZE);
    
    if (ret == CVI_SUCCESS) {
        printf("[%s] ✅ Face detection model loaded successfully!\n", TAG);
        return CVI_SUCCESS;
    } else {
        printf("[%s] ❌ Failed to load face detection model: %d\n", TAG, ret);
        CVI_TDL_DestroyHandle(g_face_ctx.tdl_handle);
        g_face_ctx.tdl_handle = NULL;
        free(g_model_buffer);
        g_model_buffer = NULL;
        return CVI_FAILURE;
    }
}

/**
 * Initialize face detection module
 */
CVI_S32 face_detection_init(void)
{
    CVI_S32 ret = CVI_SUCCESS;
    
    printf("[%s] Initializing face detection module...\n", TAG);
    
    if (g_face_ctx.initialized) {
        printf("[%s] Face detection already initialized\n", TAG);
        return CVI_SUCCESS;
    }
    
    // Initialize mutex
    pthread_mutex_init(&g_face_ctx.mutex, NULL);
    
    // Initialize spiflash for model loading
    ret = init_weight_partition_access();
    if (ret != CVI_SUCCESS) {
        printf("[%s] Failed to initialize weight partition access\n", TAG);
        return ret;
    }
    
    // Load face detection model
    ret = load_face_detection_model();
    if (ret != CVI_SUCCESS) {
        printf("[%s] Failed to load face detection model\n", TAG);
        return ret;
    }
    
    // // Initialize VPSS for preprocessing
    // ret = init_face_detection_vpss();
    // if (ret != CVI_SUCCESS) {
    //     printf("[%s] Failed to initialize VPSS for face detection\n", TAG);
    //     CVI_TDL_DestroyHandle(g_face_ctx.tdl_handle);
    //     g_face_ctx.tdl_handle = NULL;
    //     free(g_model_buffer);
    //     g_model_buffer = NULL;
    //     return ret;
    // }
    
    g_face_ctx.initialized = CVI_TRUE;
    g_face_ctx.total_faces_detected = 0;
    g_face_ctx.detection_frames = 0;
    
    printf("[%s] ✅ Face detection module initialized successfully!\n", TAG);
    return CVI_SUCCESS;
}

/**
 * Deinitialize face detection module
 */
void face_detection_deinit(void)
{
    printf("[%s] Deinitializing face detection module...\n", TAG);
    
    if (!g_face_ctx.initialized) {
        return;
    }
    
    // Deinitialize VPSS
    // deinit_face_detection_vpss();
    
    // Destroy TDL handle
    if (g_face_ctx.tdl_handle) {
        CVI_TDL_DestroyHandle(g_face_ctx.tdl_handle);
        g_face_ctx.tdl_handle = NULL;
    }
    
    // Free model buffer
    if (g_model_buffer) {
        free(g_model_buffer);
        g_model_buffer = NULL;
    }
    
    // Destroy mutex
    pthread_mutex_destroy(&g_face_ctx.mutex);
    
    g_face_ctx.initialized = CVI_FALSE;
    printf("[%s] ✅ Face detection module deinitialized\n", TAG);
}

/**
 * Process frame for face detection
 */
int face_detection_process_frame(VIDEO_FRAME_INFO_S *frame, VPSS_CHN_ATTR_S *chn_attr,
                                 face_detection_result_t *results, int max_results)
{
    printf(" [face_detection_process_frame] \n");
    CVI_S32 ret = CVI_SUCCESS;
    VIDEO_FRAME_INFO_S processed_frame = {0};
    int num_faces = 0;
    
    if (!g_face_ctx.initialized || !frame || !results || max_results <= 0) {
        return -1;
    }
    
    pthread_mutex_lock(&g_face_ctx.mutex);

    // 22 is PIXEL_FORMAT_YUYV, see cvi_comm_video.h PIXEL_FORMAT_E
    printf("[%s] Input frame pixel format: %d\n", TAG, frame->stVFrame.enPixelFormat);
    
    // Send frame to VPSS for preprocessing (resize + format conversion)
    ret = CVI_VPSS_SendFrame(FACE_DETECTION_VPSS_GRP, frame, FACE_DETECTION_TIMEOUT);
    if (ret != CVI_SUCCESS) {
        printf("[%s] Failed to send frame to VPSS: %d\n", TAG, ret);
        pthread_mutex_unlock(&g_face_ctx.mutex);
        return -1;
    }
    
    // Get processed frame from VPSS
    ret = CVI_VPSS_GetChnFrame(FACE_DETECTION_VPSS_GRP, FACE_DETECTION_VPSS_CHN, 
                              &processed_frame, FACE_DETECTION_TIMEOUT);
    if (ret != CVI_SUCCESS) {
        printf("[%s] Failed to get processed frame from VPSS: %d\n", TAG, ret);
        pthread_mutex_unlock(&g_face_ctx.mutex);
        return -1;
    }
    
    // Perform face detection
    cvtdl_face_t face_meta = {0};
    ret = CVI_TDL_FaceDetection(g_face_ctx.tdl_handle, &processed_frame, 
                               CVI_TDL_SUPPORTED_MODEL_RETINAFACE, &face_meta);
    if (ret != CVI_SUCCESS) {
        printf("[%s] Face detection failed: %d\n", TAG, ret);
        CVI_VPSS_ReleaseChnFrame(FACE_DETECTION_VPSS_GRP, FACE_DETECTION_VPSS_CHN, &processed_frame);
        pthread_mutex_unlock(&g_face_ctx.mutex);
        return -1;
    }
    
    // Convert detection results
    num_faces = (face_meta.size < max_results) ? face_meta.size : max_results;
    for (int i = 0; i < num_faces; i++) {
        results[i].x1 = face_meta.info[i].bbox.x1 / chn_attr->u32Width;
        results[i].y1 = face_meta.info[i].bbox.y1 / chn_attr->u32Height;
        results[i].x2 = face_meta.info[i].bbox.x2 / chn_attr->u32Width;
        results[i].y2 = face_meta.info[i].bbox.y2 / chn_attr->u32Height;
        results[i].confidence = face_meta.info[i].bbox.score;
        results[i].valid = 1;
    }
    
    // Update statistics
    g_face_ctx.total_faces_detected += num_faces;
    g_face_ctx.detection_frames++;
    
    // Release processed frame
    CVI_VPSS_ReleaseChnFrame(FACE_DETECTION_VPSS_GRP, FACE_DETECTION_VPSS_CHN, &processed_frame);
    
    // Free face metadata
    CVI_TDL_Free(&face_meta);
    
    pthread_mutex_unlock(&g_face_ctx.mutex);
    
    if (num_faces > 0) {
        printf("[%s] Detected %d faces\n", TAG, num_faces);
    }
    
    return num_faces;
}

/**
 * Draw bounding boxes on frame (simple implementation)
 */
void face_detection_draw_boxes(VIDEO_FRAME_INFO_S *frame, VPSS_CHN_ATTR_S *chn_attr,
                               face_detection_result_t *results, int num_results)
{
    // This is a placeholder implementation
    // In a real implementation, you would draw rectangles on the YUYV frame
    // For now, we'll just print the detection results
    
    if (!frame || !chn_attr || !results || num_results <= 0) {
        return;
    }
    
    printf("[%s] Drawing %d face bounding boxes:\n", TAG, num_results);
    for (int i = 0; i < num_results; i++) {
        if (results[i].valid) {
            printf("  Face %d: (%.3f, %.3f) to (%.3f, %.3f), confidence: %.3f\n",
                   i, results[i].x1, results[i].y1, results[i].x2, results[i].y2, results[i].confidence);
        }
    }
}

/**
 * Get face detection statistics
 */
void face_detection_get_stats(int *total_faces, int *total_frames)
{
    if (total_faces) {
        *total_faces = g_face_ctx.total_faces_detected;
    }
    if (total_frames) {
        *total_frames = g_face_ctx.detection_frames;
    }
}

/**
 * Check if face detection is initialized
 */
CVI_BOOL face_detection_is_initialized(void)
{
    return g_face_ctx.initialized;
}