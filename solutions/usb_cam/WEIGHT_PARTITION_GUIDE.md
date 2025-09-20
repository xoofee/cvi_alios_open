# 🎯 **Weight Partition Guide: Putting .cvimodel in Flash**

## **📋 Overview**

The CV181x has a dedicated **weight partition** (8.3125 MB) specifically designed for AI models. This guide shows you how to put your `.cvimodel` file in the weight partition and load it in code.

## **🏗️ Weight Partition Details**

From `config.yaml`:
```yaml
- { name: weight, address: 0x4DE000, size: 0x850000, index: 0 }  # 8.3125 MB
```

- **Address**: 0x4DE000 - 0xD2E000
- **Size**: 8.3125 MB (plenty for AI models)
- **Purpose**: Dedicated storage for AI models and weights
- **Mount Point**: `/mnt/flash/weight/` (at runtime)

## **📁 Step 1: Place Your Model File**

### **Method A: During Build (Recommended)**

1. **Place your model in the models directory:**
```bash
# Copy your .cvimodel file to the models directory
cp your_model.cvimodel /home/xf/work/codes/cvi_alios_open/solutions/usb_cam/models/retinaface_mnet0.25_342_608.cvimodel
```

2. **The build script will automatically copy it:**
```bash
# From aft_build.sh - this happens automatically during build
cp ../../models/retinaface_mnet0.25_342_608.cvimodel $MK_GENERATED_PATH/data/
```

### **Method B: Manual Copy**

```bash
# Copy directly to generated images
cp your_model.cvimodel /home/xf/work/codes/cvi_alios_open/solutions/usb_cam/generated/images/data/retinaface_mnet0.25_342_608.cvimodel
```

## **🔧 Step 2: Build and Burn**

### **Complete Process:**
```bash
cd /home/xf/work/codes/cvi_alios_open/solutions/usb_cam

# 1. Place your model (if not already done)
cp your_model.cvimodel models/retinaface_mnet0.25_342_608.cvimodel

# 2. Build the solution
make clean
make PROJECT="chenhan"

# 3. Burn firmware (includes your model in weight partition)
python3 /home/xf/work/codes/cvi_alios_open/host-tools/usb_cdc_ota/usb_cdc_ota.py
```

## **💻 Step 3: Code Loading**

The face detection code automatically tries to load from the weight partition:

```c
// Priority order for model loading
const char* model_paths[] = {
    "/mnt/flash/weight/retinaface_mnet0.25_342_608.cvimodel",  // Weight partition
    "/mnt/sd/retinaface_mnet0.25_342_608.cvimodel",            // SD card fallback
    NULL
};
```

## **📊 What Happens During Burning**

1. **Build Process**: Your model gets copied to `generated/images/data/`
2. **Image Creation**: Model is included in the firmware image
3. **Weight Partition**: Model gets burned into the weight partition (0x4DE000)
4. **Runtime Mount**: Weight partition mounts as `/mnt/flash/weight/`
5. **Model Loading**: Code loads model from `/mnt/flash/weight/`

## **🎯 Advantages of Weight Partition**

| Feature | Weight Partition | SD Card |
|---------|------------------|---------|
| **Speed** | ⚡ Fast (flash) | 🐌 Slower |
| **Persistence** | ✅ Permanent | ✅ Permanent |
| **Dependency** | ❌ None | ✅ SD Card |
| **Size** | 8.3MB | Unlimited |
| **Reliability** | ✅ High | ⚠️ Medium |

## **🔍 Verification**

After burning, check if your model is in the weight partition:

```bash
# Connect to device via serial/UART
# Check weight partition mount
ls -la /mnt/flash/weight/

# Check your model file
ls -la /mnt/flash/weight/retinaface_mnet0.25_342_608.cvimodel

# Check file size
du -h /mnt/flash/weight/retinaface_mnet0.25_342_608.cvimodel
```

## **📱 Expected Output**

When the device boots with your model in the weight partition:

```
[face_detection] Trying to load model from: /mnt/flash/weight/retinaface_mnet0.25_342_608.cvimodel
[face_detection] ✅ Model loaded successfully from: /mnt/flash/weight/retinaface_mnet0.25_342_608.cvimodel
[face_detection] Face detection system initialized successfully!
✅ Face detection system started successfully!
🎯 Monitoring video stream for faces...
```

## **🐛 Troubleshooting**

### **Model Not Found:**
```
[face_detection] ❌ Failed to load from: /mnt/flash/weight/... (error: -1)
[face_detection] Trying to load model from: /mnt/sd/retinaface_mnet0.25_342_608.cvimodel
```

**Solutions:**
1. Check if model file exists in `models/` directory
2. Verify build process copied the model
3. Check weight partition mount: `ls /mnt/flash/weight/`

### **Weight Partition Not Mounted:**
```
ls: /mnt/flash/weight/: No such file or directory
```

**Solution:** Weight partition should mount automatically. Check system logs for mount errors.

### **File Size Issues:**
```
[face_detection] Model file seems too small (X bytes)
```

**Solution:** Verify your model file is complete and not corrupted.

## **💡 Pro Tips**

1. **Model Naming**: Use exact filename `retinaface_mnet0.25_342_608.cvimodel`
2. **File Size**: RetinaFace models are typically 2-3MB (well within 8.3MB limit)
3. **Build Order**: Always place model before building
4. **Verification**: Check model exists in weight partition after burning
5. **Fallback**: SD card path provides backup if weight partition fails

## **🚀 Quick Start**

```bash
# 1. Place your model
cp your_model.cvimodel /home/xf/work/codes/cvi_alios_open/solutions/usb_cam/models/retinaface_mnet0.25_342_608.cvimodel

# 2. Build and burn
cd /home/xf/work/codes/cvi_alios_open/solutions/usb_cam
make PROJECT="chenhan"
python3 /home/xf/work/codes/cvi_alios_open/host-tools/usb_cdc_ota/usb_cdc_ota.py

# 3. Enjoy face detection from weight partition! 🎉
```

Your `.cvimodel` file will be permanently stored in the weight partition and loaded automatically on every boot!
