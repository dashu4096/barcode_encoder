/**
 * @file ean13.c
 * @brief ean13 encoder
 * @author Hansen.Z(hansen@pay-device.com)
 * @version 1.0
 * @date 2016-09-03
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ean13.h"

//https://en.wikipedia.org/wiki/International_Article_Number

#if 0
#define DEBUG
#endif

//add left/right blank for barcode
#define EAN13_APPEND_BLANK

//add left blank to display human readable string(first digit)
#ifdef EAN13_APPEND_BLANK
#define EAN13_BLANK_LEN				8
#endif


#define EAN13_INPUT_LEN				12
#define EAN13_PATTERN_LEN			7
//start/stop
#define EAN13_MARKER_INDEX		    0
#define EAN13_MARKER_PATTERN_LEN    3
//center
#define EAN13_CENTER_MARKER_INDEX   1
#define EAN13_CENTER_PATTERN_LEN    5

static const s32 ean13_marker_pattern[] = {
	0x05, //start/stop
	0x0A  //center
};

//left-hand odd parity encoding
static const s32 ean13_left_odd_pattern[] = {
	0x0D, //0 
	0x19, //1
	0x13, //2
	0x3D, //3
	0x23, //4
	0x31, //5
	0x2F, //6
	0x3B, //7
	0x37, //8
	0x0B  //9
};

//left-hand even parity encoding
static const s32 ean13_left_even_pattern[] = {
	0x27, //0 
	0x33, //1
	0x1B, //2
	0x21, //3
	0x1D, //4
	0x39, //5
	0x05, //6
	0x11, //7
	0x09, //8
	0x17  //9
};

//right-hand parity encoding
static const s32 ean13_right_pattern[] = {
	0x72, //0
	0x66, //1
	0x6C, //2
	0x42, //3
	0x5C, //4
	0x4E, //5
	0x50, //6
	0x44, //7
	0x48, //8
	0x74  //9
};

//1:odd 0:even
static const s32 ean13_left_parity_table[] = {
	0x3F, //OOOOOO
	0x34, //OOEOEE
	0x32, //OOEEOE
	0x31, //OOEEEO
	0x2C, //OEOOEE
	0x26, //OEEOOE
	0x23, //OEEEOO
	0x2A, //OEOEOE
	0x29, //OEOEEO
	0x25  //OEEOEO
};

#ifdef EAN13_APPEND_BLANK
static s32 ean13_append_blank(s8 *out) {
	memset(out, 0, EAN13_BLANK_LEN);
	return EAN13_BLANK_LEN;
}
#endif

static s32 ean13_append_marker(s32 index, s8 *out) {
	s32 i;
	s32 marker = ean13_marker_pattern[index];
	s32 marker_len = index ? EAN13_CENTER_PATTERN_LEN : EAN13_MARKER_PATTERN_LEN;
	for (i = marker_len; i > 0; i--) {
#ifdef DEBUG
		printf("%d",(marker & (1 << (i-1))) ? 1 : 0);
#endif
		*out++ = (marker & (1 << (i-1))) ? 1 : 0;
	}
#ifdef DEBUG
	printf("\n");
#endif
	return marker_len;
}

static s32 ean13_append_data(const s8 *input, s8 *out, s32 *checksum) {
	s32 i,j;
	s32 sum = 0;
	s32 weight = 0;
	s32 index = 0;
	s32 append_len = 0;
	s32 total_len = 0;
	s32 pattern = 0;
	s32 first_index = *input - '0';
	//append left-hand
	for(i=0; i<((EAN13_INPUT_LEN>>1) + 1); i++) {
		index = *(input+i) - '0';
		if (index < 0 || index > 9) {
			printf("%s %d input err\n",__func__,__LINE__);
			goto err;
		}
#ifdef DEBUG
		printf("%c index:%d->", *(input+i), index);
#endif
		weight = ((i%2) == 0) ? 1 : 3;
		sum += index * weight;
		//skip first digit
		if (i > 0) {
			pattern = (ean13_left_parity_table[first_index] & (1 << (6-i))) ? ean13_left_odd_pattern[index] : ean13_left_even_pattern[index];
			for (j = EAN13_PATTERN_LEN; j > 0; j--) {
#ifdef DEBUG
				printf("%d",(pattern & (1 << (j-1))) ? 1 : 0);
#endif
				*out++ = (pattern & (1 << (j-1))) ? 1 : 0;
			}
			total_len += EAN13_PATTERN_LEN;
		}
#ifdef DEBUG
		printf("\n");
#endif
	}

	//append center marker
#ifdef DEBUG
	printf("center->");
#endif
	append_len = ean13_append_marker(EAN13_CENTER_MARKER_INDEX, out);
	total_len += append_len;
	out += append_len;

	//append right-hand
	for(; i<EAN13_INPUT_LEN; i++) {
		index = *(input+i) - '0';
		if (index < 0 || index > 9) {
			printf("%s %d input err\n",__func__,__LINE__);
			goto err;
		}
#ifdef DEBUG
		printf("%c index:%d->", *(input+i), index);
#endif
		pattern = ean13_right_pattern[index];
		weight = ((i%2) == 0) ? 1 : 3;
		sum += index * weight;
		for (j = EAN13_PATTERN_LEN; j > 0; j--) {
#ifdef DEBUG
			printf("%d",(pattern & (1 << (j-1))) ? 1 : 0);
#endif
			*out++ = (pattern & (1 << (j-1))) ? 1 : 0;
		}
#ifdef DEBUG
		printf("\n");
#endif
		total_len += EAN13_PATTERN_LEN;
	}
	//append checksum(modulo 10)
	sum = (sum % 10 != 0) ? (10 - (sum % 10)) : 0;
	*checksum = sum;
#ifdef DEBUG
	printf("check:%d->",sum);
#endif
	pattern = ean13_right_pattern[sum];
	for (j = EAN13_PATTERN_LEN; j > 0; j--) {
#ifdef DEBUG
		printf("%d",(pattern & (1 << (j-1))) ? 1 : 0);
#endif
		*out++ = (pattern & (1 << (j-1))) ? 1 : 0;
	}
#ifdef DEBUG
	printf("\n");
#endif
	total_len += EAN13_PATTERN_LEN;

	return total_len;
err:
	return -1;
}

//len = start + (data-1) + checksum + stop
s32 ean13_max_len(const s8 *input) {
	s32 len = 0;
	if (input != NULL) {
		len = EAN13_INPUT_LEN * EAN13_PATTERN_LEN + (EAN13_MARKER_PATTERN_LEN<<1) + EAN13_CENTER_PATTERN_LEN;
#ifdef EAN13_APPEND_BLANK
		len += (EAN13_BLANK_LEN << 1);
#endif
	}
	return len;
}


s32 ean13_encode(const s8 *input, s8 *output, s32 *checksum) {
	s32 barcode_len = 0;
	s32 input_len = 0;
	s32 append_len = 0;

	if (input == NULL || output == NULL) {
		printf("%s %d err\n",__func__,__LINE__);
		goto end;
	}
	input_len = strlen(input);
	if (input_len != EAN13_INPUT_LEN) {
		printf("%s %d input err\n",__func__,__LINE__);
		goto end;
	}

#ifdef EAN13_APPEND_BLANK
	//append left blank
	append_len = ean13_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

#ifdef DEBUG
	printf("start->");
#endif
	//append start code
	append_len = ean13_append_marker(EAN13_MARKER_INDEX, output);
	output += append_len;
	barcode_len += append_len;

	//append data code and checksum
	append_len = ean13_append_data(input, output, checksum);
	if (append_len < 0) {
		printf("%s %d end\n",__func__,__LINE__);
		goto end;
	}
	output += append_len;
	barcode_len += append_len;

#ifdef DEBUG
	printf("stop->");
#endif
	//append stop code
	append_len = ean13_append_marker(EAN13_MARKER_INDEX, output);
	output += append_len;
	barcode_len += append_len;

#ifdef EAN13_APPEND_BLANK
	//append right blank
	append_len = ean13_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

end:
	return barcode_len;
}
