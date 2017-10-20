#ifndef __EAN13_H__
#define __EAN13_H__

#include <stddef.h>
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

s32 ean13_max_len(const s8 *input);
s32 ean13_encode(const s8 *input, s8 *output, s32 *checksum);

#ifdef __cplusplus
}
#endif

#endif
