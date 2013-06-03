#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>

#define READ_SIZE (10 * 1024 * 1024) /* 10 MiB */

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
	int bfd, hfd, ret, bini, hexi;
	clock_t start, end;
	uint8_t *binary;
	char *hex;

	bfd = open("./binary.test", O_RDONLY);
	if (bfd == -1)
		errexit("Couldn't open binary.test file");
	binary = malloc(READ_SIZE);
	if (!binary)
		errexit("Out of memory");
	hex = malloc((2 * READ_SIZE) + 1);
	if (!hex)
		errexit("Out of memory");
	ret = read(bfd, binary, READ_SIZE);
	printf("Read %d bytes\n", ret);

	start = clock();
	for (hexi = bini = 0; bini < READ_SIZE; hexi+=2, bini++)
		sprintf(&hex[hexi], "%.2X", binary[bini]);
	end = clock();
	printf("sprintf bin to hex took %d clocks\n", end-start);

	/* Convert hex string back to binary */
	start = clock();
	for (hexi = bini = 0; hexi < (2*READ_SIZE)+1; hexi+=2, bini++)
		binary[bini] = htob(&hex[hexi]);
	end = clock();
	printf("htob hex to bin took %d clocks\n", end-start);

	close(bfd);
	close(hfd);
	return 0;
}
