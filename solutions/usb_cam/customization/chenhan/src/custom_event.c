#include <stdio.h>
#include <unistd.h>
#include "custom_event.h"
#include "cvi_tpu_interface.h"
#include "rtsp_func.h"
#include "gui_display.h"
#include "app_ai.h"
#include "rtsp_func.h"
#include "face_detection.h"

int APP_CustomEventStart(void)
{
    printf("%s %d.\n", __FUNCTION__, __LINE__);
#if (CONFIG_APP_RTSP_SUPPORT == 1)
    cvi_rtsp_init();
#endif

#if (CONFIG_APP_AI_SUPPORT == 1)
    // Start face detection system
    printf("Starting face detection system...\n");
    if (face_detection_start() == CVI_SUCCESS) {
        printf("✅ Face detection system started successfully!\n");
        printf("🎯 Monitoring video stream for faces...\n");
    } else {
        printf("❌ Failed to start face detection system!\n");
        printf("💡 Make sure RetinaFace model is available at: %s\n", RETINA_FACE_MODEL_PATH);
    }
#endif
    
/*#if (CONFIG_SENSOR_GCORE_GC2083_1L == 1)
    cvi_irled_init();
#endif*/
    return 0;
}
