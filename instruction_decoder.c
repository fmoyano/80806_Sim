#include <stdio.h>
#include <string.h>

static unsigned char mov_instruction_code = 0x22;

static char* RegisterByteNames[8] =
{
	"al",
	"cl",
	"dl",
	"bl",
	"ah",
	"ch",
	"dh",
	"bh"
};

static char* RegisterWordNames[8] =
{
	"ax",
	"cx",
	"dx",
	"bx",
	"sp",
	"bp",
	"si",
	"di"
};


int is_mov_instruction(char byte)
{
	return byte == mov_instruction_code;
}

void get_word_register_name(char byte, char* register_name)
{
	unsigned long long num_bytes = strlen("AX") + 1;
	strcpy_s(register_name, num_bytes, RegisterWordNames[byte & 7]);	
}

void get_byte_register_name(char byte, char* register_name)
{
	unsigned long long num_bytes = strlen("AL") + 1;
	strcpy_s(register_name, num_bytes, RegisterByteNames[byte & 7]);
}

int main(void)
{
	FILE *fp = fopen("decode_2", "rb");
	if (fp == NULL)
	{
		printf("Problem when opening file");
		return -1;
	}

	//printf ("; This is a sample comment\n");
	printf("bits 16\n\n");

	while (1)
	{
		int first_byte_int = getc(fp);
		if (first_byte_int == EOF) break;

		unsigned char first_byte = (unsigned char)first_byte_int;
		if (is_mov_instruction(first_byte >> 2))
		{
			printf("mov ");
			unsigned char second_byte = getc(fp);

			if (((second_byte >> 6) & 0x03) == 0x03) //MOD == 11 (register to register)
			{						
				char destination_register[] = "KO";
				char source_register[] = "KO";
				if ((first_byte & 0x03) == 0x01) //W = 1
				{
					get_word_register_name(second_byte, destination_register);
					get_word_register_name(second_byte >> 3, source_register);
					printf("%s, %s\n", destination_register, source_register);
				}
				else if ((first_byte & 0x03) == 0x00) //W = 0
				{
					get_byte_register_name(second_byte, destination_register);
					get_byte_register_name(second_byte >> 3, source_register);
					printf("%s, %s\n", destination_register, source_register);
				}
			}
			else
			{
				/*if (((first_byte >> 1) & 0x01) == 0x01) //D = 1 (LOAD)
				{
					printf("D = 1\n");
				}
				else if (((first_byte >> 1) & 0x00) == 0x00) //D = 0 (STORE)
				{
					printf("D = 0\n");
				}*/
			}
		}

	}
	

	return 0;	
}

