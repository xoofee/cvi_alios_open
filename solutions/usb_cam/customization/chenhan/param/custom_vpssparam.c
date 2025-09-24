/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_vpsscfg.c
 * Description:
 *   ....
 */
#include "custom_param.h"
#include "board_config.h"
#include "cvi_buffer.h"

// GRP0: a pass throught channel for usb uvc camera 0 (COLOR CAMERA)
PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP0,CHN)[] = {
    {
        .u8Rotation = ROTATION_0,
        .stVpssChnAttr = {
            .u32Width = 1920,
            .u32Height = 1080,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .bFlip = CVI_FALSE,
            .bMirror = CVI_FALSE,
            .u32Depth  = 0,
            .stAspectRatio= {
                .enMode = ASPECT_RATIO_AUTO,
                .bEnableBgColor = CVI_TRUE,
                .u32BgColor = COLOR_RGB_BLACK,
            },
            .stNormalize = {
                .bEnable = CVI_FALSE,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = CVI_FALSE,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Height = -1,
                .u32Width = -1,
            },
        },
    },
};

// GRP1: a pass throught channel for usb uvc camera 1 (IR CAMERA)
PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP1,CHN)[] = {
    {
        .u8Rotation = ROTATION_0,
        .stVpssChnAttr = {
            .u32Width = 1920,
            .u32Height = 1080,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .bFlip = CVI_FALSE,
            .bMirror = CVI_FALSE,
            .u32Depth = 0,
            .stAspectRatio= {
                .enMode = ASPECT_RATIO_AUTO,
                .bEnableBgColor = CVI_TRUE,
                .u32BgColor     = COLOR_RGB_BLACK,
            },
            .stNormalize = {
                .bEnable = CVI_FALSE,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = CVI_FALSE,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Height = -1,
                .u32Width = -1,
            },
        },
    },
};

// newly added
// Face detection VPSS channel configuration
PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP2,CHN)[] = {
    {
        .u8Rotation = ROTATION_0,
        .stVpssChnAttr = {
            .u32Width = 608,   // Face detection model input width
            .u32Height = 342,  // Face detection model input height
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_RGB_888_PLANAR,  // RGB for AI model
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
            .bFlip = CVI_FALSE,
            .bMirror = CVI_FALSE,
            .u32Depth = 1,
            .stAspectRatio= {
                .enMode = ASPECT_RATIO_NONE,  // Stretch to exact dimensions
                .bEnableBgColor = CVI_TRUE,
                .u32BgColor = COLOR_RGB_BLACK,
            },
            .stNormalize = {
                .bEnable = CVI_FALSE,
            },
        },
        .stVpssChnCropInfo = {
            .bEnable = CVI_FALSE,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Height = -1,
                .u32Width = -1,
            },
        },
    },
};

// define VI-VPSS bind
PARAM_CLASSDEFINE(PARAM_VPSS_GRP_CFG_S,GRPCFG,CTX,GRP)[] = {
    {   // GRP0 pass through for color camera
        .VpssGrp = 0,
        .u8ChnCnt = 1,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP0,CHN),
        .u8ViRotation = 0,
        .s32BindVidev = 0,  //bind VI device 0
        .stVpssGrpAttr = {
            .u8VpssDev = 0,
            .u32MaxW = -1,
            .u32MaxH = -1,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
        },
        .bBindMode = CVI_TRUE,
        .astChn[0] = {  // input .astChn[0] is just input, not channel 0 in vi or vpss
            .enModId = CVI_ID_VI,
            .s32DevId = 0,  // input: VI device 0
            .s32ChnId = 0,  // input VI channel 0
        },
        .astChn[1] = {  // output .astChn[1] is just output, not channel 0 in vi or vpss
            .enModId = CVI_ID_VPSS,
            .s32DevId = 0,  // output: VPSS device 0
            .s32ChnId = 0,  // output VPSS channel 0
        },
        .stVpssGrpCropInfo = {
            .bEnable = CVI_FALSE,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Height = -1,
                .u32Width = -1,
            },
        },
    },
    {   // GRP1 pass through for ir camera
        .VpssGrp = 1,
        .u8ChnCnt = 1,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP1,CHN),
        .u8ViRotation = 0,
        .s32BindVidev = 1,  //bind VI device 1
        .stVpssGrpAttr = {
            .u8VpssDev = 1,
            .u32MaxW = -1,
            .u32MaxH = -1,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
        },
        .bBindMode = CVI_TRUE,
        .astChn[0] = {
            .enModId = CVI_ID_VI,
            .s32DevId = 0,  // overwrited by s32BindVidev = 1?, or ignored? use ChnId 1?
            .s32ChnId = 1,
        },
        .astChn[1] = {
            .enModId = CVI_ID_VPSS,
            .s32DevId = 1,
            .s32ChnId = 0,
        },
        .stVpssGrpCropInfo = {
            .bEnable = CVI_FALSE,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Height = -1,
                .u32Width = -1,
            },
        },
    },
    {   // newly added for face detection
        .VpssGrp = 2,
        .u8ChnCnt = 1,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP2,CHN),
        .u8ViRotation = 0,
        .s32BindVidev = -1,  // Not bound to VI, will receive frames via SendFrame
        .stVpssGrpAttr = {
            .u8VpssDev = 0,
            .u32MaxW = 1920,  // Max input width
            .u32MaxH = 1080,  // Max input height
            .enPixelFormat = PIXEL_FORMAT_YUYV,   // we only use YUYV_FORMAT_INDEX in usbd_uvc.c for now
            .stFrameRate = {
                .s32SrcFrameRate = -1,
                .s32DstFrameRate = -1,
            },
        },
        .bBindMode = CVI_FALSE,  // Manual mode for face detection
        .astChn[0] = {  // not inpu for manual mode, so this is output
            .enModId = CVI_ID_VPSS,
            .s32DevId = 2,  // GR2, channel 1?
            .s32ChnId = 1,
        },
        .stVpssGrpCropInfo = {
            .bEnable = CVI_FALSE,
            .enCropCoordinate = VPSS_CROP_RATIO_COOR,
            .stCropRect = {
                .s32X = 0,
                .s32Y = 0,
                .u32Height = -1,
                .u32Width = -1,
            },
        },
    },
};


PARAM_VPSS_CFG_S  g_stVpssCtx = {
    .u8GrpCnt = 3,  // Updated to include face detection group
    .pstVpssGrpCfg = PARAM_CLASS(GRPCFG,CTX,GRP),
};

PARAM_VPSS_CFG_S * PARAM_GET_VPSS_CFG(void) {
    return &g_stVpssCtx;
}
