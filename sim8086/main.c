#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// static const char *opcode_map[] = {
// 		[0b100010] = "MOV", // reg/mem to/from reg/mem
// 		[0b1011] = "MOV",		// immediate to register
// };

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

static const char *ea_base[] = {
		[0b000] = "[BX + SI]",
		[0b001] = "[BX + DI]",
		[0b010] = "[BP + SI]",
		[0b011] = "[BP + DI]",
		[0b100] = "[SI]",
		[0b101] = "[DI]",
		[0b110] = "[BP]",
		[0b111] = "[BX]",
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

int main(int argc, const char *argv[])
{

	FILE *file;
	unsigned char *buffer;
	long fileLen;

	if (argc != 2)
	{
		printf("Missing filename of binary to be disassembled\n");
		return 1;
	}
	file = fopen(argv[1], "rb");
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

	printf("bits 16\n");

	int i = 0;

	while (i < fileLen)
	{
		if (buffer[i] >> 4 == 0b1011) // MOV immediate to reg
		{
			int W = (buffer[i] >> 3) & 1;
			int REG = (buffer[i]) & 0b111;
			const char *reg = reg_w_map[(REG << 1) + W];
			int data = buffer[i + 1];
			if (W == 1)
			{
				data = data + (buffer[i + 2] << 8);
			}
			printf("MOV %s, %d\n", reg, data);
			i += W == 1 ? 3 : 2;
		}
		else if (buffer[i] >> 2 == 0b100010) // MOV reg/mem to/from reg/mem
		{
			int W = buffer[i] & 1;

			int MOD = (buffer[i + 1] >> 6) & 0b11;
			int REG = (buffer[i + 1] >> 3) & 0b111;
			const char *reg = reg_w_map[(REG << 1) + W]; //TODO - handle direction
			int RM = buffer[i + 1] & 0b111;

			switch (MOD)
			{
			case 0b00:
			{
				const char *eff_addr = ea_base[RM];
				printf("MOV %s, %s\n", reg, eff_addr);
				i += 2;
				break;
			}

			case 0b01:
				break;

			case 0b10:
				break;

			case 0b11:
			{
				
				const char *rm = reg_w_map[(RM << 1) + W];
				printf("MOV %s, %s\n", rm, reg);
				i += 2;
				break;
			}
			}
		}
		else
		{
			printf("Unknown opcode, aborting");
			return 1;
		}
	}

	free(buffer);

	return 0;
}
