#!/bin/bash

# Setup RetinaFace model in flash partition for USB Camera
# This script copies the model to the weight partition for persistent storage

MODEL_NAME="retinaface_mnet0.25_342_608.cvimodel"
MODEL_URL="https://github.com/CVITEK/cvitek_mlir/releases/download/v1.0.0/${MODEL_NAME}"

# Flash storage paths
FLASH_WEIGHT_DIR="/mnt/flash/weight"
FLASH_MODEL_PATH="${FLASH_WEIGHT_DIR}/${MODEL_NAME}"

# SD card fallback paths  
SD_MOUNT_POINT="/mnt/sd"
SD_MODEL_PATH="${SD_MOUNT_POINT}/${MODEL_NAME}"

echo "🔧 Setting up RetinaFace model in flash storage..."

# Check if flash weight directory exists
if [ ! -d "$FLASH_WEIGHT_DIR" ]; then
    echo "❌ Flash weight directory not found: $FLASH_WEIGHT_DIR"
    echo "💡 This directory should be mounted from the weight partition"
    echo "💡 Check if weight partition is properly mounted"
    exit 1
fi

# Create weight directory if it doesn't exist
mkdir -p "$FLASH_WEIGHT_DIR"

# Check if model already exists in flash
if [ -f "$FLASH_MODEL_PATH" ]; then
    echo "✅ Model already exists in flash: $FLASH_MODEL_PATH"
    ls -lh "$FLASH_MODEL_PATH"
    exit 0
fi

# Try to copy from SD card first
if [ -f "$SD_MODEL_PATH" ]; then
    echo "📋 Copying model from SD card to flash..."
    cp "$SD_MODEL_PATH" "$FLASH_MODEL_PATH"
    
    if [ $? -eq 0 ]; then
        echo "✅ Model copied successfully to flash!"
        ls -lh "$FLASH_MODEL_PATH"
        exit 0
    else
        echo "❌ Failed to copy from SD card"
    fi
fi

# Download model directly to flash
echo "📥 Downloading model directly to flash storage..."
wget -O "$FLASH_MODEL_PATH" "$MODEL_URL"

if [ $? -eq 0 ]; then
    echo "✅ Model downloaded successfully to flash!"
    ls -lh "$FLASH_MODEL_PATH"
    
    # Verify file size (should be ~2-3MB)
    FILE_SIZE=$(stat -c%s "$FLASH_MODEL_PATH")
    if [ $FILE_SIZE -lt 1000000 ]; then
        echo "⚠️  Warning: Model file seems too small ($FILE_SIZE bytes)"
        echo "💡 Please verify the download was successful"
    fi
else
    echo "❌ Failed to download model to flash"
    exit 1
fi

echo ""
echo "🎯 Flash model setup complete!"
echo "📝 Model location: $FLASH_MODEL_PATH"
echo "💾 Storage: Flash weight partition (persistent)"
echo "🚀 Face detection will now use flash storage (faster, no SD card required)"
