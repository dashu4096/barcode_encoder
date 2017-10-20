/**
 * @file codabar.c
 * @brief codabar encoder
 * @author Hansen.Z(hansen@pay-device.com)
 * @version 1.0
 * @date 2016-09-03
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codabar.h"

//https://en.wikipedia.org/wiki/Codabar

#if 0
#define DEBUG
#endif

//add left/right blank for barcode
//#define CODABAR_APPEND_BLANK

#ifdef CODABAR_APPEND_BLANK
#define CODABAR_BLANK_LEN			10
#endif

#define CODABAR_PATTERN_NUM			20
#define CODABAR_PATTERN_LEN			9
#define CODABAR_PATTERN_LEN_10		10
#define CODABAR_PATTERN_LEN_12		12

//Code 11 is restricted to 20 characters,A-D mapping start/stop A-D
//moved '+' to end for coding simply 
static const s8 codabar_table[] = "0123456789-$:/.ABCD+";

//[0]-[11]:9bit [12]-[18]:10bit [19]:12bit
static const s32 codabar_pattern[] = {
	0x153, // 0
	0x159, // 1
	0x14B, // 2
	0x195, // 3
	0x169, // 4
	0x1A9, // 5
	0x12B, // 6
	0x12D, // 7
	0x135, // 8
	0x1A5, // 9
	0x14D, // -
	0x165, // $
	0x35B, // :
	0x36B, // /
	0x36D, // .
	0x2C9, // Start/Stop A
	0x24B, // Start/Stop B
	0x293, // Start/Stop C
	0x299, // Start/Stop D
	0xB33  // +
};

static s32 codabar_mapping_code(const s8 str) {
	s32 i = CODABAR_PATTERN_NUM - 1;
#ifdef DEBUG
	printf("%c ",str);
#endif
	while(i > -1) {
		if (str == codabar_table[i]) {
			return i;
		}
		i--;
	}
	return -1;
}

#ifdef CODABAR_APPEND_BLANK
static s32 codabar_append_blank(s8 *out) {
	memset(out, 0, CODABAR_BLANK_LEN);
	return CODABAR_BLANK_LEN;
}
#endif

static s32 codabar_append_pattern(s32 index, s8 *out) {
	s32 i;
	s32 pattern_len = 0;
	s32 pattern = codabar_pattern[index];

	if (index == CODABAR_PATTERN_NUM - 1) {
		pattern_len = CODABAR_PATTERN_LEN_12;
	} else if (index > 11 && index < CODABAR_PATTERN_NUM - 1) {
		pattern_len = CODABAR_PATTERN_LEN_10;
	} else {
		pattern_len = CODABAR_PATTERN_LEN;
	}
#ifdef DEBUG
	printf("index:%d->",index);
#endif
	for (i = pattern_len; i > 0; i--) {
#ifdef DEBUG
		printf("%d",(pattern & (1 << (i-1))) ? 1 : 0);
#endif
		*out++ = (pattern & (1 << (i-1))) ? 1 : 0;
	}
#ifdef DEBUG
	printf("\n");
#endif
	return pattern_len;
}

//len = start + data + stop + each gap
s32 codabar_max_len(const s8 *input) {
	s32 len = 0;
	if (input != NULL) {
		len = CODABAR_PATTERN_LEN_12 * (strlen(input) - 2) + (strlen(input) - 1) + (CODABAR_PATTERN_LEN_10 << 1);
#ifdef CODABAR_APPEND_BLANK
		len += (CODABAR_BLANK_LEN << 1);
#endif
	}
	return len;
}

s32 codabar_encode(const s8 *input, s8 *output) {

	s32 barcode_len = 0;
	s32 input_len = 0;
	s32 append_len = 0;
	s32 index = 0;
	s32 i = 0;

	if (input == NULL || output == NULL) {
		printf("%s %d err\n",__func__,__LINE__);
		goto end;
	}

	input_len = strlen(input);

	if (*input < 'A' || *input > 'D' || *(input+input_len-1) < 'A' || *(input+input_len-1) > 'D') {
		printf("%s %d err\n",__func__,__LINE__);
		goto end;
	}

#ifdef CODABAR_APPEND_BLANK
	//append left blank
	append_len = codabar_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

	//append data code
	for(i=0; i<input_len; i++) {
		index = codabar_mapping_code(*(input+i));
		if (index < 0) {
			printf("%s %d input err\n",__func__,__LINE__);
			barcode_len = 0;
			goto end;
		} else {
			append_len = codabar_append_pattern(index, output);
			output += append_len;
			barcode_len += append_len;
			//no gap after stop code
			if (i < input_len - 1) {
				//append gap
				*output++ = 0;
				barcode_len++;
			}
		}
	}

#ifdef CODABAR_APPEND_BLANK
	//append right blank
	append_len = codabar_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

end:
	return barcode_len;
}
