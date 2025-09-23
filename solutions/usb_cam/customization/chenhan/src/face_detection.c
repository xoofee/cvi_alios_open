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
#include "cvi_tdl_core.h"
#include "cvi_tpu_interface.h"
#include "cvi_vpss.h"
#include "cvi_param.h"
#include "drv/spiflash.h"

#define TAG "face_detection"

// Face detection configuration
#define FACE_DETECTION_VPSS_GRP 15
#define FACE_DETECTION_VPSS_CHN 2
#define FACE_DETECTION_TIMEOUT 2000

// Weight partition configuration (from config.yaml)
#define WEIGHT_PARTITION_ADDR 0x4DE000
// #define WEIGHT_PARTITION_SIZE 0x850000  // 8.3125 MB

// Global variables
static cvitdl_handle_t g_face_detection_handle = NULL;
static pthread_t g_face_detection_thread;
static int g_face_detection_running = 0;
static pthread_mutex_t g_face_detection_mutex;
static csi_spiflash_t g_spiflash_handle;
static int8_t* g_model_buffer = NULL;

// Face detection statistics
static int g_total_faces_detected = 0;
static int g_detection_frames = 0;

/**
 * Initialize spiflash for weight partition access
 */
int init_weight_partition_access(void)
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
 * CVIModel header structure (from cvi_runtime/include/runtime/model.hpp)
 */
typedef struct {
    char magic[8];        // "CviModel" magic string
    uint32_t body_size;   // Size of model parameters
    char major;           // Major version
    char minor;           // Minor version  
    char md5[16];         // MD5 checksum
    char chip[16];        // Target chip type
    char padding[2];     // Padding bytes
} MODEL_HEADER;

// /**
//  * Detect actual model size from CVIModel format using official structure
//  */
// uint32_t detect_model_size_from_buffer(int8_t* buffer, uint32_t max_size, uint32_t* actual_size)
// {
//     // Check minimum size for MODEL_HEADER
//     if (max_size < sizeof(MODEL_HEADER)) {
//         printf("[%s] Buffer too small for MODEL_HEADER (%d < %d)\n", TAG, max_size, sizeof(MODEL_HEADER));
//         return CVI_FAILURE;
//     }
    
//     // Parse MODEL_HEADER
//     MODEL_HEADER* header = (MODEL_HEADER*)buffer;
    
//     // Check CVIModel magic - should be "CviModel"
//     if (strncmp(header->magic, "CviModel", 8) != 0) {
//         printf("[%s] Invalid CVIModel magic: %.8s (expected: CviModel)\n", TAG, header->magic);
//         return CVI_FAILURE;
//     }
    
//     // Get model body size
//     uint32_t body_size = header->body_size;
    
//     // Calculate total model size: header + body
//     uint32_t total_size = sizeof(MODEL_HEADER) + body_size;
    
//     // Validate total size
//     if (total_size < sizeof(MODEL_HEADER) || total_size > max_size) {
//         printf("[%s] Invalid model size: %d bytes (header: %d, body: %d, max: %d)\n", 
//                TAG, total_size, sizeof(MODEL_HEADER), body_size, max_size);
//         return CVI_FAILURE;
//     }
    
//     *actual_size = total_size;
//     printf("[%s] CVIModel detected: version %d.%d, chip: %.16s, total size: %d bytes\n", 
//            TAG, header->major, header->minor, header->chip, total_size);
//     return CVI_SUCCESS;
// }

/**
 * Load model from weight partition using CVI_TDL_OpenModel_FromBuffer
 */
