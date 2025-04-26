#include <stdio.h>
#include <string.h>

static unsigned char mov_instruction_code = 0x22;

enum RegisterByte
{
	AL = 0,
	CL,
	DL,
	BL,
	AH,
	CH,
	DH,
	BH
};

enum RegisterWord
{
	AX = 0,
	CX,
	DX,
	BX,
	SP,
	BP,
	SI,
	DI
};

int is_mov_instruction(char byte)
{
	return byte == mov_instruction_code;
}

void get_word_register_name(char byte, char* register_name)
{
	unsigned long long num_bytes = strlen("AX") + 1;

	if ((byte & 7) == AX)
	{
		strcpy_s(register_name, num_bytes, "ax");
	}
	else if ((byte & 7) == CX)
	{
		strcpy_s(register_name, num_bytes, "cx");
	}
	else if ((byte & 7) == DX)
	{
		strcpy_s(register_name, num_bytes, "dx");
	}
	else if ((byte & 7) == BX)
	{
		strcpy_s(register_name, num_bytes, "bx");
	}
	else if ((byte & 7) == SP)
	{
		strcpy_s(register_name, num_bytes, "sp");
	}
	else if ((byte & 7) == BP)
	{
		strcpy_s(register_name, num_bytes, "bp");
	}
	else if ((byte & 7) == SI)
	{
		strcpy_s(register_name, num_bytes, "si");
	}
	else if ((byte & 7) == DI)
	{
		strcpy_s(register_name, num_bytes, "di");
	}
}

void get_byte_register_name(char byte, char* register_name)
{
	unsigned long long num_bytes = strlen("AL") + 1;

	if ((byte & 7) == AL)
	{
		strcpy_s(register_name, num_bytes, "al");
	}
	else if ((byte & 7) == CL)
	{
		strcpy_s(register_name, num_bytes, "cl");
	}
	else if ((byte & 7) == DL)
	{
		strcpy_s(register_name, num_bytes, "dl");
	}
	else if ((byte & 7) == BL)
	{
		strcpy_s(register_name, num_bytes, "bl");
	}
	else if ((byte & 7) == AH)
	{
		strcpy_s(register_name, num_bytes, "ah");
	}
	else if ((byte & 7) == CH)
	{
		strcpy_s(register_name, num_bytes, "ch");
	}
	else if ((byte & 7) == DH)
	{
		strcpy_s(register_name, num_bytes, "dh");
	}
	else if ((byte & 7) == BH)
	{
		strcpy_s(register_name, num_bytes, "bh");
	}
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
				//Not sure if this is relevant for register to register	
				/*if (((first_byte >> 1) & 0x01) == 0x01) //D = 1
				{
					printf("D = 1\n");
				}
				else if (((first_byte >> 1) & 0x00) == 0x00)
				{
					printf("D = 0\n");
				}*/

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
		}

	}
	

	return 0;	
}

