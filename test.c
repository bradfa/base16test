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

#define MAX_READ_SIZE (100 * 1024 * 1024) /* 100 MiB */

int errexit(char *s)
{
	printf("%s\n", s);
	exit(-1);
}

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
	if (!binary)
		errexit("Out of memory");
	hex = malloc((2 * MAX_READ_SIZE) + 1);
	if (!hex)
		errexit("Out of memory");
	readbytes = read(bfd, binary, MAX_READ_SIZE);
	printf("Read %d bytes\n", (int)readbytes);

	start = clock();
	for (hexi = bini = 0; bini < readbytes; hexi+=2, bini++)
		sprintf(&hex[hexi], "%.2X", binary[bini]);
	end = clock();
	printf("sprintf bin to hex took %d clocks\n", (int)(end-start));

	/* Convert hex string back to binary */
	start = clock();
	for (hexi = bini = 0; hexi < (2*readbytes)+1; hexi+=2, bini++)
		binary[bini] = htob(&hex[hexi]);
	end = clock();
	printf("htob hex to bin took %d clocks\n", (int)(end-start));

	close(bfd);
	return 0;
}
