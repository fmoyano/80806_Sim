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

static char* EffectiveAddressMod01[8] =
{
	"[bx + si",
	"[bx + di",
	"[bp + si",
	"[bp + di",
	"[si",
	"[di",
	"[bp",
	"[bx",
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

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("Pass the name of the executable\n");
		return -1;
	}

	FILE *fp = fopen(argv[1], "rb");
	if (fp == NULL)
	{
		printf("Problem when opening file");
		return -1;
	}

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

			if (((second_byte >> 6) & 3) == 3) //MOD == 11 (register to register)
			{						
				char destination_register[] = "KO";
				char source_register[] = "KO";
				if ((first_byte & 1) == 1) //W = 1
				{
					get_word_register_name(second_byte, destination_register);
					get_word_register_name(second_byte >> 3, source_register);
					printf("%s, %s\n", destination_register, source_register);
				}
				else if ((first_byte & 1) == 0) //W = 0
				{
					get_byte_register_name(second_byte, destination_register);
					get_byte_register_name(second_byte >> 3, source_register);
					printf("%s, %s\n", destination_register, source_register);
				}
			}
			else if (((second_byte >> 6) & 1) == 1) //MOD == 01 (memory mode with 8-bit displacement)
			{				
				//The register o registers used depend on R/M
				char* registers = EffectiveAddressMod01[second_byte & 7];

				//Read 8-bit displacement
				unsigned char third_byte = getc(fp);

				if (((first_byte >> 1) & 1) == 1) //D = 1 (LOAD)
				{
					//get register to load
					char* register_to_load = "register";
					if ((first_byte & 1) == 1) //W = 1
					{
						get_word_register_name(second_byte >> 3, register_to_load);
					}
					else if ((first_byte & 1) == 0) //W = 0
					{
						get_byte_register_name(second_byte >> 3, register_to_load);
					}

					//To register from memory
					printf("%s, %s + %d]\n", register_to_load, registers, third_byte);
				}
				else if (((first_byte >> 1) & 1) == 0) //D = 0 (STORE)
				{
					//get register to store
					char* register_to_store = "register";
					if ((first_byte & 1) == 1) //W = 1
					{
						get_word_register_name(second_byte >> 3, register_to_store);
					}
					else if ((first_byte & 1) == 0) //W = 0
					{
						get_byte_register_name(second_byte >> 3, register_to_store);
					}

					//To memory from register
					printf("%s + %d], %s\n", registers, third_byte, register_to_store);
				}
			}
			else if (((second_byte >> 6) & 2) == 2) //MOD == 10 (memory mode with 16-bit displacement)
			{

			}
		}

	}
	

	return 0;	
}

