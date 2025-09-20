#!/bin/bash

# Download RetinaFace model for face detection
# This script downloads the required AI model for face detection

MODEL_URL="https://github.com/CVITEK/cvitek_mlir/releases/download/v1.0.0/retinaface_mnet0.25_342_608.cvimodel"
MODEL_PATH="/mnt/sd/retinaface_mnet0.25_342_608.cvimodel"
SD_MOUNT_POINT="/mnt/sd"

echo "🔽 Downloading RetinaFace model for face detection..."

# Check if SD card is mounted
if [ ! -d "$SD_MOUNT_POINT" ]; then
    echo "❌ SD card not mounted at $SD_MOUNT_POINT"
    echo "💡 Please mount SD card first or change MODEL_PATH in face_detection.c"
    exit 1
fi

# Create directory if it doesn't exist
mkdir -p "$SD_MOUNT_POINT"

# Download model if it doesn't exist
if [ ! -f "$MODEL_PATH" ]; then
    echo "📥 Downloading model from: $MODEL_URL"
    wget -O "$MODEL_PATH" "$MODEL_URL"
    
    if [ $? -eq 0 ]; then
        echo "✅ Model downloaded successfully to: $MODEL_PATH"
        ls -lh "$MODEL_PATH"
    else
        echo "❌ Failed to download model"
        exit 1
    fi
else
    echo "✅ Model already exists: $MODEL_PATH"
    ls -lh "$MODEL_PATH"
fi

echo ""
echo "🎯 Face detection model setup complete!"
echo "📝 Model location: $MODEL_PATH"
echo "🚀 You can now build and run the USB camera with face detection"
