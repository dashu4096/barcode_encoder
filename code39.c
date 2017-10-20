/**
 * @file code39.c
 * @brief code39 encoder
 * @author Hansen.Z(hansen@pay-device.com)
 * @version 1.0
 * @date 2016-09-03
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code39.h"

//https://en.wikipedia.org/wiki/Code_39

#if 0
#define DEBUG
#endif

//add left/right blank for barcode
//#define CODE39_APPEND_BLANK

#ifdef CODE39_APPEND_BLANK
#define CODE39_BLANK_LEN		10
#endif

#define CODE39_PATTERN_NUM		43
//one pattern have 12 bits
#define CODE39_PATTERN_LEN		12

//Code 39 is restricted to 43 characters
static const s8 code39_table[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";

//12bits(display 5bars + 4spaces)
static const s32 code39_pattern[] = {
	0xA6D, //'0'
	0xD2B, //'1'
	0xB2B, //'2'
	0xD95, //'3'
	0xA6B, //'4'
	0xD35, //'5'
	0xB35, //'6'
	0xA5B, //'7'
	0xD2D, //'8'
	0xB2D, //'9'
	0xD4B, //'A'
	0xB4B, //'B'
	0xDA5, //'C'
	0xACB, //'D'
	0xD65, //'E'
	0xB65, //'F'
	0xA9B, //'G'
	0xD4D, //'H'
	0xB4D, //'I'
	0xACD, //'J'
	0xD53, //'K'
	0xB53, //'L'
	0xDA9, //'M'
	0xAD3, //'N'
	0xD69, //'O'
	0xB69, //'P'
	0xAB3, //'Q'
	0xD59, //'R'
	0xB59, //'S'
	0xAD9, //'T'
	0xCAB, //'U'
	0x9AB, //'V'
	0xCD5, //'W'
	0x96B, //'X'
	0xCB5, //'Y'
	0x9B5, //'Z'
	0x95B, //'-'
	0xCAD, //'.'
	0x9AD, //' '
	0x925, //'$'
	0x929, //'/'
	0x949, //'+'
	0xA49, //'%'
	0x96D  //'*'
};
#define CODE39_MARKER_INDEX	(sizeof(code39_pattern)/sizeof(s32) - 1)

static s32 code39_mapping_code(const s8 str) {
	s32 i = CODE39_PATTERN_NUM - 1;
#ifdef DEBUG
	printf("%c ",str);
#endif
	while(i > -1) {
		if (str == code39_table[i]) {
			return i;
		}
		i--;
	}
	return -1;
}

#ifdef CODE39_APPEND_BLANK
static s32 code39_append_blank(s8 *out) {
	memset(out, 0, CODE39_BLANK_LEN);
	return CODE39_BLANK_LEN;
}
#endif

static s32 code39_append_pattern(s32 index, s8 *out) {
	s32 i;
	s32 pattern = code39_pattern[index];
#ifdef DEBUG
	printf("index:%d->",index);
#endif
	for (i = CODE39_PATTERN_LEN; i > 0; i--) {
#ifdef DEBUG
		printf("%d",(pattern & (1 << (i-1))) ? 1 : 0);
#endif
		*out++ = (pattern & (1 << (i-1))) ? 1 : 0;
	}
#ifdef DEBUG
	printf("\n");
#endif
	return CODE39_PATTERN_LEN;
}

s32 code39_max_len(const s8 *input) {
	s32 len = 0;
	if (input != NULL) {
		//include gap for each data
		len = CODE39_PATTERN_LEN * (strlen(input) + 2) + strlen(input) + 1;
#ifdef CODE39_APPEND_BLANK
		len += (CODE39_BLANK_LEN << 1);
#endif
	}
	return len;
}

s32 code39_encode(const s8 *input, s8 *output) {

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

#ifdef CODE39_APPEND_BLANK
	//append left blank
	append_len = code39_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif
	//append start code
	append_len = code39_append_pattern(CODE39_MARKER_INDEX, output);
	output += append_len;
	barcode_len += append_len;
	//append start gap
	*output++ = 0;
	barcode_len++;

	//append data code
	for(i=0; i<input_len; i++) {
		index = code39_mapping_code(*(input+i));
		if (index < 0) {
			printf("%s %d input err\n",__func__,__LINE__);
			barcode_len = 0;
			goto end;
		} else {
			code39_append_pattern(index, output);
			output += append_len;
			barcode_len += append_len;
			//append gap
			*output++ = 0;
			barcode_len++;
		}
	}
	//append stop code
	code39_append_pattern(CODE39_MARKER_INDEX, output);
	output += append_len;
	barcode_len += append_len;

#ifdef CODE39_APPEND_BLANK
	//append right blank
	append_len = code39_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

end:
	return barcode_len;
}
