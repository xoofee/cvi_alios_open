# Face Detection Implementation for Chenhan USB Camera

This document describes the face detection feature implemented for the chenhan USB camera solution.

## Overview

The face detection system provides real-time face detection capabilities integrated with the UVC video stream. It uses hardware-accelerated VPSS for efficient frame preprocessing and the CVI TDL SDK for AI inference.

## Architecture

### Data Flow
```
Camera Sensor (GC2093)
    ↓ Raw Bayer Data (12-bit)
VI Device (MIPI Interface)
    ↓ Raw Bayer Data
VI Pipe (ISP Processing)
    ↓ Processed RGB/YUV Data
VI Channel (Format Conversion)
    ↓ YUV 4:2:0 Planar (1920x1080)
VPSS Group 0/1 (UVC Output)
    ↓ Full Size YUYV/NV21
UVC Component
    ↓ USB Video Stream
PC Camera View

VPSS Group 2 (Face Detection)
    ↓ Resized RGB (320x240)
Face Detection Model
    ↓ Bounding Boxes
UVC Adapter
    ↓ Overlay on Video Stream
```

### Components

1. **Face Detection Module** (`face_detection.c/h`)
   - Manages VPSS preprocessing pipeline
   - Loads AI model from weight partition
   - Performs face detection inference
   - Provides statistics and results

2. **UVC Adapter** (`uvc_adapter.c/h`)
   - Bridges UVC component callbacks to solution modules
   - Integrates face detection with video stream
   - Handles initialization and cleanup

3. **VPSS Configuration** (`custom_vpssparam.c`)
   - Configures VPSS Group 2 for face detection preprocessing
   - Resizes frames from full size to 320x240
   - Converts YUV to RGB format for AI model

4. **System Configuration** (`custom_sysparam.c`)
   - Adds VB pool for face detection buffers
   - Configures memory allocation for RGB frames

## Configuration

### VPSS Configuration
- **Group 2**: Face detection preprocessing
- **Input**: Full size frames (up to 1920x1080)
- **Output**: 320x240 RGB planar format
- **Mode**: Manual (not bound to VI)

### VB Pool Configuration
- **Pool 2**: Face detection buffers
- **Size**: 320x240 RGB planar
- **Blocks**: 3 buffers for processing pipeline

### Model Configuration
- **Model**: RetinaFace face detection
- **Input Size**: 320x240 RGB
- **Source**: Weight partition (0x4DE000)
- **Size**: 551,576 bytes

## API Reference

### Face Detection Module

#### `face_detection_init()`
Initializes the face detection module, including VPSS setup and model loading.

```c
CVI_S32 face_detection_init(void);
```

**Returns**: `CVI_SUCCESS` on success, error code on failure.

#### `face_detection_deinit()`
Deinitializes the face detection module and cleans up resources.

```c
void face_detection_deinit(void);
```

#### `face_detection_process_frame()`
Processes a video frame for face detection.

```c
int face_detection_process_frame(VIDEO_FRAME_INFO_S *frame, VPSS_CHN_ATTR_S *chn_attr,
                                 face_detection_result_t *results, int max_results);
```

**Parameters**:
- `frame`: Input video frame
- `chn_attr`: Channel attributes
- `results`: Output array for detection results
- `max_results`: Maximum number of results to return

**Returns**: Number of faces detected, -1 on error.

#### `face_detection_draw_boxes()`
Draws bounding boxes on the video frame.

```c
void face_detection_draw_boxes(VIDEO_FRAME_INFO_S *frame, VPSS_CHN_ATTR_S *chn_attr,
                               face_detection_result_t *results, int num_results);
```

#### `face_detection_get_stats()`
Gets face detection statistics.

```c
void face_detection_get_stats(int *total_faces, int *total_frames);
```

#### `face_detection_is_initialized()`
Checks if face detection is initialized.

```c
CVI_BOOL face_detection_is_initialized(void);
```

### UVC Adapter

