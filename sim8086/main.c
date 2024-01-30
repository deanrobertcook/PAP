#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static const char *opcode_map[] = {
		[0b000000] = "ADD",
		[0b100010] = "MOV",
};

// index is REG << 3 + W
static const char *reg_w_map[] = {
		[0b0000] = "AL",
		[0b0001] = "AX",
		[0b0010] = "CL",
		[0b0011] = "CX",
		[0b0100] = "DL",
		[0b0101] = "DX",
		[0b0110] = "BL",
		[0b0111] = "BX",
		[0b1000] = "AH",
		[0b1001] = "SP",
		[0b1010] = "CH",
		[0b1011] = "BP",
		[0b1100] = "DH",
		[0b1101] = "SI",
		[0b1110] = "BH",
		[0b1111] = "DI",
};

void print_binary(const unsigned char *array, size_t size)
{
	for (int i = 0; i < size; i++)
	{
		for (int j = 7; j >= 0; --j)
		{
			printf("%c", (array[i] & (1 << j)) ? '1' : '0');
		}
		printf(" ");
	}
	printf("\n");
}

int main()
{

	FILE *file;
	unsigned char *buffer;
	long fileLen;

	// file = fopen("listing_0037_single_register_mov", "rb");
	file = fopen("listing_0038_many_register_mov", "rb");
	if (!file)
	{
		perror("Unable to open file");
		return 1;
	}

	// Seek to the end to determine the file size
	fseek(file, 0, SEEK_END);
	fileLen = ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer = (unsigned char *)malloc(fileLen + 1);
	if (!buffer)
	{
		perror("Memory error!\n");
		fclose(file);
		return 1;
	}

	fread(buffer, fileLen, 1, file);
	fclose(file);

	printf("file len: %ld\n", fileLen);
	// print_binary(buffer, fileLen);

	for (int i = 0; i < fileLen; i+=2)
	{

		print_binary(&buffer[i], 1);
		print_binary(&buffer[i + 1], 1);

		int opcode = buffer[i] >> 2;
		printf("opcode is: %s\n", opcode_map[opcode]);

		int D = (buffer[i] >> 1) & 1;
		int W = buffer[i] & 1;

		printf("D is: %c, W is %c\n", D ? '1' : '0', W ? '1' : '0');

		int MOD = (buffer[i + 1] >> 6) & 0b11;

		int REG = (buffer[i + 1] >> 3) & 0b111;
		printf("REG + W %d\n", (REG << 1) + W);
		const char *reg = reg_w_map[(REG << 1) + W];
		printf("MOD is %d, REG is %d, register is %s\n", MOD, REG, reg);
	}

	free(buffer);

	return 0;
}
