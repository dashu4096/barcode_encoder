/**
 * @file code93.c
 * @brief code93 encoder
 * @author Hansen.Z(hansen@pay-device.com)
 * @version 1.0
 * @date 2016-09-03
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code93.h"

//https://en.wikipedia.org/wiki/Code_93

#if 0
#define DEBUG
#endif

//add left/right blank for barcode
//#define CODE93_APPEND_BLANK

#ifdef CODE93_APPEND_BLANK
#define CODE93_BLANK_LEN		10
#endif


#define CODE93_PATTERN_NUM		43
#define CODE93_PATTERN_LEN		9

//Code 93 is restricted to 43 characters
static const s8 code93_table[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";

static const s32 code93_pattern[] = {
	0x114, //'0'   100010100
	0x148, //'1'   101001000
	0x144, //'2'   101000100
	0x142, //'3'   101000010
	0x128, //'4'   100101000
	0x124, //'5'   100100100
	0x122, //'6'   100100010
	0x150, //'7'   101010000
	0x112, //'8'   100010010
	0x10A, //'9'   100001010
	0x1A8, //'A'   110101000
	0x1A4, //'B'   110100100
	0x1A2, //'C'   110100010
	0x194, //'D'   110010100
	0x192, //'E'   110010010
	0x18A, //'F'   110001010
	0x168, //'G'   101101000
	0x164, //'H'   101100100
	0x162, //'I'   101100010
	0x134, //'J'   100110100
	0x11A, //'K'   100011010
	0x158, //'L'   101011000
	0x14C, //'M'   101001100
	0x146, //'N'   101000110
	0x12C, //'O'   100101100
	0x116, //'P'   100010110
	0x1B4, //'Q'   110110100
	0x1B2, //'R'   110110010
	0x1AC, //'S'   110101100
	0x1A6, //'T'   110100110
	0x196, //'U'   110010110
	0x19A, //'V'   110011010
	0x16C, //'W'   101101100
	0x166, //'X'   101100110
	0x136, //'Y'   100110110
	0x13A, //'Z'   100111010
	0x12E, //'-'   100101110
	0x1D4, //'.'   111010100
	0x1D2, //' '   111010010
	0x1CA, //'$'   111001010
	0x16E, //'/'   101101110
	0x176, //'+'   101110110
	0x1AE, //'%'   110101110
	0x126, //'($)' 100100110
	0x1DA, //'(%)' 111011010
	0x1D6, //'(/)' 111010110
	0x132, //'(+)' 100110010
	0x15E  //'*'   101011110
};
#define CODE93_MARKER_INDEX	(sizeof(code93_pattern)/sizeof(s32) - 1)

static s32 code93_mapping_code(const s8 str) {
	s32 i = CODE93_PATTERN_NUM - 1;
#ifdef DEBUG
	printf("%c ",str);
#endif
	while(i > -1) {
		if (str == code93_table[i]) {
			return i;
		}
		i--;
	}

	return -1;
}

#ifdef CODE93_APPEND_BLANK
static s32 code93_append_blank(s8 *out) {
	memset(out, 0, CODE93_BLANK_LEN);
	return CODE93_BLANK_LEN;
}
#endif

static s32 code93_append_pattern(s32 index, s8 *out) {
	int i;
	s32 pattern = code93_pattern[index];
#ifdef DEBUG
	printf("index:%d->",index);
#endif
	for (i = CODE93_PATTERN_LEN; i > 0; i--) {
#ifdef DEBUG
		printf("%d",(pattern & (1 << (i-1))) ? 1 : 0);
#endif
		*out++ = (pattern & (1 << (i-1))) ? 1 : 0;
	}
#ifdef DEBUG
	printf("\n");
#endif
	return CODE93_PATTERN_LEN;
}

static s32 code93_checksum_c(const s8 *buf, s32 len) {
	s32 sum = 0;
	s32 i = len - 1;
	s32 weight = 1;

	while(i > -1) {
		if (weight > 20) {
			weight = 1;
		}
		sum += *(buf+i) * weight++;
		i--;
	}

	return (sum % 47);
}

static s32 code93_checksum_k(const s8 *buf, s32 len) {
	s32 sum = 0;
	s32 i = len - 1;
	s32 weight = 1;

	while(i > -1) {
		if (weight > 15) {
			weight = 1;
		}
		sum += *(buf+i) * weight++;
		i--;
	}

	return (sum % 47);
}

//len = start + data + check C + check K + stop + termination
s32 code93_max_len(const s8 *input) {
	s32 len = 0;
	if (input != NULL) {
		len = CODE93_PATTERN_LEN * (strlen(input) + 4) + 1;
#ifdef CODE93_APPEND_BLANK
		len += (CODE93_BLANK_LEN << 1);
#endif
	}
	return len;
}

s32 code93_encode(const s8 *input, s8 *output) {

	s32 barcode_len = 0;
	s32 input_len = 0;
	s32 append_len = 0;
	s32 index = 0;
	s32 i = 0;
	s8 *index_array = NULL;

	if (input == NULL || output == NULL) {
		printf("%s %d err\n",__func__,__LINE__);
		goto end;
	}
	input_len = strlen(input);
	index_array = (s8*)malloc(input_len + 1);//data + check C
	if (index_array == NULL) {
		printf("%s %d err\n",__func__,__LINE__);
		goto end;
	}
	memset(index_array, 0, input_len + 1);

#ifdef CODE93_APPEND_BLANK
	//append left blank
	append_len = code93_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

#ifdef DEBUG
	printf("* ");
#endif
	//append start code
	append_len = code93_append_pattern(CODE93_MARKER_INDEX, output);
	output += append_len;
	barcode_len += append_len;

	//append data code
	for(i=0; i<input_len; i++) {
		index = code93_mapping_code(*(input+i));
		if (index < 0) {
			printf("%s %d input err\n",__func__,__LINE__);
			barcode_len = 0;
			goto end;
		} else {
			*(index_array + i) = index;
			append_len = code93_append_pattern(index, output);
			output += append_len;
			barcode_len += append_len;
		}
	}
	//append check C
	index = code93_checksum_c(index_array, input_len);
#ifdef DEBUG
	printf("check C:\n ");
#endif
	*(index_array + input_len) = index;
	append_len = code93_append_pattern(index, output);
	output += append_len;
	barcode_len += append_len;

	//append check K
	index = code93_checksum_k(index_array, input_len + 1);//data + check C
#ifdef DEBUG
	printf("check K:\n ");
#endif
	append_len = code93_append_pattern(index, output);
	output += append_len;
	barcode_len += append_len;

#ifdef DEBUG
	printf("* ");
#endif
	//append stop code
	append_len = code93_append_pattern(CODE93_MARKER_INDEX, output);
	output += append_len;
	barcode_len += append_len;

	//append termination bar
	*output++ = 1;
	barcode_len++;

#ifdef CODE93_APPEND_BLANK
	//append right blank
	append_len = code93_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

end:
	if (index_array != NULL) {
		free(index_array);
		index_array = NULL;
	}

	return barcode_len;
}
