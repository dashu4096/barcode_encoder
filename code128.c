/**
 * @file code128.c
 * @brief code128 encoder
 * @author Hansen.Z(hansen@pay-device.com)
 * @version 1.0
 * @date 2016-09-03
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code128.h"

//https://en.wikipedia.org/wiki/Code_128

#if 0
#define DEBUG
#endif

//bits len
#define CODE128_QUIET_ZONE_LEN	10
#define CODE128_CODE_LEN		11
#define CODE128_STOP_CODE_LEN	13

#define CODE128_START_A_INDEX	103
#define CODE128_START_B_INDEX	104
#define CODE128_START_C_INDEX	105
#define CODE128_STOP_INDEX		106

#define CODE128_MODE_A			0x1
#define CODE128_MODE_B			0x2
#define CODE128_MODE_C			0x3

#define CODE128_FNC1_CODE		"[FNC1]"



//FNC1-4 are not in ascii, defined here
static const s8 CODE128_FNC1 = 0x81;
static const s8 CODE128_FNC2 = 0x82;
static const s8 CODE128_FNC3 = 0x83;
static const s8 CODE128_FNC4 = 0x84;

static const s32 code128_pattern[] = {
	0x6CC, 
	0x66C,
	0x666,
	0x498,
	0x48C,
	0x44C,
	0x4C8,
	0x4C4,
	0x464,
	0x648,
	0x644,
	0x624,
	0x59C,
	0x4DC,
	0x4CE,
	0x5CC,
	0x4EC,
	0x4E6,
	0x672,
	0x65C,
	0x64E,
	0x6E4,
	0x674,
	0x76E,
	0x74C,
	0x72C,
	0x726,
	0x764,
	0x734,
	0x732,
	0x6D8,
	0x6C6,
	0x636,
	0x518,
	0x458,
	0x446,
	0x588,
	0x468,
	0x462,
	0x688,
	0x628,
	0x622,
	0x5B8,
	0x58E,
	0x46E,
	0x5D8,
	0x5C6,
	0x476,
	0x776,
	0x68E,
	0x62E,
	0x6E8,
	0x6E2,
	0x6EE,
	0x758,
	0x746,
	0x716,
	0x768,
	0x762,
	0x71A,
	0x77A,
	0x642,
	0x78A,
	0x530,
	0x50C,
	0x4B0,
	0x486,
	0x42C,
	0x426,
	0x590,
	0x584,
	0x4D0,
	0x4C2,
	0x434,
	0x432,
	0x612,
	0x650,
	0x7BA,
	0x614,
	0x47A,
	0x53C,
	0x4BC,
	0x49E,
	0x5E4,
	0x4F4,
	0x4F2,
	0x7A4,
	0x794,
	0x792,
	0x6DE,
	0x6F6,
	0x7B6,
	0x578,
	0x51E,
	0x45E,
	0x5E8,
	0x5E2,
	0x7A8,
	0x7A2,
	0x5DE,
	0x5EE,
	0x75E,
	0x7AE,
	0x684,//103 startA
	0x690,//104 startB
	0x69C,//105 startC
	0x18EB//106 stop
};

//check if buf have appoint count digits
static inline s32 code128_check_digit(const s8 *buf, s32 count) {
	s32 num = 0;
	s32 i=0;
	if (*(buf) == CODE128_FNC1) {
		i++;
		count++;
	}
	for(; i< count; i++) {
		//buf[i]>='0' && buf[i] <= '9'
		if (buf[i] > 47 && buf[i] < 58) {
			num++;
		} else {
			//have not continues digits
			break;
		}
	}
	return num;
}

//128A (Code Set A) – ASCII characters 00 to 95 (0–9, A–Z and control codes), special characters, and FNC 1–4
static s32 code128_mapping_a(const s8 *buf) {
	//displayable characters 32-95
	if (buf[0] > 31 && buf[0] < 96)
		return buf[0] - 32;
	//control characters 0-31
	else if (buf[0] < 32)
		return buf[0] + 64;
	else if (buf[0] == CODE128_FNC1)
		return 102;
	else if (buf[0] == CODE128_FNC2)
		return 97;
	else if (buf[0] == CODE128_FNC3)
		return 96;
	else if (buf[0] == CODE128_FNC4)
		return 101;
	else
		return -1;
}

//128B (Code Set B) – ASCII characters 32 to 127 (0–9, A–Z, a–z), special characters, and FNC 1–4
static s32 code128_mapping_b(const s8 *buf) {
	//displayable characters 32-127
	if (buf[0] > 31) // value <= 127 is implied
		return buf[0] - 32;
	else if (buf[0] == CODE128_FNC1)
		return 102;
	else if (buf[0] == CODE128_FNC2)
		return 97;
	else if (buf[0] == CODE128_FNC3)
		return 96;
	else if (buf[0] == CODE128_FNC4)
		return 100;
	else
		return -1;
}

//128C (Code Set C) – 00–99 (encodes two digits with a single code point) and FNC1
static s32 code128_mapping_c(const s8 *buf) {
	if (buf[0] == CODE128_FNC1)
		return 102;

	if (buf[0] > 47 && buf[0] < 58 &&
			buf[1] > 47 && buf[1] < 58) {
		s32 index = 10 * (buf[0] - '0') + (buf[1] - '0');
		return index;
	}

	return -1;
}

/**
 * @brief get the index of switch code
 *
 * @param prev_mode: previous code set
 * @param next_mode : next code set
 *
 * @return index of the switch code pattern
 */
