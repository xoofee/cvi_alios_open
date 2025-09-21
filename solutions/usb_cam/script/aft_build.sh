#!/bin/sh
BASE_PWD=`pwd`
MK_GENERATED_PATH=generated

echo "[INFO] Generated output files ..."

rm -fr $MK_GENERATED_PATH
mkdir -p $MK_GENERATED_PATH/data/

OBJCOPY=riscv64-unknown-elf-objcopy

#output yoc.bin
ELF_NAME=`ls Obj/*.elf`
$OBJCOPY -O binary $ELF_NAME yoc.bin

#Prepare bin
BOARDS_CONFIG_PATH=$BOARD_PATH/configs
BOOT0_BIN="${BOARD_PATH}/bootimgs/boot0"
BOOT_BIN="${BOARD_PATH}/bootimgs/boot"

cp ${BOOT0_BIN} $MK_GENERATED_PATH/data
cp ${BOOT_BIN} $MK_GENERATED_PATH/data
cp "$BOARDS_CONFIG_PATH/config.yaml" $MK_GENERATED_PATH/data/

# Copy fip.bin to generated images
echo "[INFO]2 Copy fip.bin to generated images"
cp ../../boards/tools/fip/181x_250918/fip.bin $MK_GENERATED_PATH/data/

# Copy AI model to weight partition
echo "[INFO] Copy AI model to weight partition"
if [ -f "../../models/retinaface_mnet0.25_342_608.cvimodel" ]; then
    cp ../../models/retinaface_mnet0.25_342_608.cvimodel $MK_GENERATED_PATH/data/
    echo "[INFO] Model copied: retinaface_mnet0.25_342_608.cvimodel"
    
    # Create weight partition image
    echo "[INFO] Creating weight partition image..."
    MODEL_SIZE=$(stat -c%s "../../models/retinaface_mnet0.25_342_608.cvimodel")
    WEIGHT_SIZE=0x850000  # 8.3125 MB
    
    if [ $MODEL_SIZE -le $WEIGHT_SIZE ]; then
        # Create weight.bin with model file
        dd if=/dev/zero of="$MK_GENER
         ATED_PATH/data/weight.bin" bs=1 count=$WEIGHT_SIZE 2>/dev/null
        dd if="../../models/retinaface_mnet0.25_342_608.cvimodel" of="$MK_GENERATED_PATH/data/weight.bin" bs=1 conv=notrunc 2>/dev/null
        echo "[INFO] Weight partition image created: weight.bin ($(($WEIGHT_SIZE / 1024 / 1024)) MB)"
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

$BOARD_DIR\\pack\\pack.exe -d 0 -r ${RTOS_IMG} -f ${FS_DATA_DIR} -o ${OUT_DIR} -m ${MK_GENERATED_IMGS_PATH} -c ${CONFIG_YAML_FILE}
