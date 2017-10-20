#ifndef __MSI_H__
#define __MSI_H__

#include <stddef.h>
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

s32 msi_max_len(const s8 *input);
s32 msi_encode(const s8 *input, s8 *output);

#ifdef __cplusplus
}
#endif

#endif
