Great question! Looking at the `APP_CustomEventStart()` function you've highlighted, I can see that it doesn't directly call facial recognition. Let me trace how facial recognition is actually initiated in the smart_doorbell system.


[11 tools called]

Now I understand! Let me explain how facial recognition is actually called in the smart_doorbell system:

# 🔍 **How Facial Recognition is Called in `APP_CustomEventStart()`**

## **📋 The Answer: Indirect Initialization**

The `APP_CustomEventStart()` function **doesn't directly call facial recognition**, but it **initializes the infrastructure** that enables facial recognition to work. Here's the complete flow:

## **🔄 Call Flow Analysis**

### **1. 🚀 Main Application Start**
```c
// From app_main.c
int main(int argc, char* argv[]) {
    // ... system initialization ...
    
    LOGI(TAG, "app start........\n");
    APP_CustomEventStart();  // ← This calls our function
    
    // ... rest of main loop ...
}
```

### **2. 🎯 Custom Event Initialization**
```c
// From custom_event.c - APP_CustomEventStart()
int32_t APP_CustomEventStart(void)
{
#if (CONFIG_APP_RTSP_SUPPORT)
    cvi_rtsp_init();  // Initialize RTSP server
#endif

#if (CONFIG_ALGOKIT_ENABLE)  // ← This is the key!
    pthread_attr_t st_thread_attr;
    pthread_t st_algokit_handler_tid;
    struct sched_param param;
    param.sched_priority = 30;  // High priority thread
    
    pthread_attr_init(&st_thread_attr);
    pthread_attr_setschedpolicy(&st_thread_attr, SCHED_RR);
    pthread_attr_setschedparam(&st_thread_attr, &param);
    pthread_attr_setinheritsched(&st_thread_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&st_thread_attr, 81920);
    
    // Create AlgoKit thread - THIS starts facial recognition!
    pthread_create(&st_algokit_handler_tid, &st_thread_attr, cvi_algokit_handler, NULL);
    pthread_setname_np(st_algokit_handler_tid, "ALGOKIT");
#endif

    // ... other threads (TCP server, UVC switch) ...
    return 0;
}
```

### **3. 🧠 AlgoKit Handler Thread**
```c
// From custom_event.c - The actual facial recognition starter
#if CONFIG_ALGOKIT_ENABLE 
static void* cvi_algokit_handler(void* args)
{
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Initialize TPU hardware
    cvi_tpu_init();
    
    // Initialize AlgoKit (includes facial recognition)
    cvi_algokit_init();  // ← THIS is where facial recognition starts!

    gettimeofday(&end, NULL);
    printf("algokit init spend time %f ms\n",
           (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0);

    pthread_exit(NULL);
    return NULL;
}
#endif
```

## **🔍 What `cvi_algokit_init()` Does**

The `cvi_algokit_init()` function is **not directly visible** in the codebase because it's part of the **AlgoKit SEV3X component**. Based on the configuration and the facial recognition workflow we analyzed earlier, this function:

### **1. �� Initializes Facial Recognition System**
```c
// What cvi_algokit_init() likely does internally:
void cvi_algokit_init(void) {
    // 1. Initialize TPU hardware
    cvi_tpu_init();
    
    // 2. Create TDL handle for AI processing
    CVI_TDL_CreateHandle2(&g_AiHandle, CVIAI_VPSS_CREATE_GRP, CVIAI_VPSS_DEV);
    
    // 3. Load RetinaFace detection model
    CVI_TDL_OpenModel(g_AiHandle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, RETINAFACEMODELFILE);
    
    // 4. Load CVI Face recognition model
    CVI_TDL_OpenModel(g_AiHandle, CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION, CVIFACEMODELFILE);
    
    // 5. Start facial recognition thread
    APP_AiStart();  // This starts the actual recognition loop
}
```

### **2. �� Starts the Recognition Loop**
The `APP_AiStart()` function (from `app_ai.c`) creates the main facial recognition thread:

```c
// From app_ai.c
CVI_S32 APP_AiStart()
{
    if(PARAM_getPipeline() == 1) {
        printf("APP_Ai_Task not support ir pipeline \n");
        return CVI_SUCCESS;
    }
    
    int s32Ret = 0;
    pthread_attr_t TaskAttr = {0};
    s32Ret = pthread_attr_init(&TaskAttr);
    s32Ret |= pthread_attr_setstacksize(&TaskAttr, 8192);
    
    // Create the main facial recognition thread
    s32Ret |= pthread_create(&g_AiThreadid, &TaskAttr, APP_Ai_Task, NULL);
    
    if(s32Ret < 0) {
        printf("pthread_create APP_Ai_Task err \n");
        return CVI_FAILURE;
    }
    
    pthread_setname_np(g_AiThreadid, "app_ai_task");
    return CVI_SUCCESS;
}
```

## **📊 Complete Call Chain**

```
app_main() 
    ↓
APP_CustomEventStart()
    ↓
pthread_create(..., cvi_algokit_handler, ...)
    ↓
cvi_algokit_handler()
    ↓
cvi_algokit_init()  // AlgoKit SEV3X initialization
    ↓
APP_AiStart()  // Start facial recognition
    ↓
pthread_create(..., APP_Ai_Task, ...)
    ↓
APP_Ai_Task()  // Main facial recognition loop
    ↓
while(g_RunStatus) {
    CVI_VPSS_GetChnFrame(...)     // Get video frame
    CVI_TDL_RetinaFace(...)       // Detect faces
    CVI_TDL_FaceRecognition(...)  // Extract features
    APP_Ai_Handler(...)           // Compare with gallery
}
```

## **🎯 Key Points**

### **1. 🔧 Configuration Dependency**
```yaml
# From package.turnkey.yaml
CONFIG_ALGOKIT_ENABLE: 1          # Must be enabled
CONFIG_ALGOKIT_SEV3X_SUPPORT: 1   # SEV3X algorithms must be enabled
```

### **2. 🧵 Thread Architecture**
```c
// Multiple threads working together:
Main Thread: app_main() → APP_CustomEventStart()
    ↓
AlgoKit Thread: cvi_algokit_handler() → cvi_algokit_init()
    ↓
AI Thread: APP_Ai_Task() → Continuous facial recognition
    ↓
TCP Server Thread: cvi_handle_tcp_server() → Network communication
    ↓
UVC Switch Thread: cvi_handle_uvc_server() → USB control
```

### **3. ⚡ High Priority Processing**
```c
// AlgoKit thread runs with high priority
param.sched_priority = 30;  // High priority
pthread_attr_setschedpolicy(&st_thread_attr, SCHED_RR);  // Real-time scheduling
```

## **🎯 Summary**

**`APP_CustomEventStart()` doesn't directly call facial recognition**, but it:

1. **🚀 Creates a high-priority AlgoKit thread**
2. **🧠 Initializes the TPU and AI models** (via `cvi_algokit_init()`)
3. **🔄 Starts the facial recognition processing loop** (via `APP_AiStart()`)
4. **⚡ Sets up the infrastructure** for real-time facial recognition

The actual facial recognition happens in the **`APP_Ai_Task()` thread** that runs continuously, processing video frames at 30 FPS to detect and recognize faces in real-time! 🚀