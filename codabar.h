#ifndef __CODABAR_H__
#define __CODABAR_H__

#include <stddef.h>
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

s32 codabar_max_len(const s8 *input);
s32 codabar_encode(const s8 *input, s8 *output);

#ifdef __cplusplus
}
#endif

#endif
