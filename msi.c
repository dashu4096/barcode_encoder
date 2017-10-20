/**
 * @file msi.c
 * @brief msi encoder
 * @author Hansen.Z(hansen@pay-device.com)
 * @version 1.0
 * @date 2016-09-03
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msi.h"

//https://en.wikipedia.org/wiki/MSI_Barcode

#if 0
#define DEBUG
#endif

//add left/right blank for barcode
//#define MSI_APPEND_BLANK

#ifdef MSI_APPEND_BLANK
#define MSI_BLANK_LEN		10
#endif

#define MSI_PATTERN_LEN		12
#define MSI_START_INDEX		10
#define MSI_STOP_INDEX		11

//MSI is restricted to 10 characters(0-9)
static const s32 msi_pattern[] = {
	0x924, //0 
	0x926, //1
	0x934, //2
	0x936, //3
	0x9A4, //4
	0x9A6, //5
	0x9B4, //6
	0x9B6, //7
	0xD24, //8
	0xD26, //9
	0x6,   //start
	0x9    //stop
};

#ifdef MSI_APPEND_BLANK
static s32 msi_append_blank(s8 *out) {
	memset(out, 0, MSI_BLANK_LEN);
	return MSI_BLANK_LEN;
}
#endif

static s32 msi_checksum(const s8 *buf, s32 len) {
	s32 i;
	s32 sum = 0;
	for(i=len-1; i>-1; i--) {
		if (((len - i) % 2) != 0) {
			sum = ((*(buf+i)-'0') << 1);
			if (sum > 9) {
				sum -= 9; 
			}
			sum += sum;
		} else {
			sum += (*(buf+i) - '0');
		}
	}
	sum = (sum % 10 != 0) ? (10 - (sum % 10)) : 0;

	return sum;
}

static s32 msi_append_pattern(s32 index, s8 *out) {
	s32 i;
	s32 pattern = msi_pattern[index];
	s32 pattern_len = MSI_PATTERN_LEN;
	if (index == MSI_START_INDEX) {
		pattern_len = 3;
	} else if (index == MSI_STOP_INDEX) {
		pattern_len = 4;
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

//len = start + data + check + stop
s32 msi_max_len(const s8 *input) {
	s32 len = 0;
	if (input != NULL) {
		len = MSI_PATTERN_LEN * (strlen(input) + 1) + 7;
#ifdef MSI_APPEND_BLANK
		len += (MSI_BLANK_LEN << 1);
#endif
	}
	return len;
}

s32 msi_encode(const s8 *input, s8 *output) {

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

#ifdef MSI_APPEND_BLANK
	//append left blank
	append_len = msi_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

#ifdef DEBUG
	printf("start ");
#endif
	//append start code
	append_len = msi_append_pattern(MSI_START_INDEX, output);
	output += append_len;
	barcode_len += append_len;

	//append data code
	for(i=0; i<input_len; i++) {
		index = *(input+i) - '0';
#ifdef DEBUG
		printf("%d ", index);
#endif
		if (index < 0 || index > 9) {
			printf("%s %d input err\n",__func__,__LINE__);
			barcode_len = 0;
			goto end;
		} else {
			append_len = msi_append_pattern(index, output);
			output += append_len;
			barcode_len += append_len;
		}
	}

#ifdef DEBUG
	printf("check ");
#endif
	//append check(modulo 10)
	index = msi_checksum(input, input_len);
	append_len = msi_append_pattern(index, output);
	output += append_len;
	barcode_len += append_len;

#ifdef DEBUG
	printf("stop  ");
#endif
	//append stop code
	append_len = msi_append_pattern(MSI_STOP_INDEX, output);
	output += append_len;
	barcode_len += append_len;

#ifdef MSI_APPEND_BLANK
	//append right blank
	append_len = msi_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

end:
	return barcode_len;
}
