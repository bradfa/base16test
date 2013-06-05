#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#define MAX_READ_SIZE (32 * 1024 * 1024) /* 32 MiB */

const char *bin2hexlut[] = {
"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F", 
"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F", 
"20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F", 
"30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F", 
"40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F", 
"50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F", 
"60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F", 
"70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F", 
"80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F", 
"90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F", 
"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF", 
"B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF", 
"C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF", 
"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF", 
"E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF", 
"F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"
};

const uint8_t hex2binlut[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t htob(char *in)
{
	uint8_t out = 0;
	char input[3] = {'F', 'F', '\0'};
	if (!isxdigit(in[0]) || !isxdigit(in[1])) {
		errno = -EINVAL;
		return out;
	}
	memcpy(input, in, 2);
	out = strtol(input, NULL, 16);
	errno = 0;
	return out;
}

uint8_t poorstrtol(char *in)
{
	uint8_t out = 0;
	if (!isxdigit(in[0]) || !isxdigit(in[1]))
		return 0;
	if (in[0] <= 0x39)
		out += (in[0] - 0x30) << 4;
	else if (in[0] <= 0x46)
		out += (in[0] - 0x41 + 0xA) << 4;
	else
		return 0;
	if (in[1] <= 0x39)
		out += (in[1] - 0x30);
	else if (in[1] <= 0x46)
		out += (in[1] - 0x41 + 0xA);
	else
		return 0;
	return out;
}

uint8_t htobinlut(char *in)
{
	uint8_t out = 0;
	out += hex2binlut[(uint8_t)in[0]] << 4;
	out += hex2binlut[(uint8_t)in[1]];
	return out;
}

int main(int argc, char *argv[])
{
	ssize_t readbytes;
	int bfd, bini, hexi;
	clock_t start, end;
	uint8_t *binary;
	char *hex;

	if (argc != 2) {
		printf("Usage: %s input.file\n", argv[0]);
		return -1;
	}
	bfd = open(argv[1], O_RDONLY);
	if (bfd == -1) {
		printf("Couldn't open %s\n", argv[1]);
		return -1;
	}
	binary = malloc(MAX_READ_SIZE);
	if (!binary) {
		printf("Out of memory!\n");
		return -ENOMEM;
	}
	hex = malloc((2 * MAX_READ_SIZE) + 1);
	if (!hex) {
		printf("Out of memory!");
		return -ENOMEM;
	}
	readbytes = read(bfd, binary, MAX_READ_SIZE);
	printf("Read %d bytes\n", (int)readbytes);
	close(bfd);

	/* sprintf() binary to hex test */
	start = clock();
	for (hexi = bini = 0; bini < readbytes; hexi+=2, bini++)
		sprintf(&hex[hexi], "%.2X", binary[bini]);
	end = clock();
	printf("sprintf bin to hex took %d clocks\n", (int)(end-start));

	/* bintohlut() binary to hex test */
	start = clock();
	for (hexi = bini = 0; bini < readbytes; hexi+=2, bini++)
		memcpy(&hex[hexi], bin2hexlut[binary[bini]], 2);
	hex[2*readbytes] = '\0';
	end = clock();
	printf("bintohlut bin to hex took %d clocks\n", (int)(end-start));

	/* htob() hex to binary test */
	start = clock();
	for (hexi = bini = 0; hexi < 2*readbytes; hexi+=2, bini++)
		binary[bini] = htob(&hex[hexi]);
	end = clock();
	printf("htob hex to bin took %d clocks\n", (int)(end-start));

	/* poorstrtol() hex to binary test */
	start = clock();
	for (hexi = bini = 0; hexi < 2*readbytes; hexi+=2, bini++)
		binary[bini] = poorstrtol(&hex[hexi]);
	end = clock();
	printf("poorstrtol hex to bin took %d clocks\n", (int)(end-start));

	/* htobinlut() hex to binary test */
	start = clock();
	for (hexi = bini = 0; hexi < 2*readbytes; hexi+=2, bini++)
		binary[bini] = htobinlut(&hex[hexi]);
	end = clock();
	printf("htobinlut hex to bin took %d clocks\n", (int)(end-start));

	return 0;
}
