#!/bin/sh

set -e

BASE_PWD=`pwd`
MK_GENERATED_PATH=generated

echo "[INFO] Generated output files ..."

# rm -fr $MK_GENERATED_PATH
# mkdir -p $MK_GENERATED_PATH/data/

OBJCOPY=riscv64-unknown-elf-objcopy

#output yoc.bin
# ELF_NAME=`ls Obj/*.elf`   // already created before aft_build.sh
# $OBJCOPY -O binary $ELF_NAME yoc.bin

# Copy yoc.bin to generated directory
cp yoc.bin $MK_GENERATED_PATH/data/

#Prepare bin
# BOARDS_CONFIG_PATH=$BOARD_PATH/configs
# BOOT0_BIN="${BOARD_PATH}/bootimgs/boot0"
# BOOT_BIN="${BOARD_PATH}/bootimgs/boot"

# cp ${BOOT0_BIN} $MK_GENERATED_PATH/data
# cp ${BOOT_BIN} $MK_GENERATED_PATH/data
# cp "$BOARDS_CONFIG_PATH/config.yaml" $MK_GENERATED_PATH/data/

# Copy fip.bin to generated images
echo "[INFO] Copy fip.bin to generated images"
cp ../../boards/tools/fip/181x_250918/fip.bin $MK_GENERATED_PATH/data/

# Copy AI model to weight partition
echo "[INFO] Copy AI model to weight partition"
if [ -f "../../models/retinaface_mnet0.25_342_608.cvimodel" ]; then
    
    # Create weight partition image
    echo "[INFO] Creating weight partition image..."
    MODEL_SIZE=$(stat -c%s "../../models/retinaface_mnet0.25_342_608.cvimodel")
    WEIGHT_SIZE=$((0x850000))  # 8.3125 MB (8724480 bytes)
    
    if [ $MODEL_SIZE -le $WEIGHT_SIZE ]; then
        cp ../../models/retinaface_mnet0.25_342_608.cvimodel $MK_GENERATED_PATH/data/weight.bin
        echo "[INFO] Model copied: retinaface_mnet0.25_342_608.cvimodel"

    else
        echo "[ERROR] Model file too large for weight partition!"
        echo "[ERROR] Model: $MODEL_SIZE bytes, Weight partition: $WEIGHT_SIZE bytes"
    fi
else
    echo "[WARN] Model file not found: ../../models/retinaface_mnet0.25_342_608.cvimodel"
    echo "[INFO] Please place your .cvimodel file in: ../../models/"
fi

CURDIR=${BASE_PWD}
HAASUI_SDK_DIR=$PATH_HAASUI_SDK
BOARD_DIR=$BOARD_PATH
OUT_DIR=${CURDIR}/out
RTOS_IMG=${CURDIR}/yoc.bin
FS_DATA_DIR=${CURDIR}/data
MK_GENERATED_IMGS_PATH=${CURDIR}/generated
CONFIG_YAML_FILE=$BOARDS_CONFIG_PATH/config.yaml

if [ ! -d $OUT_DIR ]; then
    mkdir $OUT_DIR
fi

if [ ! -d "${MK_GENERATED_IMGS_PATH}/data" ]; then
    echo "folder ${MK_GENERATED_IMGS_PATH}/data not find."
    exit 1
fi

# Create image files
echo "[INFO] Create bin files"
echo "[INFO] Create imtb bin file"

# need for a full image. switch to master to know how it works (xoofee)
# product image ${MK_GENERATED_IMGS_PATH}/images.zip -i ${MK_GENERATED_IMGS_PATH}/data -l -p
# product image ${MK_GENERATED_IMGS_PATH}/images.zip -e ${MK_GENERATED_IMGS_PATH} -x
# unzip -o generated/images.zip -d generated/images

# $BOARD_DIR\\pack\\pack.exe -d 0 -r ${RTOS_IMG} -f ${FS_DATA_DIR} -o ${OUT_DIR} -m ${MK_GENERATED_IMGS_PATH} -c ${CONFIG_YAML_FILE}

echo 'aft build done'
# read
