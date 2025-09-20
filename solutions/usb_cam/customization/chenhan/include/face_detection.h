/*
 * Face Detection Module Header for USB Camera
 */

#ifndef __FACE_DETECTION_H__
#define __FACE_DETECTION_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize face detection system
 * @return CVI_SUCCESS on success, error code on failure
 */
int face_detection_init(void);

/**
 * Deinitialize face detection system
 */
void face_detection_deinit(void);

/**
 * Start face detection thread
 * @return CVI_SUCCESS on success, error code on failure
 */
int face_detection_start(void);

/**
 * Stop face detection thread
 */
void face_detection_stop(void);

/**
 * Get face detection statistics
 * @param total_faces Output parameter for total faces detected
 * @param total_frames Output parameter for total frames processed
 */
void face_detection_get_stats(int *total_faces, int *total_frames);

#ifdef __cplusplus
}
#endif

#endif /* __FACE_DETECTION_H__ */