int load_model_from_weight_partition(cvitdl_handle_t handle)
{
    CVI_S32 ret = CVI_SUCCESS;
    
    printf("[%s] Loading model from weight partition...\n", TAG);

    const uint8_t header_size = sizeof(MODEL_HEADER);
    
    uint8_t header_buffer[header_size];
    ret = csi_spiflash_read(&g_spiflash_handle, WEIGHT_PARTITION_ADDR, 
                           header_buffer, header_size);
    if (ret < 0) {
        printf("[%s] Failed to read model header: %d\n", TAG, ret);
        return CVI_FAILURE;
    }

    MODEL_HEADER* header = (MODEL_HEADER*)header_buffer;
    
    printf("[%s] MODEL_HEADER info:\n", TAG);
    printf("  magic     : %.8s\n", header->magic);
    printf("  body_size : %u\n", header->body_size);
    printf("  major     : %d\n", header->major);
    printf("  minor     : %d\n", header->minor);
    printf("  md5       :");
    for (int i = 0; i < 16; ++i) {
        printf(" %02X", (unsigned char)header->md5[i]);
    }
    printf("\n");
    printf("  chip      : %.16s\n", header->chip);
    printf("  padding   : %02X %02X\n", (unsigned char)header->padding[0], (unsigned char)header->padding[1]);

    
    // Detect actual model size
    uint32_t model_size = 551576;
    
    // Allocate buffer for actual model size
    g_model_buffer = (int8_t*)malloc(model_size);
    if (!g_model_buffer) {
        printf("[%s] Failed to allocate model buffer (%d bytes)\n", TAG, model_size);
        return CVI_FAILURE;
    }
    
    printf("[%s] Reading complete model from weight partition (0x%X)...\n", TAG, WEIGHT_PARTITION_ADDR);
    
    // Read complete model data
    ret = csi_spiflash_read(&g_spiflash_handle, WEIGHT_PARTITION_ADDR, 
                           g_model_buffer, model_size);
    if (ret < 0) {
        printf("[%s] Failed to read model data: %d\n", TAG, ret);
        free(g_model_buffer);
        g_model_buffer = NULL;
        return CVI_FAILURE;
    }
    
    printf("[%s] ✅ Model data read from weight partition (%d bytes)\n", TAG, model_size);
    
    ret = CVI_TDL_OpenModel_FromBuffer(handle, 
                                      CVI_TDL_SUPPORTED_MODEL_RETINAFACE, 
                                      g_model_buffer,
                                      model_size);
    
    if (ret == CVI_SUCCESS) {
        printf("[%s] ✅ Model loaded successfully from weight partition!\n", TAG);
        // Note: Don't free g_model_buffer here - TDL keeps reference to it
        return CVI_SUCCESS;
    } else {
        printf("[%s] ❌ Failed to load model from buffer: %d\n", TAG, ret);
        free(g_model_buffer);
        g_model_buffer = NULL;
        return CVI_FAILURE;
    }
}

/**
 * Initialize face detection system
 */
int face_detection_init(void)
{
    CVI_S32 ret = CVI_SUCCESS;
    
    printf("[%s] Initializing face detection system...\n", TAG);
    
    // Initialize TPU
    cvi_tpu_init();
    // if (ret != CVI_SUCCESS) {
    //     printf("[%s] TPU initialization failed: %d\n", TAG, ret);
    //     return ret;
    // }
    
    // Initialize spiflash for weight partition access
    ret = init_weight_partition_access();
    if (ret != CVI_SUCCESS) {
        printf("[%s] Spiflash initialization failed: %d\n", TAG, ret);
        cvi_tpu_deinit();
        return ret;
    }
    
    // Wait for TPU to be ready
    aos_msleep(1000);
    
    // Create TDL handle
    ret = CVI_TDL_CreateHandle2(&g_face_detection_handle, FACE_DETECTION_VPSS_GRP, 0);
    if (ret != CVI_SUCCESS) {
        printf("[%s] TDL handle creation failed: %d\n", TAG, ret);
        csi_spiflash_spi_uninit(&g_spiflash_handle);
        cvi_tpu_deinit();
        return ret;
    }
    
    // Load model from weight partition using FromBuffer
    ret = load_model_from_weight_partition(g_face_detection_handle);
    if (ret != CVI_SUCCESS) {
        printf("[%s] Failed to load model from weight partition\n", TAG);
        CVI_TDL_DestroyHandle(g_face_detection_handle);
        csi_spiflash_spi_uninit(&g_spiflash_handle);
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
    
    // Cleanup model buffer (TDL should have freed it, but be safe)
    if (g_model_buffer) {
        free(g_model_buffer);
        g_model_buffer = NULL;
    }
    
    // Deinitialize spiflash
    csi_spiflash_spi_uninit(&g_spiflash_handle);
    
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