#### `uvc_adapter_init()`
Initializes the UVC adapter and face detection.

```c
int uvc_adapter_init(void);
```

#### `uvc_adapter_deinit()`
Deinitializes the UVC adapter.

```c
void uvc_adapter_deinit(void);
```

## Data Structures

### `face_detection_result_t`
Face detection result structure.

```c
typedef struct {
    float x1, y1, x2, y2;  // Bounding box coordinates (normalized 0-1)
    float confidence;       // Detection confidence
    int valid;             // Whether this detection is valid
} face_detection_result_t;
```

### `face_detection_context_t`
Face detection context structure.

```c
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
```

## Testing

### CLI Commands

The implementation includes test commands for verification:

```bash
# Test face detection initialization
test_face_init

# Test face detection deinitialization
test_face_deinit

# Show face detection statistics
face_stats

# Show face detection status
face_status
```

### Expected Output

When working correctly, you should see:

```
[face_detection] Initializing face detection module...
[face_detection] Initializing spiflash for weight partition access...
[face_detection] ✅ Spiflash initialized for weight partition access
[face_detection] Loading face detection model from weight partition...
[face_detection] ✅ Model data read from weight partition (551576 bytes)
[face_detection] ✅ Face detection model loaded successfully!
[face_detection] Initializing VPSS for face detection preprocessing...
[face_detection] VPSS initialized for face detection: 1920x1080 -> 320x240
[face_detection] ✅ Face detection module initialized successfully!
[uvc_adapter] Initializing UVC adapter...
[uvc_adapter] UVC adapter initialized successfully!
```

During operation:
```
[uvc_adapter] UVC frame callback adapter - Frame size: 1920x1080
[face_detection] Detected 2 faces
[uvc_adapter] Face detection stats: 15 faces in 8 frames
```

## Performance Considerations

### Hardware Acceleration
- Uses VPSS for efficient frame resizing and format conversion
- Hardware-accelerated preprocessing reduces CPU load
- Dedicated VB pool for face detection buffers

### Memory Usage
- Small VB pool (3 buffers) for face detection
- Efficient memory management with proper cleanup
- Model loaded once and reused for all frames

### Processing Pipeline
- Asynchronous processing to avoid blocking UVC stream
- Thread-safe operations with mutex protection
- Configurable timeout for VPSS operations

## Troubleshooting

### Common Issues

1. **Model Loading Failure**
   - Verify weight partition is properly configured
   - Check model size and address in `WEIGHT_PARTITION_ADDR`
   - Ensure spiflash is accessible

2. **VPSS Initialization Failure**
   - Check VPSS group 2 configuration
   - Verify VB pool allocation
   - Ensure no conflicts with other VPSS groups

3. **No Face Detection**
   - Verify model is loaded correctly
   - Check input frame format and size
   - Ensure proper lighting conditions

### Debug Output

Enable debug output by checking the console logs:
- Face detection initialization status
- VPSS configuration details
- Model loading progress
- Detection statistics

## Future Enhancements

### Potential Improvements
1. **Multiple AI Models**: Support for different detection models
2. **Face Recognition**: Add face recognition capabilities
3. **Performance Optimization**: Further hardware acceleration
4. **Configuration**: Runtime configuration of detection parameters
5. **Visual Overlay**: Actual bounding box drawing on video frames

### Extensibility
The architecture is designed to be extensible:
- Modular design allows easy addition of new AI features
- Flexible VPSS configuration supports different input/output formats
- Clean API allows integration with other solution modules

## Integration Notes

### With Existing UVC System
- Integrates seamlessly with existing UVC implementation
- Uses callback mechanism for non-intrusive integration
- Maintains full compatibility with UVC video streaming

### With CVI TDL SDK
- Uses standard CVI TDL APIs for model management
- Leverages hardware acceleration for inference
- Follows CVI TDL best practices for memory management

This implementation provides a solid foundation for real-time face detection in the USB camera solution while maintaining good performance and extensibility.
