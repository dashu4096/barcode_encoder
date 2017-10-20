#ifndef __CODE128_H__
#define __CODE128_H__

#include <stddef.h>
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

s32 code128_max_len(const s8 *input);
s32 code128_encode(const s8 *input, s8 *output);

#ifdef __cplusplus
}
#endif

#endif