static s32 code128_mapping_switch_code(s32 prev_mode, s32 next_mode) {
	switch (prev_mode) {
		case CODE128_MODE_A:
			switch (next_mode) {
				case CODE128_MODE_B:
					return 100;
				case CODE128_MODE_C:
					return 99;
			}

		case CODE128_MODE_B:
			switch (next_mode) {
				case CODE128_MODE_A:
					return 101;
				case CODE128_MODE_C:
					return 99;
			}

		case CODE128_MODE_C:
			switch (next_mode) {
				case CODE128_MODE_B:
					return 100;
				case CODE128_MODE_A:
					return 101;
			}
	}
	return -1;
}

static s32 code128_append_pattern(s32 index, s32 pattern_length, s8 *out) {
	int i;
	s32 pattern = code128_pattern[index];
	for (i = pattern_length; i > 0; i--)
		*out++ = (pattern & (1 << (i-1))) ? 1 : 0;
	return pattern_length;
}

static s32 code128_append_quiet_zone(s8 *out) {
	memset(out, 0, CODE128_QUIET_ZONE_LEN);
	return CODE128_QUIET_ZONE_LEN;
}

static s32 code128_append_check_code(s32 sum, s8 *out) {
	return code128_append_pattern((sum % 103), CODE128_CODE_LEN, out);
}

static s32 code128_append_start_code(s32 start_index, s8 *out) {
	return code128_append_pattern(start_index, CODE128_CODE_LEN, out);
}

static s32 code128_append_stop_code(s8 *out) {
	return code128_append_pattern(CODE128_STOP_INDEX, CODE128_STOP_CODE_LEN, out);
}

static s32 code128_append_data_code(s32 index, s8 *out) {
	return code128_append_pattern(index, CODE128_CODE_LEN, out);
}

s32 code128_max_len(const s8 *input) {
	s32 len = 0;
	if (input != NULL) {
		len = CODE128_QUIET_ZONE_LEN
			+ CODE128_CODE_LEN //start code
			+ CODE128_CODE_LEN * (strlen(input) * 11 / 10) // contents + 10% padding
			+ CODE128_CODE_LEN //check code
			+ CODE128_STOP_CODE_LEN
			+ CODE128_QUIET_ZONE_LEN;
	}
	return len;
}

