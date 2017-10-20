CFLAGS ?= -O2 -Wall -Wextra -g

all: barcode

barcode: main.o code128.o code39.o code93.o code11.o codabar.o msi.o i25.o ean8.o ean13.o upca.o upce.o
	$(CC) $^ -o $@
	rm -f *.o

clean:
	rm -f barcode *.o

format-code:
	astyle *.c *.h

.PHONY: clean format-code all
