/**
 * @file upce.c
 * @brief upce encoder
 * @author Hansen.Z(hansen@pay-device.com)
 * @version 1.0
 * @date 2016-09-03
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "upce.h"

//https://en.wikipedia.org/wiki/International_Article_Number

#if 0
#define DEBUG
#endif

//add left/right blank for barcode
#define UPCE_APPEND_BLANK

//add left blank to display human readable string(first digit)
#ifdef UPCE_APPEND_BLANK
#define UPCE_BLANK_LEN				8
#endif


#define UPCA_INPUT_LEN				11
#define UPCE_INPUT_LEN				6
#define UPCE_PATTERN_LEN			7
//start
#define UPCE_START_INDEX		    0
#define UPCE_START_PATTERN_LEN      3
//center and stop
#define UPCE_STOP_INDEX		    	1
#define UPCE_STOP_PATTERN_LEN      	6

static const s32 upce_marker_pattern[] = {
	0x05, //101 start
	0x15  //010101 center and stop
};

//left-hand odd parity encoding
static const s32 upce_left_odd_pattern[] = {
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
static const s32 upce_left_even_pattern[] = {
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

//1:odd 0:even
static const s32 upce_system_0_parity_table[] = {
	0x07, //EEEOOO
	0x0B, //EEOEOO
	0x0D, //EEOOEO
	0x0E, //EEOOOE
	0x13, //EOEEOO
	0x19, //EOOEEO
	0x1C, //EOOOEE
	0x15, //EOEOEO
	0x16, //EOEOOE
	0x1A  //EOOEOE
};

//1:odd 0:even
static const s32 upce_system_1_parity_table[] = {
	0x38, //OOOEEE 
	0x34, //OOEOEE
	0x32, //OOEEOE
	0x31, //OOEEEO
	0x2C, //OEOOEE
	0x26, //OEEOOE
	0x23, //OEEEOO
	0x2A, //OEOEOE
	0x29, //OEOEEO
	0x25, //OEEOEO
};


#ifdef UPCE_APPEND_BLANK
static s32 upce_append_blank(s8 *out) {
	memset(out, 0, UPCE_BLANK_LEN);
	return UPCE_BLANK_LEN;
}
#endif

static s32 upce_append_marker(s32 index, s8 *out) {
	s32 i;
	s32 marker = upce_marker_pattern[index];
	s32 marker_len = index ? UPCE_STOP_PATTERN_LEN : UPCE_START_PATTERN_LEN;
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

//input must be UPC-E 6 digits
static s32 upce_append_data(s8 start_code, const s8 *input, s8 *out, const s32 checksum) {
	s32 i,j;
	s32 index;
	s32 pattern;

	s32 parity = (start_code == '0') ? upce_system_0_parity_table[checksum] : upce_system_1_parity_table[checksum];
#ifdef DEBUG
	printf("parity:");
	for (j = UPCE_INPUT_LEN; j > 0; j--) {
		printf("%c",(parity & (1 << (j-1))) ? 'O' : 'E');
	}
	printf("\n");
#endif

	for(i=0; i<UPCE_INPUT_LEN; i++) {
		index = *(input + i) - '0';
		pattern = (parity & (1 << (UPCE_INPUT_LEN-i-1))) ? upce_left_odd_pattern[index] : upce_left_even_pattern[index];
#ifdef DEBUG
		printf("%d->", index);
#endif
		for (j = UPCE_PATTERN_LEN; j > 0; j--) {
#ifdef DEBUG
			printf("%d",(pattern & (1 << (j-1))) ? 1 : 0);
#endif
			*out++ = (pattern & (1 << (j-1))) ? 1 : 0;
		}
#ifdef DEBUG
		printf("\n");
#endif
	}
	return UPCE_INPUT_LEN * UPCE_PATTERN_LEN;
}

//convert UPC-A 11 digits to UPC-E 6 digits
static s32 upce_convert_from_upca(const s8 *from, s8 *to) {
	s32 i;
	s8 ends[3] = {0};
	s8 product_code[5] = {0};
	//get last three digits
	for(i=5; i>2; i--) {
		ends[i-3] = *(from + i);
	}
	//get product code
	for(i=6; i>11; i++) {
		product_code[i] = *(from + i);
	}
	//refer to:http://www.barcodeisland.com/upce.phtml#Conversion
	if (ends[2] == '0' && ends[1] == '0' && ends[0] < '3') {
		//check product code
		if (atoi(product_code) > 999) {
			printf("%s %d err:can't converted to UPC-E\n",__func__,__LINE__);
			goto err;
		}
		//take first 2 digits
		*to++ = *(from + 1);
		*to++ = *(from + 2);
		//take latest 3 digits
		*to++ = *(from + 8);
		*to++ = *(from + 9);
		*to++ = *(from + 10);
		//take end
		*to = ends[0];
	} else if (ends[2] == '0' && ends[1] == '0' && ends[0] > '2') {
		//check product code
		if (atoi(product_code) > 99) {
			printf("%s %d err:can't converted to UPC-E\n",__func__,__LINE__);
			goto err;
		}
		//take first 3 digits
		*to++ = *(from + 1);
		*to++ = *(from + 2);
		*to++ = *(from + 3);
		//take latest 2 digits
		*to++ = *(from + 9);
		*to++ = *(from + 10);
		//end is '3'
		*to = '3';
	} else if (ends[2] == '0') {
		//check product code
		if (atoi(product_code) > 9) {
			printf("%s %d err:can't converted to UPC-E\n",__func__,__LINE__);
			goto err;
		}
		//take first 4 digits
		for(i=1; i<5; i++) {
			*to++ = *(from + i);
		}
		//take end digits
		*to++ = *(from + 10);
		//end is '4'
		*to = '4';
	} else {
		//check product code
		if (atoi(product_code) > 9 || atoi(product_code) < 5) {
			printf("%s %d err:can't converted to UPC-E\n",__func__,__LINE__);
			goto err;
		}
		//take manufacturer code
		for(i=1; i<6; i++) {
			*to++ = *(from + i);
		}
		//take end digits
		*to++ = *(from + 10);
	}

	return 0;
err:
	return -1;
}

//convert UPC-E 6 digits to UPC-A 11 digits
static s32 upce_convert_to_upca(const s8 *from, s8 *to) {
	s8 end = *(from + UPCE_INPUT_LEN - 1);
	s32 i;

	//always start with '0'
	*to++ = '0';

	switch(end) {
		case '0':
		case '1':
		case '2':
			//take first 2 digits
			*to++ = *from;
			*to++ = *(from + 1);
			//take the last ditit
			*to++ = end;
			//take four 0 digits
			for(i=0; i<4; i++) {
				*to++ = '0';
			}
			//take 3 through 5
			for(i=2; i<5; i++) {
				*to++ = *(from + i);
			}
			break;

		case '3':
			//take first 3 digits
			for(i=0; i<3; i++) {
				*to++ = *(from + i);
			}
			//take five 0 digits
			for(i=0; i<5; i++) {
				*to++ = '0';
			}
			//take 4 through 5
			*to++ = *(from + 3);
			*to++ = *(from + 4);
			break;

		case '4':
			//take first 4 digits
			for(i=0; i<4; i++) {
				*to++ = *(from + i);
			}
			//take five 0 digits
			for(i=0; i<5; i++) {
				*to++ = '0';
			}
			//take 5th digit
			*to++ = *(from + 4);
			break;

		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			//take first 5 digits
			for(i=0; i<5; i++) {
				*to++ = *(from + i);
			}
			//take four 0 digits
			for(i=0; i<4; i++) {
				*to++ = '0';
			}
			//take last digit
			*to = end;
			break;
	}
	return 0;
}

static s32 upce_checksum(const s8 *input, s32 len) {
	s32 i;
	s32 sum = 0;
	s32 weight = 0;
	s32 index = 0;
	s8 str[UPCA_INPUT_LEN] = {0};

	if (len == UPCE_INPUT_LEN) {
		//convert UPC-E to UPC-A 11 digits then compute checksum
		upce_convert_to_upca(input, str);
	} else {
		memcpy(str, input, len);
	}
	for(i=0; i<UPCA_INPUT_LEN; i++) {
		index = str[i] - '0';
		if (index < 0 || index > 9) {
			printf("%s %d input err\n",__func__,__LINE__);
			goto err;
		}
		weight = ((i%2) == 0) ? 3 : 1;
		sum += index * weight;
	}
	sum = (sum % 10 != 0) ? (10 - (sum % 10)) : 0;
#ifdef DEBUG
	if (len == UPCE_INPUT_LEN) {
		printf("UPCE->UPCA: ");
		for(i=0;i<UPCA_INPUT_LEN;i++) {
			printf("%c",str[i]);
		}
		printf("%d",sum);
		printf("\n");
	}
#endif
	return sum;
err:
	return -1;
}

//len = start + data + stop
s32 upce_max_len(const s8 *input) {
	s32 len = 0;
	if (input != NULL) {
		len = UPCE_INPUT_LEN * UPCE_PATTERN_LEN + UPCE_START_PATTERN_LEN + UPCE_STOP_PATTERN_LEN;
#ifdef UPCE_APPEND_BLANK
		len += (UPCE_BLANK_LEN << 1);
#endif
	}
	return len;
}

//input: 11 digits(must start with 0 or 1 to be converted UPC-E), 6 digits
s32 upce_encode(const s8 *input, s8 *output, s32 *checksum) {
	s32 barcode_len = 0;
	s32 input_len = 0;
	s32 append_len = 0;
	s8 str[UPCE_INPUT_LEN] = {0};
	s8 start_code = '0';

	if (input == NULL || output == NULL) {
		printf("%s %d err\n",__func__,__LINE__);
		goto end;
	}
	input_len = strlen(input);
	if (input_len != UPCE_INPUT_LEN && input_len != UPCA_INPUT_LEN) {
		printf("%s %d input err\n",__func__,__LINE__);
		goto end;
	}
	if (input_len == UPCA_INPUT_LEN) {
		//check start for upc-a
		if (*input != '0' && *input != '1') {
			printf("%s %d input err\n",__func__,__LINE__);
			goto end;
		}
		//convert upc-a to upc-e
		if (upce_convert_from_upca(input, str) < 0) {
			printf("%s %d input err\n",__func__,__LINE__);
			goto end;
		}
		start_code = *input;
#ifdef DEBUG
		printf("UPCA->UPCE:");
#endif
	} else {
#ifdef DEBUG
		printf("UPCE:");
#endif
		memcpy(str, input, UPCE_INPUT_LEN);
	}
#ifdef DEBUG
	int i;
	for(i=0;i<UPCE_INPUT_LEN;i++) {
		printf("%c",str[i]);
	}
	printf("\n");
#endif
#ifdef UPCE_APPEND_BLANK
	//append left blank
	append_len = upce_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

#ifdef DEBUG
	printf("start->");
#endif
	//append start code
	append_len = upce_append_marker(UPCE_START_INDEX, output);
	output += append_len;
	barcode_len += append_len;

	//compute check digit by input string
	*checksum = upce_checksum(input, input_len);

	//append data code and checksum
	append_len = upce_append_data(start_code, str, output, *checksum);
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
	append_len = upce_append_marker(UPCE_STOP_INDEX, output);
	output += append_len;
	barcode_len += append_len;

#ifdef UPCE_APPEND_BLANK
	//append right blank
	append_len = upce_append_blank(output);
	output += append_len;
	barcode_len += append_len;
#endif

end:
	return barcode_len;
}
