/*
 * UVC Adapter
 * Solution-level adapter that bridges UVC component callbacks to solution modules
 */

#include <stdio.h>
#include "usbd_uvc_callback.h"
#include "face_detection.h"

/**
 * UVC frame callback adapter
 * Bridges UVC component callbacks to solution-level modules
 */
void uvc_frame_callback_adapter(VIDEO_FRAME_INFO_S *frame, VPSS_CHN_ATTR_S *chn_attr)
{
    printf("[uvc_adapter] UVC frame callback adapter - Frame size: %dx%d\n", 
           frame->stVFrame.u32Width, frame->stVFrame.u32Height);
    
    // Process face detection if initialized
    if (face_detection_is_initialized()) {
        face_detection_result_t results[MAX_FACES];
        int num_faces = face_detection_process_frame(frame, chn_attr, results, MAX_FACES);
        
        if (num_faces > 0) {
            // Draw bounding boxes on the frame
            face_detection_draw_boxes(frame, chn_attr, results, num_faces);
            
            // Print statistics
            int total_faces, total_frames;
            face_detection_get_stats(&total_faces, &total_frames);
            printf("[uvc_adapter] Face detection stats: %d faces in %d frames\n", 
                   total_faces, total_frames);
        }
    } else {
        printf("[uvc_adapter] Face detection not initialized, skipping...\n");
    }
}

/**
 * Initialize UVC adapter
 * Registers the adapter callback with UVC component and initializes face detection
 */
int uvc_adapter_init(void)
{
    printf("[uvc_adapter] Initializing UVC adapter...\n");
    
    // Initialize face detection module
    int ret = face_detection_init();
    if (ret != CVI_SUCCESS) {
        printf("[uvc_adapter] Failed to initialize face detection: %d\n", ret);
        return ret;
    }
    
    // Register adapter callback with UVC component
    ret = uvc_frame_callback_register(uvc_frame_callback_adapter);
    if (ret != CVI_SUCCESS) {
        printf("[uvc_adapter] Failed to register UVC callback: %d\n", ret);
        face_detection_deinit();
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
    
    // Deinitialize face detection module
    face_detection_deinit();
    
    printf("[uvc_adapter] UVC adapter deinitialized!\n");
}
