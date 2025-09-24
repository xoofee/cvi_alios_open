/*
 * UVC Adapter Header
 */

#ifndef __UVC_ADAPTER_H__
#define __UVC_ADAPTER_H__

#include "cvi_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize UVC adapter
 * @return CVI_SUCCESS on success, CVI_FAILURE on error
 */
int uvc_adapter_init(void);

/**
 * Deinitialize UVC adapter
 */
void uvc_adapter_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __UVC_ADAPTER_H__ */
