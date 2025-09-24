/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: uvc_adapter.h
 * Description: UVC adapter interface for solution-level integration
 */

#ifndef __UVC_ADAPTER_H__
#define __UVC_ADAPTER_H__

#include "cvi_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize UVC adapter
 * Registers callbacks and initializes face detection
 * 
 * @return CVI_SUCCESS on success, error code on failure
 */
int uvc_adapter_init(void);

/**
 * Deinitialize UVC adapter
 * Unregisters callbacks and cleans up resources
 */
void uvc_adapter_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __UVC_ADAPTER_H__ */
