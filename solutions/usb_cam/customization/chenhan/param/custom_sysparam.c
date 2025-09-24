/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_sysparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"
#include "board_config.h"

PARAM_CLASSDEFINE(PARAM_VB_CFG_S,VBPOOL,CTX,VB)[] = {
    {
       .u16width = 1920,
       .u16height = 1080,
       .u8VbBlkCnt = 9,
       .fmt = PIXEL_FORMAT_NV21,
       .enBitWidth = DATA_BITWIDTH_8,
       .enCmpMode = COMPRESS_MODE_NONE,
   },
    {
        .u16width = 2400,
        .u16height = 1300,
        .u8VbBlkCnt = 5,
        .fmt = PIXEL_FORMAT_NV21,
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
    },
    //  {
    //     .u16width = 1280,
    //     .u16height = 720,
    //     .u8VbBlkCnt = 10,
    //     .fmt = PIXEL_FORMAT_NV21,
    //     .enBitWidth = DATA_BITWIDTH_10,
    //     .enCmpMode = COMPRESS_MODE_NONE,
    // },
    {
        .u16width = 608,   // Face detection model input width
        .u16height = 342,  // Face detection model input height
        .u8VbBlkCnt = 3,   // Small buffer pool for face detection
        .fmt = PIXEL_FORMAT_RGB_888_PLANAR,  // RGB for AI model
        .enBitWidth = DATA_BITWIDTH_8,
        .enCmpMode = COMPRESS_MODE_NONE,
    },
};

PARAM_SYS_CFG_S  g_stSysCtx = {
    .u8VbPoolCnt = 3,  // Updated to include face detection VB pool
    .u8ViCnt = 2,
    .stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_OFFLINE,
    .stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_OFFLINE,
    .stVPSSMode.ViPipe[0] = 0,
    .stVPSSMode.aenInput[0] = VPSS_INPUT_MEM,
    .stVPSSMode.enMode = VPSS_MODE_DUAL,
    .stVPSSMode.ViPipe[1] = 0,
    .stVPSSMode.aenInput[1] = VPSS_INPUT_MEM,
    .pstVbPool = PARAM_CLASS(VBPOOL,CTX,VB),
};

PARAM_SYS_CFG_S * PARAM_GET_SYS_CFG(void) {
    return &g_stSysCtx;
}
