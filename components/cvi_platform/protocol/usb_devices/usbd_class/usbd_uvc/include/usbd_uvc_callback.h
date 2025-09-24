/*
 * UVC Callback Interface
 * Component-level interface for frame processing callbacks
 */

#ifndef __USBD_UVC_CALLBACK_H__
#define __USBD_UVC_CALLBACK_H__

#include "cvi_type.h"
#include "cvi_comm_vpss.h"


// Forward declarations
// typedef struct VIDEO_FRAME_INFO_S VIDEO_FRAME_INFO_S;
// typedef struct VPSS_CHN_ATTR_S VPSS_CHN_ATTR_S;

// Frame callback function type
typedef void (*uvc_frame_callback_func_t)(VIDEO_FRAME_INFO_S *frame, VPSS_CHN_ATTR_S *chn_attr);

/**
 * Register a frame callback function
 * @param callback Function to call when a new frame is available
 * @return CVI_SUCCESS on success, CVI_FAILURE on error
 */
int uvc_frame_callback_register(uvc_frame_callback_func_t callback);

/**
 * Unregister the frame callback function
 * @return CVI_SUCCESS on success, CVI_FAILURE on error
 */
int uvc_frame_callback_unregister(void);

/**
 * Check if callback is registered
 * @return CVI_TRUE if registered, CVI_FALSE otherwise
 */
int uvc_frame_callback_is_registered(void);

void uvc_frame_callback_notify(VIDEO_FRAME_INFO_S *frame, VPSS_CHN_ATTR_S *chn_attr);


#endif /* __USBD_UVC_CALLBACK_H__ */
