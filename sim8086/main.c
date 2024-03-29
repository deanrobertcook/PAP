#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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

char *get_ea_with_displ(int RM, int displ)
{
    const char *base = ea_base[RM];
    char *base_w_disp = (char *)calloc(30, sizeof(char));

    size_t len_base = strnlen(base, 10);

    char displ_str[10];
    sprintf(displ_str, "%d", displ);
    size_t len_disp = strnlen(displ_str, 10);

    int i;
    for (i = 0; i < len_base - 1; i++)
    {
        base_w_disp[i] = base[i];
    }
    base_w_disp[i++] = ' ';
    base_w_disp[i++] = '+';
    base_w_disp[i++] = ' ';
    for (int j = 0; j < len_disp; j++)
    {
        base_w_disp[i++] = displ_str[j];
    }
    base_w_disp[i++] = ']';
    return base_w_disp;
}

void print_memory_to_reg(const char *opcode, int RM, int D, const char *reg, int displ)
{
    char *base_w_disp = get_ea_with_displ(RM, displ);
    if (D)
        printf("%s %s, %s\n", opcode, reg, base_w_disp);
    else
        printf("%s %s, %s\n", opcode, base_w_disp, reg);
    free(base_w_disp);
}

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

unsigned char *read_bytes(const char *file_name, long *file_len)
{
    FILE *file;
    unsigned char *buffer;

    file = fopen(file_name, "rb");
    if (!file)
    {
        perror("Unable to open file");
        *file_len = -1;
        return NULL;
    }

    // Seek to the end to determine the file size
    fseek(file, 0, SEEK_END);
    *file_len = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (unsigned char *)malloc(*file_len + 1);
    if (!buffer)
    {
        perror("Memory error!\n");
        fclose(file);
        *file_len = -1;
        return NULL;
    }

    fread(buffer, *file_len, 1, file);
    fclose(file);
    return buffer;
}

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        printf("Missing filename of binary to be disassembled\n");
        return 1;
    }

    long file_len;
    unsigned char *buffer = read_bytes(argv[1], &file_len);

    if (!buffer)
    {
        perror("Memory error!\n");
        return 1;
    }

    printf("bits 16\n");

    int i = 0;

    while (i < file_len)
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
            int D = buffer[i] & (1 << 1);

            int MOD = (buffer[i + 1] >> 6) & 0b11;
            int REG = (buffer[i + 1] >> 3) & 0b111;
            const char *reg = reg_w_map[(REG << 1) + W];
            int RM = buffer[i + 1] & 0b111;

            switch (MOD)
            {
            case 0b00:
            {
                print_memory_to_reg("MOV", RM, D, reg, 0);
                i += 2;
                break;
            }

            case 0b01:
            {
                int disp = buffer[i + 2];
                print_memory_to_reg("MOV", RM, D, reg, disp);
                i += 3;
                break;
            }

            case 0b10:
            {
                int disp = (buffer[i + 3] << 8) + buffer[i + 2];
                print_memory_to_reg("MOV", RM, D, reg, disp);
                i += 4;
                break;
            }

            case 0b11:
            {
                const char *rm = reg_w_map[(RM << 1) + W];
                printf("MOV %s, %s\n", rm, reg);
                i += 2;
                break;
            }
            }
        }
        else if (buffer[i] >> 2 == 0b00000) // ADD reg/mem with reg to either
        {
            int W = buffer[i] & 1;
            int D = buffer[i] & (1 << 1);

            int MOD = (buffer[i + 1] >> 6) & 0b11;
            int REG = (buffer[i + 1] >> 3) & 0b111;
            const char *reg = reg_w_map[(REG << 1) + W];
            int RM = buffer[i + 1] & 0b111;

            switch (MOD)
            {
            case 0b00:
            {
                print_memory_to_reg("ADD", RM, D, reg, 0);
                i += 2;
                break;
            }

            case 0b01:
            {
                int disp = buffer[i + 2];
                print_memory_to_reg("ADD", RM, D, reg, disp);
                i += 3;
                break;
            }

            case 0b10:
            {
                int disp = (buffer[i + 3] << 8) + buffer[i + 2];
                print_memory_to_reg("ADD", RM, D, reg, disp);
                i += 4;
                break;
            }

            case 0b11:
            {
                const char *rm = reg_w_map[(RM << 1) + W];
                printf("ADD %s, %s\n", rm, reg);
                i += 2;
                break;
            }
            }
        }
        else if (buffer[i] >> 2 == 0b100000) // ADD immediate to reg/mem
        {
            int SW = buffer[i] & 0b11;
            int W = buffer[i] & 1;
            int S = buffer[i] & (1 << 1);

            int MOD = (buffer[i + 1] >> 6) & 0b11;
            int RM = buffer[i + 1] & 0b111;

            char *w_str = W ? "word" : "byte";

            int data = buffer[i + 2];
            if (SW == 0b01)
                data = data + (buffer[i + 3] << 8); // TODO(dean) need to properly handle sign extension
            int data_len = SW == 0b01 ? 2 : 1;

            switch (MOD)
            {
            case 0b11:
            {
                const char *rm = reg_w_map[(RM << 1) + W];
                printf("ADD %s, %d\n", rm, data);
                i += 2 + data_len;
                break;
            }
            case 0b00:
            {
                char *ea_w_disp = get_ea_with_displ(RM, 0);
                printf("ADD %s %s, %d\n", w_str, ea_w_disp, data);
                free(ea_w_disp);
                i += 2 + data_len;
                break;
            }

            case 0b01:
            {
                int disp = buffer[i + 2];
                char *ea_w_disp = get_ea_with_displ(RM, disp);
                printf("ADD %s %s, %d\n", w_str, ea_w_disp, data);
                free(ea_w_disp);
                i += 3 + data_len;
                break;
            }

            case 0b10:
            {
                int disp = (buffer[i + 3] << 8) + buffer[i + 2];
                char *ea_w_disp = get_ea_with_displ(RM, disp);
                printf("ADD %s %s, %d\n", w_str, ea_w_disp, data);
                free(ea_w_disp);
                i += 4 + data_len;
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
