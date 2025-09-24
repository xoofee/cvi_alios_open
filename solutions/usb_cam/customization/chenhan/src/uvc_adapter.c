/*
 * UVC Adapter
 * Solution-level adapter that bridges UVC component callbacks to solution modules
 */

#include <stdio.h>
#include "usbd_uvc_callback.h"
#include "brightness_analyzer.h"

/**
 * UVC frame callback adapter
 * Bridges UVC component callbacks to solution-level modules
 */
void uvc_frame_callback_adapter(VIDEO_FRAME_INFO_S *frame, VPSS_CHN_ATTR_S *chn_attr)
{
    printf("[uvc_adapter] UVC frame callback adapter\n");
    // Forward to brightness analyzer
    // brightness_analyzer_process_frame(frame, chn_attr);
    
    uint64_t brightness_sum = 0;
    int pixel_count = 0;
    
    // Process YUYV format: Y0 U0 Y1 V0 Y2 U1 Y3 V1 ...
    // We only care about Y (luminance) values at even indices
    uint8_t *yuyv_data = (uint8_t*)frame->stVFrame.u64PhyAddr[0];
    for (int i = 0; i < frame->stVFrame.u32Width * frame->stVFrame.u32Height * 2; i += 2) {
        brightness_sum += yuyv_data[i];  // Y value
        pixel_count++;
    }
    
    float brightness = brightness_sum / pixel_count;
    printf("[uvc_adapter] Brightness: %f\n", brightness);

    // Future: Add other analyzers here
    // face_detector_process_frame(frame, chn_attr);
    // motion_detector_process_frame(frame, chn_attr);
}

/**
 * Initialize UVC adapter
 * Registers the adapter callback with UVC component
 */
int uvc_adapter_init(void)
{
    printf("[uvc_adapter] Initializing UVC adapter...\n");
    
    // Register adapter callback with UVC component
    int ret = uvc_frame_callback_register(uvc_frame_callback_adapter);
    if (ret != CVI_SUCCESS) {
        printf("[uvc_adapter] Failed to register UVC callback: %d\n", ret);
        return ret;
    }
    
    printf("[uvc_adapter] UVC adapter initialized successfully!\n");
    return CVI_SUCCESS;
}

/**
 * Deinitialize UVC adapter
 */
void uvc_adapter_deinit(void)
{
    printf("[uvc_adapter] Deinitializing UVC adapter...\n");
    
    // Unregister callback from UVC component
    uvc_frame_callback_unregister();
    
    printf("[uvc_adapter] UVC adapter deinitialized!\n");
}
