#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "code128.h"
#include "code39.h"
#include "code93.h"
#include "code11.h"
#include "msi.h"
#include "i25.h"
#include "ean13.h"
#include "ean8.h"
#include "upca.h"
#include "upce.h"
#include "codabar.h"

#define CODE128	"code128"
#define CODE39	"code39"
#define CODE93	"code93"
#define CODE11	"code11"
#define MSI		"msi"
#define I25		"i25"
#define EAN8	"ean8"
#define EAN13	"ean13"
#define UPCA	"upca"
#define UPCE	"upce"
#define CODABAR	"codabar"

void print_barcode (char* buffer, int len) {
	int height,i;
	for ( height = 0; height < 6; height++ ) {
		for ( i = 0; i < len; i++ ) {
			if ( buffer[i] ) {
				printf ("%c%c%c", 0xE2, 0x96, 0x88 );
			} else {
				printf (" ");
			}
		}
		printf( "\n" );
	}
	printf("binary array len:%d\n",len);
	for ( i = 0; i < len; i++ ) {
		printf("%d",buffer[i]&0xff?1:0);
	}
	printf( "\n" );
}

static void bin2hex(s8 *bin_buf, u8* hex_buf, s32 hex_buf_len) {
	s8 byte;
	s32 p, pos, bit;
	for (p=0, pos = 0; pos < hex_buf_len; pos++) {
		byte = 0;
		for (bit = 0; bit < 8; bit++) {
			byte |= (*(bin_buf+(p++)) << (7-bit));
		}
		*(hex_buf + pos) = byte & 0xff;
		//print hex arrays
		printf("0x%02x,",byte&0xff);
	}
	printf("\n");
}

int main(int argc, char **argv) {

	s32 bin_len = 0;
	s32 hex_len = 0;
	s32 max_len = 0;
	s8 *bin = NULL;

	struct timeval start;
	struct timeval end;

	if (argc != 3) {
		printf("[31mUsage:%s CODE_MODE string\n[0m",argv[0]);
		printf("eg:%s code93 TEST93\n",argv[0]);
		exit (0);
	}

	gettimeofday(&start, NULL);
	if (strncmp(CODE128, argv[1], strlen(CODE128)) == 0) {
		//get code128 max len
		max_len = code128_max_len(argv[2]);
		bin = (s8*)malloc(max_len);
		memset(bin, 0, max_len);
		bin_len = code128_encode(argv[2], bin);
	} else if (strncmp(CODE39, argv[1], strlen(CODE39)) == 0) {
		//get code39 len
		max_len = code39_max_len(argv[2]);
		bin = (s8*)malloc(max_len);
		memset(bin, 0, max_len);
		bin_len = code39_encode(argv[2], bin);
	} else if (strncmp(CODE93, argv[1], strlen(CODE93)) == 0) {
		//get code93 len
		max_len = code93_max_len(argv[2]);
		bin = (s8*)malloc(max_len);
		memset(bin, 0, max_len);
		bin_len = code93_encode(argv[2], bin);
	} else if (strncmp(CODE11, argv[1], strlen(CODE11)) == 0) {
		//get code11 len
		max_len = code11_max_len(argv[2]);
		bin = (s8*)malloc(max_len);
		memset(bin, 0, max_len);
		bin_len = code11_encode(argv[2], bin);
	} else if (strncmp(CODABAR, argv[1], strlen(CODABAR)) == 0) {
		//get codabar len
		max_len = codabar_max_len(argv[2]);
		bin = (s8*)malloc(max_len);
		memset(bin, 0, max_len);
		bin_len = codabar_encode(argv[2], bin);
	} else if (strncmp(MSI, argv[1], strlen(MSI)) == 0) {
		//get msi len
		max_len = msi_max_len(argv[2]);
		bin = (s8*)malloc(max_len);
		memset(bin, 0, max_len);
		bin_len = msi_encode(argv[2], bin);
	} else if (strncmp(I25, argv[1], strlen(I25)) == 0) {
		//get i25 len
		max_len = i25_max_len(argv[2]);
		bin = (s8*)malloc(max_len);
		memset(bin, 0, max_len);
		bin_len = i25_encode(argv[2], bin);
	} else if (strncmp(EAN8, argv[1], strlen(EAN8)) == 0) {
		//get ean8 len
		int checksum = -1;
		max_len = ean8_max_len(argv[2]);
		bin = (s8*)malloc(max_len);
		memset(bin, 0, max_len);
		bin_len = ean8_encode(argv[2], bin, &checksum);
		printf("checksum:%d\n",checksum);
	} else if (strncmp(EAN13, argv[1], strlen(EAN13)) == 0) {
		//get ean13 len
		int checksum = -1;
		max_len = ean13_max_len(argv[2]);
		bin = (s8*)malloc(max_len);
		memset(bin, 0, max_len);
		bin_len = ean13_encode(argv[2], bin, &checksum);
		printf("checksum:%d\n",checksum);
	} else if (strncmp(UPCA, argv[1], strlen(UPCA)) == 0) {
		//get upca len
		int checksum = -1;
		max_len = upca_max_len(argv[2]);
		bin = (s8*)malloc(max_len);
		memset(bin, 0, max_len);
		bin_len = upca_encode(argv[2], bin, &checksum);
		printf("checksum:%d\n",checksum);
	} else if (strncmp(UPCE, argv[1], strlen(UPCE)) == 0) {
		//get upce len
		int checksum = -1;
		max_len = upce_max_len(argv[2]);
		bin = (s8*)malloc(max_len);
		memset(bin, 0, max_len);
		bin_len = upce_encode(argv[2], bin, &checksum);
		printf("checksum:%d\n",checksum);
	}


	gettimeofday(&end,NULL);
	printf("total used(us):%ld\n", 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec -start.tv_usec);

	if (bin_len > 0)
		print_barcode(bin, bin_len);
	
	
	hex_len = (bin_len>>3);
	if ((bin_len%8) > 0) {
		hex_len++;
	}
	printf("binary max_len:%d\n",max_len);
	printf("hex_len:%d\n",hex_len);
	u8 hex[512];
	memset(hex, 0, 512);
	//convert binary array to hex array for printer
	bin2hex(bin, hex, hex_len);

	if (bin != NULL) {
		free(bin);
		bin = NULL;
	}

	return 0;
}
