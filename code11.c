/**
 * @file code11.c
 * @brief code11 encoder
 * @author Hansen.Z(hansen@pay-device.com)
 * @version 1.0
 * @date 2016-09-03
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code11.h"

//https://en.wikipedia.org/wiki/Code_11

#if 0
#define DEBUG
#endif

//add left/right blank for barcode
//#define CODE11_APPEND_BLANK

#ifdef CODE11_APPEND_BLANK
#define CODE11_BLANK_LEN		10
#endif

#define CODE11_PATTERN_NUM		12
#define CODE11_PATTERN_LEN		7
#define CODE11_MARKER_INDEX		8

//Code 11 is restricted to 12 characters
static const s8 code11_table[] = "12345678*90-";

//for coding simply, changed the array order
static const s32 code11_pattern[] = {
	0x6B,//1
	0x4B,//2
	0x65,//3
	0x5B,//4
	0x6D,//5
	0x4D,//6
	0x53,//7
	0x69,//8
	0x59,//* 7bit
	0x35,//9 6bit
	0x2B,//0 6bit 
	0x2D,//- 6bit
};

static s32 code11_mapping_code(const s8 str) {
	s32 i = CODE11_PATTERN_NUM - 1;
#ifdef DEBUG
	printf("%c ",str);
#endif
	while(i > -1) {
		if (str == code11_table[i]) {
			return i;
		}
		i--;
	}
	return -1;
}

#ifdef CODE11_APPEND_BLANK
static s32 code11_append_blank(s8 *out) {
	memset(out, 0, CODE11_BLANK_LEN);
	return CODE11_BLANK_LEN;
}
#endif

static s32 code11_append_pattern(s32 index, s8 *out) {
	s32 i;
	s32 pattern_len = CODE11_PATTERN_LEN;
	s32 pattern = code11_pattern[index];
	if (index > CODE11_MARKER_INDEX)
		pattern_len--;
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
s32 code11_max_len(const s8 *input) {
	s32 len = 0;
	if (input != NULL) {
		//include gap for each data
		len = CODE11_PATTERN_LEN * (strlen(input) + 2) + strlen(input) + 1;
#ifdef CODE11_APPEND_BLANK
		len += (CODE11_BLANK_LEN << 1);
#endif
	}
	return len;
}

s32 code11_encode(const s8 *input, s8 *output) {

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

#ifdef CODE11_APPEND_BLANK
	//append left blank
	append_len = code11_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

#ifdef DEBUG
	printf("* ");
#endif
	//append start code
	append_len = code11_append_pattern(CODE11_MARKER_INDEX, output);
	output += append_len;
	barcode_len += append_len;
	//append start gap
	*output++ = 0;
	barcode_len++;

	//append data code
	for(i=0; i<input_len; i++) {
		index = code11_mapping_code(*(input+i));
		if (index < 0) {
			printf("%s %d input err\n",__func__,__LINE__);
			barcode_len = 0;
			goto end;
		} else {
			append_len = code11_append_pattern(index, output);
			output += append_len;
			barcode_len += append_len;
			//append gap
			*output++ = 0;
			barcode_len++;
		}
	}

#ifdef DEBUG
	printf("* ");
#endif
	//append stop code
	append_len = code11_append_pattern(CODE11_MARKER_INDEX, output);
	output += append_len;
	barcode_len += append_len;

#ifdef CODE11_APPEND_BLANK
	//append right blank
	append_len = code11_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

end:
	return barcode_len;
}
