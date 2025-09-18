/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_viparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"

extern unsigned char cvi_alios_cv1810c_gc2093_0423_pq_param[];
extern unsigned int cvi_alios_cv1810c_gc2093_0423_pq_param_length;
extern unsigned char cvi_ir_pq_param[];
extern unsigned int cvi_ir_pq_param_length;

PARAM_CLASSDEFINE(PARAM_SNS_CFG_S,SENSORCFG,CTX,Sensor)[] = {
    {
        .enSnsType = CONFIG_SNS0_TYPE,
        .s32I2cAddr = 0x37,
        .s8I2cDev = 1,
        .u32Rst_port_idx = 4,//PWR_GPIO2
        .u32Rst_pin = 2,
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
        .s32WDRMode = 1,

        .bSetDevAttrMipi = 1,
        .as16LaneId[0] = 2,
        .as16LaneId[1] = 0,
        .as16LaneId[2] = 1,
        .as16LaneId[3] = -1,
        .as16LaneId[4] = -1,
        .as8PNSwap[0] = 0,
        .as8PNSwap[1] = 0,
        .as8PNSwap[2] = 0,
        .as8PNSwap[3] = 0,
        .as8PNSwap[4] = 0,
        .s32Framerate = -1,
        .bSetDevAttr = 1,
        .u8MclkCam = 0,
        .u8MclkFreq = CAMPLL_FREQ_27M,
        .s16MacClk = RX_MAC_CLK_200M,
    },
    {
        .enSnsType = CONFIG_SNS1_TYPE,
        .s32I2cAddr = 0x37,
        .s8I2cDev = 2,
        .u32Rst_port_idx = 4,//PWR_GPIO2
        .u32Rst_pin = 1,
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,

        .bSetDevAttrMipi = 1,
        .s32Framerate = -1,
        .as16LaneId[0] = 4,
        .as16LaneId[1] = 3,
        .as16LaneId[2] = -1,
        .as16LaneId[3] = -1,
        .as16LaneId[4] = -1,
        .as8PNSwap[0] = 0,
        .as8PNSwap[1] = 0,
        .as8PNSwap[2] = 0,
        .as8PNSwap[3] = 0,
        .as8PNSwap[4] = 0,
        .bSetDevAttr = 1,
        .u8MclkCam = 1,
        .u8MclkFreq = CAMPLL_FREQ_27M,
        .s16MacClk = RX_MAC_CLK_200M,
    },
};

PARAM_CLASSDEFINE(PARAM_ISP_CFG_S,ISPCFG,CTX,ISP)[] = {
    {
        .bMonoSet = {0, 0},
        .bUseSingleBin = 0,
        .astPQBinDes[0] =
        {
            .pIspBinData = cvi_alios_cv1810c_gc2093_0423_pq_param,
            .binID = CVI_BIN_ID_ISP0,
        },
        .astPQBinDes[1] =
        {
            .pIspBinData = cvi_ir_pq_param,
            .binID = CVI_BIN_ID_ISP1,
        },
    },
};

PARAM_CLASSDEFINE(PARAM_DEV_CFG_S,VIDEVCFG,CTX,VI)[] = {
    {
        .pViDmaBuf = NULL,
        .u32ViDmaBufSize = 0,
    #if CONFIG_SENSOR_DUAL_SWITCH
        .isMux = true,
        .u8AttachDev = 0,
        .switchGpioIdx = -1,
        .switchGpioPin = -1,
        .switchGPioPol = -1,
    #endif
    },
    {
    #if CONFIG_SENSOR_DUAL_SWITCH
        .isMux = true,
        .u8AttachDev = 1,
        .switchGpioIdx = -1,
        .switchGpioPin = -1,
        .switchGPioPol = -1,
    #endif
    }
};

PARAM_VI_CFG_S g_stViCtx = {
    .u32WorkSnsCnt = 2,
    .pstSensorCfg = PARAM_CLASS(SENSORCFG,CTX,Sensor),
    .pstIspCfg = PARAM_CLASS(ISPCFG,CTX,ISP),
    .pstDevInfo = PARAM_CLASS(VIDEVCFG,CTX,VI)
};

PARAM_VI_CFG_S * PARAM_GET_VI_CFG(void) {
    g_stViCtx.pstIspCfg[0].astPQBinDes[0].u32IspBinDataLen = cvi_alios_cv1810c_gc2093_0423_pq_param_length;
    g_stViCtx.pstIspCfg[0].astPQBinDes[1].u32IspBinDataLen = cvi_ir_pq_param_length;
    return &g_stViCtx;
}
