#include <stdio.h>
#include <string.h>

static unsigned char register_memory_mov = 0x22;
static unsigned char immediate_register_mov = 0x0b;

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

static char* EffectiveAddressWithDisplacement[8] =
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

void decode_Register_Memory_Mov(unsigned char first_byte, FILE* fp)
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
	else if (((second_byte >> 6) & 3) == 1) //MOD == 01 (memory mode with 8-bit displacement)
	{				
		//The register o registers used depend on R/M
		char* registers = EffectiveAddressWithDisplacement[second_byte & 7];

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
	else if (((second_byte >> 6) & 3) == 2) //MOD == 10 (memory mode with 16-bit displacement)
	{
		//The register o registers used depend on R/M
		char* registers = EffectiveAddressWithDisplacement[second_byte & 7];

		//Read 16-bit displacement
		unsigned char third_byte = getc(fp);
		unsigned char forth_byte = getc(fp);
		short displacement = (forth_byte << 8) | third_byte;

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
			printf("%s, %s + %d]\n", register_to_load, registers, displacement);
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
			printf("%s + %d], %s\n", registers, displacement, register_to_store);
		}
	}
	else if (((second_byte >> 6) & 3) == 0) //MOD == 00 (no displacement except when R/M = 110)
	{			
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

			if ((second_byte & 7) != 6)
			{
				char* registers = EffectiveAddressWithDisplacement[second_byte & 7];
				printf("%s, %s]\n", register_to_load, registers);
			}
			else
			{
				unsigned char third_byte = getc(fp);
				unsigned char forth_byte = getc(fp);
				short direct_address = (forth_byte << 8) | third_byte;
				printf("%s, [%d]\n", register_to_load, direct_address);
			}
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

			if ((second_byte & 7) != 6)
			{
				char* registers = EffectiveAddressWithDisplacement[second_byte & 7];
				printf("[%s], %s\n", registers, register_to_store);
			}
			else
			{
				unsigned char third_byte = getc(fp);
				unsigned char forth_byte = getc(fp);
				short direct_address = (forth_byte << 8) | third_byte;
				printf("[%d], %s\n", direct_address, register_to_store);
			}
		}
	}
}

void decode_Immediate_Register_Mov(unsigned char first_byte, FILE* fp)
{
	printf("mov ");

	unsigned char second_byte = getc(fp);

	//get register to load
	char* register_to_load = "register";
	if ((first_byte >> 3 & 1) == 1) //W = 1
	{
		get_word_register_name(first_byte & 7, register_to_load);

		unsigned char third_byte = getc(fp);
		short immediate_value = (third_byte << 8) | second_byte;
		printf("%s, %d\n", register_to_load, immediate_value);
	}
	else if ((first_byte >> 3 & 1) == 0) //W = 0
	{
		get_byte_register_name(first_byte & 7, register_to_load);
		printf("%s, %d\n", register_to_load, second_byte);
	}
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
		if (first_byte >> 2 == register_memory_mov)
		{
			decode_Register_Memory_Mov(first_byte, fp);
		}
		else if ((first_byte >> 4) == immediate_register_mov)
		{
			decode_Immediate_Register_Mov(first_byte, fp);
		}

	}
	
	return 0;	
}

