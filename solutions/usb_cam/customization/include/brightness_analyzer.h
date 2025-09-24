/*
 * Brightness Analyzer Header
 a strange file that cause usb uvc camera to fail
 */

#ifndef __BRIGHTNESS_ANALYZER_H__
#define __BRIGHTNESS_ANALYZER_H__

#include "cvi_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize brightness analyzer
 * @return CVI_SUCCESS on success, CVI_FAILURE on error
 */
int brightness_analyzer_init(void);

/**
 * Deinitialize brightness analyzer
 */
void brightness_analyzer_deinit(void);

/**
 * Process frame directly - called by UVC adapter
 * @param frame Video frame data
 * @param chn_attr VPSS channel attributes
 */
void brightness_analyzer_process_frame(VIDEO_FRAME_INFO_S *frame, VPSS_CHN_ATTR_S *chn_attr);

/**
 * Get brightness analyzer statistics
 * @param total_frames Total number of frames processed
 * @param avg_brightness Average brightness value
 * @param min_brightness Minimum brightness value
 * @param max_brightness Maximum brightness value
 */
void brightness_analyzer_get_stats(int *total_frames, float *avg_brightness, 
                                  float *min_brightness, float *max_brightness);

#ifdef __cplusplus
}
#endif

#endif /* __BRIGHTNESS_ANALYZER_H__ */