/**
* @brief encode input by code128
*
* @param input: input strings
* @param output: coded data,format is binary array
*
* @return length of coded data
*/
s32 code128_encode(const s8 *input, s8 *output) {

	s8 *p = NULL;
	s8 *pos_i = NULL;
	s8 *str = NULL;
	s32 prev_mode = CODE128_MODE_C;
	s32 next_mode  = CODE128_MODE_C;
	s32 append_len = 0;
	s32 barcode_len = 0;
	s32 input_len = 0;
	s32 str_len = 0;
	s32 checksum = 0;
	s32 count = 1;
	s32 index = 0;
	s32 switch_index = 0;
	s32 digits = 0;
	s32 i = 0;

	if (input == NULL || output == NULL) {
		printf("%s %d err\n",__func__,__LINE__);
		goto end;
	}
	input_len = strlen(input);
	str = (s8*)malloc(input_len + 1);
	if (str == NULL) {
		printf("%s %d err\n",__func__,__LINE__);
		goto end;
	}
	//GS1-128 compatible and removes spaces
	if (strncmp(input, CODE128_FNC1_CODE, 6) == 0) {
#ifdef DEBUG
		printf("GS1-128\n");
#endif
		p = str;
		*p++ = CODE128_FNC1;
		input += 6;
		while (*input != '\0') {
			if (*input != ' ') {
				*p++ = *input++;
			} else {
				input++;
			}
		}
		*p = '\0';
	} else {
		memcpy(str, input, input_len);
		*(str+input_len) = '\0';
	}
	str_len = strlen(str);
	pos_i = str;
#ifdef DEBUG
	printf("str(%d):%s\n",str_len,str);
#endif

	//append quiet zone
	append_len = code128_append_quiet_zone(output);
	output += append_len;

	//append start character
	if (input_len == 2 || (input_len > 3 && code128_check_digit(pos_i, 4) > 3)) {
		index = code128_mapping_c(pos_i);
	} else {
		index = -1;
	}
	if (index > -1) {
		//start C
		if (index == 102) {
#ifdef DEBUG
			printf("start C with FNC1\n");
			printf("code C:%c idx:%d\n",*(pos_i),index);
#endif
			//start with [FNC1]
			prev_mode = CODE128_MODE_C;
			append_len = code128_append_start_code(CODE128_START_C_INDEX, output);
			output += append_len;
			checksum += CODE128_START_C_INDEX;
			//append FNC1
			append_len = code128_append_data_code(index, output);
			output += append_len;
			checksum += (index * (count++));
			pos_i += 1;
			index = code128_mapping_c(pos_i);
#ifdef DEBUG
			printf("code C:%c%c idx:%d\n",*(pos_i),*(pos_i+1),index);
#endif
			pos_i += 2;
		} else {
#ifdef DEBUG
			printf("start C\n");
			printf("code C:%c%c idx:%d\n",*(pos_i),*(pos_i+1),index);
#endif
			pos_i += 2;

			prev_mode = CODE128_MODE_C;
			append_len = code128_append_start_code(CODE128_START_C_INDEX, output);
			output += append_len;
			checksum += CODE128_START_C_INDEX;
		}
	} else {
		index = code128_mapping_b(pos_i);
		if (index > -1) {
#ifdef DEBUG
			printf("start B\n");
			printf("code B:%c idx:%d\n",*(pos_i),index);
#endif
			//start B
			pos_i += 1;
			prev_mode = CODE128_MODE_B;
			append_len = code128_append_start_code(CODE128_START_B_INDEX, output);
			output += append_len;
			checksum += CODE128_START_B_INDEX;
		} else {
			index = code128_mapping_a(pos_i);
			if (index > -1) {
#ifdef DEBUG
			printf("start A\n");
			printf("code A:%c idx:%d\n",*(pos_i),index);
#endif
				//start A
				pos_i += 1;
				prev_mode = CODE128_MODE_A;
				append_len = code128_append_start_code(CODE128_START_A_INDEX, output);
				output += append_len;
				checksum += CODE128_START_A_INDEX;
			} else {
				printf("%s %d err\n",__func__,__LINE__);
				barcode_len = 0;
				goto end;
			}
		}
	}

	//append first data character
	append_len = code128_append_data_code(index, output);
	output += append_len;
	checksum += (index * (count++));
	next_mode = prev_mode;

	//continues append data character
	while(*pos_i != '\0') {
		if (prev_mode < CODE128_MODE_C) {
			//middle of data (surrounded by characters from code set A or B). need digits >= 6
			digits = code128_check_digit(pos_i, 6);
			//printf("pos:%c pos-1:%c pos+1(digits):%d pos-1(digits):%d\n",*pos_i,*(pos_i-1),code128_check_digit(pos_i+1, 6),code128_check_digit(pos_i-1,1));
			//such as 'abc000000', split to 'abc'+'000000'
			if (digits > 3 && (code128_check_digit(pos_i+1,(str_len-count-2)%2)) != 0 /*&& code128_check_digit(pos_i-1,1) == 0*/) {
				next_mode = CODE128_MODE_C;
#ifdef DEBUG
				printf("next digits:");
				for(i=0;i<digits;i++)
					printf("%c",*(pos_i+i));
				printf("\n");
#endif
				switch_index = code128_mapping_switch_code(prev_mode, next_mode);
#ifdef DEBUG
				printf("code %c to %c idx:%d pattern:%x\n",(prev_mode==1)?('A'):((prev_mode==2)?('B'):('C')),
						(next_mode==1)?('A'):((next_mode==2)?('B'):('C')),switch_index,code128_pattern[switch_index]);
#endif
				append_len = code128_append_data_code(switch_index, output);
				output += append_len;
				prev_mode = next_mode;
				checksum += (switch_index * (count++));

				for(i=0; i<(digits>>1); i++) {
					index = code128_mapping_c(pos_i);
#ifdef DEBUG
					printf("code-C:%c%c idx:%d\n",*pos_i,*(pos_i+1),index);
#endif
					//code C
					pos_i += 2;
					append_len = code128_append_data_code(index, output);
					output += append_len;
					checksum += (index * (count++));
				}
				continue;
			} else {
				//such as 'abc0000001', split to 'abc0'+'000001'
				index = -1;
			}
		} else {
			index = code128_mapping_c(pos_i);
		}
		if (index > -1) {
#ifdef DEBUG
			printf("code C:%c%c idx:%d\n",*pos_i,*(pos_i+1),index);
#endif
			//code C
			pos_i += 2;
			next_mode = CODE128_MODE_C;
		} else {
			index = code128_mapping_b(pos_i);
			if (index > -1) {
#ifdef DEBUG
				printf("code B:%c idx:%d\n",*pos_i,index);
#endif
				//code B
				pos_i += 1;
				next_mode = CODE128_MODE_B;
			} else {
				index = code128_mapping_a(pos_i);
				if (index > -1) {
#ifdef DEBUG
					printf("code A:%c idx:%d\n",*pos_i,index);
#endif
					//code A
					pos_i += 1;
					next_mode = CODE128_MODE_A;
				} else {
					printf("%s %d err\n",__func__,__LINE__);
					barcode_len = 0;
					goto end;
				}
			}
		}
		//will change code set, append switch code
		if (prev_mode != next_mode) {
			switch_index = code128_mapping_switch_code(prev_mode, next_mode);
#ifdef DEBUG
			printf("code %c to %c idx:%d pattern:%x\n",(prev_mode==1)?('A'):((prev_mode==2)?('B'):('C')),
					(next_mode==1)?('A'):((next_mode==2)?('B'):('C')),switch_index,code128_pattern[switch_index]);
#endif
			append_len = code128_append_data_code(switch_index, output);
			output += append_len;
			prev_mode = next_mode;
			checksum += (switch_index * (count++));
		}

		append_len = code128_append_data_code(index, output);
		output += append_len;
		checksum += (index * (count++));
	}

	//append check character
	append_len = code128_append_check_code(checksum, output);
	output += append_len;
	count++;

	//append stop character
	append_len = code128_append_stop_code(output);
	output += append_len;

	//append quiet zone
	append_len = code128_append_quiet_zone(output);
	output += append_len;

	//count = start code + data code + check code
	barcode_len = count*CODE128_CODE_LEN + CODE128_STOP_CODE_LEN + (CODE128_QUIET_ZONE_LEN << 1);
#ifdef DEBUG
	printf("barcode len:%d\n",barcode_len);
#endif
end:
	if (str != NULL) {
		free(str);
		str = NULL;
	}
	return barcode_len;
}
