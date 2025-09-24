/*
 * UVC Callback Implementation
 * Component-level callback system for frame processing
 */

#include <stdio.h>
#include "usbd_uvc_callback.h"

// Global callback function pointer
static uvc_frame_callback_func_t g_uvc_frame_callback = NULL;

int uvc_frame_callback_register(uvc_frame_callback_func_t callback)
{
    if (callback == NULL) {
        printf("[uvc_callback] Error: callback function is NULL\n");
        return CVI_FAILURE;
    }
    
    if (g_uvc_frame_callback != NULL) {
        printf("[uvc_callback] Warning: callback already registered, replacing...\n");
    }
    
    g_uvc_frame_callback = callback;
    printf("[uvc_callback] UVC frame callback registered successfully\n");
    
    return CVI_SUCCESS;
}

int uvc_frame_callback_unregister(void)
{
    if (g_uvc_frame_callback == NULL) {
        printf("[uvc_callback] Warning: no callback registered\n");
        return CVI_FAILURE;
    }
    
    g_uvc_frame_callback = NULL;
    printf("[uvc_callback] UVC frame callback unregistered\n");
    
    return CVI_SUCCESS;
}

int uvc_frame_callback_is_registered(void)
{
    return (g_uvc_frame_callback != NULL) ? CVI_TRUE : CVI_FALSE;
}

// Internal function to call the registered callback
void uvc_frame_callback_notify(VIDEO_FRAME_INFO_S *frame, VPSS_CHN_ATTR_S *chn_attr)
{
    if (g_uvc_frame_callback != NULL && frame != NULL && chn_attr != NULL) {
        g_uvc_frame_callback(frame, chn_attr);
    }
}
