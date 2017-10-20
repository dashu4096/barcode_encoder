#ifndef __I25_H__
#define __I25_H__

#include <stddef.h>
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

s32 i25_max_len(const s8 *input);
s32 i25_encode(const s8 *input, s8 *output);

#ifdef __cplusplus
}
#endif

#endif
