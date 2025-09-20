# 🎯 Face Detection Feature for USB Camera

## 📋 Overview

This enhancement adds **real-time face detection** capability to the USB camera solution. When faces are detected in the video stream, the system will print detailed messages with face coordinates, confidence scores, and statistics.

## 🚀 Features

- ✅ **Real-time face detection** using RetinaFace model
- ✅ **TPU hardware acceleration** for fast processing
- ✅ **Detailed face information** (position, confidence)
- ✅ **Detection statistics** (total faces, detection rate)
- ✅ **Thread-safe implementation** with mutex protection
- ✅ **Automatic model loading** and error handling

## 🏗️ Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Video Stream  │───▶│   VPSS Channel  │───▶│ Face Detection  │
│   (USB Camera)  │    │   (Group 15)    │    │   Thread        │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                      │
                                                      ▼
                                              ┌─────────────────┐
                                              │   Print Face    │
                                              │   Messages      │
                                              └─────────────────┘
```

## 📁 Files Added/Modified

### **New Files:**
- `customization/chenhan/src/face_detection.c` - Main face detection implementation
- `customization/chenhan/include/face_detection.h` - Header file
- `script/download_face_model.sh` - Model download script
- `FACE_DETECTION_README.md` - This documentation

### **Modified Files:**
- `package.yaml` - Added AI/TPU support configurations
- `customization/chenhan/src/custom_event.c` - Integrated face detection startup

## ⚙️ Configuration

### **Required Configurations in `package.yaml`:**
```yaml
CONFIG_SUPPORT_TPU: 1              # Enable TPU hardware
CONFIG_APP_AI_SUPPORT: 1           # Enable AI features
CONFIG_ALGOKIT_ENABLE: 1           # Enable AlgoKit
CONFIG_ALGOKIT_SEV3X_SUPPORT: 1    # Enable SEV3X algorithms
```

### **VPSS Configuration:**
- **Group**: 15
- **Channel**: 2
- **Timeout**: 2000ms
- **Frame Rate**: ~30 FPS

## 🤖 AI Model

### **Model Information:**
- **Name**: RetinaFace MNet0.25
- **Input Size**: 342x608
- **Format**: CVIModel (.cvimodel)
- **Size**: ~2-3MB

### **Model Storage Locations (Priority Order):**

#### **🥇 Flash Storage (RECOMMENDED):**
```bash
# Persistent storage in weight partition
/mnt/flash/weight/retinaface_mnet0.25_342_608.cvimodel
```

**Advantages:**
- ✅ **Persistent** - survives reboots
- ✅ **Fast access** - no SD card dependency
- ✅ **Dedicated partition** - 8.3MB available
- ✅ **Always available**

#### **🥈 SD Card (Fallback):**
```bash
# SD card storage
/mnt/sd/retinaface_mnet0.25_342_608.cvimodel
```

### **Setup Model:**

#### **Option 1: Flash Storage (Recommended):**
```bash
# Setup model in flash weight partition
./script/setup_flash_model.sh
```

#### **Option 2: SD Card:**
```bash
# Download to SD card
./script/download_face_model.sh
```

#### **Option 3: Manual Setup:**
```bash
# Download directly to flash
wget -O /mnt/flash/weight/retinaface_mnet0.25_342_608.cvimodel \
  https://github.com/CVITEK/cvitek_mlir/releases/download/v1.0.0/retinaface_mnet0.25_342_608.cvimodel

# Or to SD card
wget -O /mnt/sd/retinaface_mnet0.25_342_608.cvimodel \
  https://github.com/CVITEK/cvitek_mlir/releases/download/v1.0.0/retinaface_mnet0.25_342_608.cvimodel
```

## 🔧 Build Instructions

### **1. Enable Face Detection:**
```bash
cd /home/xf/work/codes/cvi_alios_open/solutions/usb_cam
```

### **2. Build with Face Detection:**
```bash
make PROJECT="chenhan"
```

### **3. Flash Firmware:**
```bash
# Use your preferred burning method
python3 /home/xf/work/codes/cvi_alios_open/host-tools/usb_cdc_ota/usb_cdc_ota.py
```

## 📊 Output Messages

### **System Initialization:**
```
[face_detection] Initializing face detection system...
[face_detection] Face detection system initialized successfully!
[face_detection] Face detection thread started!
✅ Face detection system started successfully!
🎯 Monitoring video stream for faces...
```

### **Face Detection Results:**
```
🎯 [FACE DETECTED] 2 face(s) found in frame!
   Face 1: Position=(120.5,80.2,200.3,180.7) Confidence=0.892
   Face 2: Position=(300.1,150.4,380.6,250.8) Confidence=0.756
   📊 Total faces detected so far: 15
   📈 Detection rate: 85.7% (12/14 frames)
```

### **Statistics:**
```
[face_detection] Face detection stopped!
[face_detection] Final statistics: 25 faces detected in 100 frames
```

## 🐛 Troubleshooting

### **Common Issues:**

#### **1. Model Not Found:**
```
[face_detection] RetinaFace model loading failed: -1
[face_detection] Please ensure model file exists: /mnt/sd/retinaface_mnet0.25_342_608.cvimodel
```
**Solution**: Download the model using the provided script

#### **2. TPU Initialization Failed:**
```
[face_detection] TPU initialization failed: -1
```
**Solution**: Ensure TPU is enabled in configuration and hardware is working

#### **3. VPSS Frame Timeout:**
```
[face_detection] Face detection failed: -1
```
**Solution**: Check VPSS configuration and video stream availability

### **Debug Commands:**
```bash
# Check if model exists
ls -la /mnt/sd/retinaface_mnet0.25_342_608.cvimodel

# Check TPU status
cat /proc/cvi-tpu/status

# Monitor face detection logs
tail -f /var/log/messages | grep face_detection
```

## 🔄 Integration Points

### **Startup Sequence:**
1. `main()` → `APP_CustomEventStart()`
2. `APP_CustomEventStart()` → `face_detection_start()`
3. `face_detection_start()` → `face_detection_init()` + thread creation
4. Thread runs continuously processing video frames

### **Shutdown Sequence:**
1. System shutdown signal
2. `face_detection_stop()` called
3. Thread stops, resources cleaned up
4. Statistics printed

## 📈 Performance

### **Expected Performance:**
- **Detection Rate**: ~30 FPS
- **Latency**: <50ms per frame
- **Memory Usage**: ~10MB for model + buffers
- **CPU Usage**: Low (TPU accelerated)

### **Optimization Tips:**
- Use SD card with fast read speed for model loading
- Ensure adequate cooling for TPU operation
- Monitor memory usage during long operation

## 🎯 Future Enhancements

- [ ] Face recognition (not just detection)
- [ ] Multiple face tracking
- [ ] Face quality assessment
- [ ] Integration with USB video streaming
- [ ] Web interface for statistics
- [ ] Face database management

## 📞 Support

For issues or questions:
1. Check this documentation first
2. Review system logs for error messages
3. Verify model file and TPU status
4. Test with known good video input

---

**Happy Face Detecting! 🎉**
