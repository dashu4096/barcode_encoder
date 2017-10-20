#ifndef __UPCA_H__
#define __UPCA_H__

#include <stddef.h>
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

s32 upca_max_len(const s8 *input);
s32 upca_encode(const s8 *input, s8 *output, s32 *checksum);

#ifdef __cplusplus
}
#endif

#endif
