/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: face_detection.h
 * Description: Face detection module for USB camera solution
 */

#ifndef __FACE_DETECTION_H__
#define __FACE_DETECTION_H__

#include "cvi_vpss.h"
#include "cvi_tdl.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// Face detection configuration
#define FACE_DETECTION_VPSS_GRP 2  // Use VPSS group 2 for face detection
#define FACE_DETECTION_VPSS_CHN 1  // Use channel 1 for face detection
#define FACE_DETECTION_MODEL_WIDTH 608
#define FACE_DETECTION_MODEL_HEIGHT 342
#define FACE_DETECTION_TIMEOUT 2000
#define MAX_FACES 10

// Face detection result structure
typedef struct {
    float x1, y1, x2, y2;  // Bounding box coordinates (normalized 0-1)
    float confidence;       // Detection confidence
    int valid;             // Whether this detection is valid
} face_detection_result_t;

// Face detection context
typedef struct {
    cvitdl_handle_t tdl_handle;
    CVI_BOOL initialized;
    CVI_BOOL vpss_initialized;
    VPSS_GRP_ATTR_S vpss_grp_attr;
    VPSS_CHN_ATTR_S vpss_chn_attr;
    pthread_mutex_t mutex;
    int total_faces_detected;
    int detection_frames;
} face_detection_context_t;

/**
 * Initialize face detection module
 * @return CVI_SUCCESS on success, error code on failure
 */
CVI_S32 face_detection_init(void);

/**
 * Deinitialize face detection module
 */
void face_detection_deinit(void);

/**
 * Process frame for face detection
 * @param frame Input video frame
 * @param chn_attr Channel attributes
 * @param results Output array for detection results
 * @param max_results Maximum number of results to return
 * @return Number of faces detected, -1 on error
 */
int face_detection_process_frame(VIDEO_FRAME_INFO_S *frame, VPSS_CHN_ATTR_S *chn_attr,
                                 face_detection_result_t *results, int max_results);

/**
 * Draw bounding boxes on frame
 * @param frame Video frame to draw on
 * @param chn_attr Channel attributes
 * @param results Face detection results
 * @param num_results Number of results
 */
void face_detection_draw_boxes(VIDEO_FRAME_INFO_S *frame, VPSS_CHN_ATTR_S *chn_attr,
                               face_detection_result_t *results, int num_results);

/**
 * Get face detection statistics
 * @param total_faces Total faces detected
 * @param total_frames Total frames processed
 */
void face_detection_get_stats(int *total_faces, int *total_frames);

/**
 * Check if face detection is initialized
 * @return CVI_TRUE if initialized, CVI_FALSE otherwise
 */
CVI_BOOL face_detection_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif /* __FACE_DETECTION_H__ */