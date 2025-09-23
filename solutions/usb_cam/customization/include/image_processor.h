/*
 * Real-time Image Processor Header
 */

#ifndef __IMAGE_PROCESSOR_H__
#define __IMAGE_PROCESSOR_H__

#include "cvi_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize image processor
 * @return CVI_SUCCESS on success, CVI_FAILURE on error
 */
int image_processor_init(void);

/**
 * Start image processor thread
 * @return CVI_SUCCESS on success, CVI_FAILURE on error
 */
int image_processor_start(void);

/**
 * Stop image processor
 */
void image_processor_stop(void);

/**
 * Get image processor statistics
 * @param total_frames Total number of frames processed
 * @param avg_brightness Average brightness value
 * @param min_brightness Minimum brightness value
 * @param max_brightness Maximum brightness value
 */
void image_processor_get_stats(int *total_frames, float *avg_brightness, 
                              float *min_brightness, float *max_brightness);

#ifdef __cplusplus
}
#endif

#endif /* __IMAGE_PROCESSOR_H__ */
