/**
 * @file i25.c
 * @brief i25 encoder
 * @author Hansen.Z(hansen@pay-device.com)
 * @version 1.0
 * @date 2016-09-03
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "i25.h"

//https://en.wikipedia.org/wiki/Interleaved_2_of_5

#if 0
#define DEBUG
#endif

//add left/right blank for barcode
//#define I25_APPEND_BLANK

#ifdef I25_APPEND_BLANK
#define I25_BLANK_LEN		10
#endif


#define I25_START			0xA	//1010
#define I25_STOP			0xD	//1101
#define I25_START_STOP_LEN	4
#define I25_PATTERN_NUM		10
#define I25_PATTERN_LEN		5


static s8 *i25_pattern[] = {
	"NNWWN", //0
	"WNNNW", //1
	"NWNNW", //2
	"WWNNN", //3
	"NNWNW", //4
	"WNWNN", //5
	"NWWNN", //6
	"NNNWW", //7
	"WNNWN", //8
	"NWNWN"  //9
};

#ifdef I25_APPEND_BLANK
static s32 i25_append_blank(s8 *out) {
	memset(out, 0, I25_BLANK_LEN);
	return I25_BLANK_LEN;
}
#endif

static s32 i25_append_marker(s32 code, s8 *out) {
	s32 i;
	for (i = 4; i > 0; i--) {
#ifdef DEBUG
		printf("%d",(code & (1 << (i-1))) ? 1 : 0);
#endif
		*out++ = (code & (1 << (i-1))) ? 1 : 0;
	}
#ifdef DEBUG
	printf("\n");
#endif
	return I25_START_STOP_LEN;
}

static s32 i25_append_data(const s32 input_len, const s8 *input, s8 *output) {
	s32 i,j,k;
	s8 *high;
	s8 *low;
	s8 sum[I25_PATTERN_LEN<<1];
	s32 bit = 0;
	s32 len = 0;

	for(i=0; i<input_len; i+=2) {
#ifdef DEBUG
		printf("%c+%c->",*(input+i),*(input+i+1));
#endif
		//mapping 1st digit and 2st digit
		high = i25_pattern[*(input+i) - '0']; 
		low = i25_pattern[*(input+i+1) - '0']; 

		//interleaved merge 1st and 2st digit
		for(j=0; j<(I25_PATTERN_LEN<<1); j+=2) {
			sum[j] = *high++;
			sum[j+1] = *low++;
		}
		//convet to binary
		for(k=0; k<(I25_PATTERN_LEN<<1); k++) {
			bit = (k%2) ? 0 : 1;
#ifdef DEBUG
			printf("%d",bit);
#endif
			if (sum[k] == 'W') {
				*output++ = bit;
#ifdef DEBUG
			printf("%d",bit);
#endif
			}
			*output++ = bit;
		}
#ifdef DEBUG
		printf("\n");
#endif
		len += 14;
	}
	return len;
}

//len = start + data + stop
s32 i25_max_len(const s8 *input) {
	s32 len = 0;
	if (input != NULL) {
		len = (strlen(input) >> 1) * 14 + 4 + 4;
#ifdef I25_APPEND_BLANK
		len += (I25_BLANK_LEN << 1);
#endif
	}
	return len;
}


s32 i25_encode(const s8 *input, s8 *output) {
	s32 barcode_len = 0;
	s32 input_len = 0;
	s32 append_len = 0;

	if (input == NULL || output == NULL) {
		printf("%s %d err\n",__func__,__LINE__);
		goto end;
	}
	input_len = strlen(input);
	if (input_len % 2 != 0) {
		printf("%s %d input err\n",__func__,__LINE__);
		goto end;
	}

#ifdef I25_APPEND_BLANK
	//append left blank
	append_len = i25_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

#ifdef DEBUG
	printf("start->");
#endif
	//append start
	append_len = i25_append_marker(I25_START, output);
	output += append_len;
	barcode_len += append_len;

	//append data
	append_len = i25_append_data(input_len, input, output);
	output += append_len;
	barcode_len += append_len;

#ifdef DEBUG
	printf("stop->");
#endif
	//append stop
	append_len = i25_append_marker(I25_STOP, output);
	output += append_len;
	barcode_len += append_len;

#ifdef I25_APPEND_BLANK
	//append right blank
	append_len = i25_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

end:
	return barcode_len;

}

