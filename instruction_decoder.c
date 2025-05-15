#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SHORT_VAL_CHARACTERS 5
#define MAX_LENGTH_DISPLACEMENT_TEXT (MAX_SHORT_VAL_CHARACTERS + 5)
#define BASE_10 10
#define MAX_JUMP_NAME 10

//Movs
static unsigned char register_memory_mov = 0x22;
static unsigned char immediate_register_mov = 0x0b;
static unsigned char immediate_register_memory_mov = 0x63;
static unsigned char memory_to_accumulator_mov = 0x50;
static unsigned char accumulator_to_memory_mov = 0x51;

//Arith operators (add, sub, cmp)
static unsigned char immediate_register_mem_arith = 0x20;
static unsigned char register_memory_add = 0;
static unsigned char immediate_accum_add = 0x02;
static unsigned char register_memory_sub = 0x0a;
static unsigned char immediate_accum_sub = 0x16;
static unsigned char register_memory_cmp = 0x0e;
static unsigned char immediate_accum_cmp = 0x1e;

//Jumps
static unsigned char je = 0x74;

static char* RegisterByteNames[] =
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

static char* RegisterWordNames[] =
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

static char* EffectiveAddressWithDisplacement[] =
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

static char* Arith_Ops[] =
{
	"add ", "", "", "", "", "sub ", "", "cmp "
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

void get_corrected_displacement_text(short val, char* text_to_print)
{
	if (val == 0)
	{
		sprintf_s(text_to_print, 2, "]");
	}
	else
	{
		short abs_val = abs(val);
		char value_buffer[MAX_SHORT_VAL_CHARACTERS + 1];
		_itoa_s(abs_val, value_buffer, MAX_SHORT_VAL_CHARACTERS + 1, BASE_10);

		if (val < 0)
		{
			sprintf_s(text_to_print, MAX_LENGTH_DISPLACEMENT_TEXT, " - %s]", value_buffer);
		}
		else
		{
			sprintf_s(text_to_print, MAX_LENGTH_DISPLACEMENT_TEXT, " + %s]", value_buffer);
		}		
	}
}

void decode_register_memory_op(unsigned char first_byte, FILE* fp, int is_mov_op, int arith_op)
{
	if (is_mov_op)
		printf("mov ");
	else
		printf("%s ", Arith_Ops[arith_op]);

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
			printf("%s, %s", register_to_load, registers);
			char displacement_to_print[MAX_LENGTH_DISPLACEMENT_TEXT];
			get_corrected_displacement_text((signed char)third_byte, displacement_to_print);
			printf("%s\n", displacement_to_print);
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
			printf("%s", registers);
			char displacement_to_print[MAX_LENGTH_DISPLACEMENT_TEXT];
			get_corrected_displacement_text((signed char)third_byte, displacement_to_print);
			printf("%s, %s\n", displacement_to_print, register_to_store);
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
			printf("%s, %s", register_to_load, registers);
			char displacement_to_print[MAX_LENGTH_DISPLACEMENT_TEXT];
			get_corrected_displacement_text(displacement, displacement_to_print);
			printf("%s\n", displacement_to_print);
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
			printf("%s", registers);
			char displacement_to_print[MAX_LENGTH_DISPLACEMENT_TEXT];
			get_corrected_displacement_text(displacement, displacement_to_print);
			printf("%s, %s\n", displacement_to_print, register_to_store);

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

			if ((second_byte & 7) != 6) //RM !=6
			{
				char* registers = EffectiveAddressWithDisplacement[second_byte & 7];
				printf("%s], %s\n", registers, register_to_store);
			}
			else //RM = 6 (direct address, no registers involved)
			{
				unsigned char third_byte = getc(fp);
				unsigned char forth_byte = getc(fp);
				short direct_address = (forth_byte << 8) | third_byte;
				printf("[%d], %s\n", direct_address, register_to_store);
			}
		}
	}
}

void decode_immediate_register_mov(unsigned char first_byte, FILE* fp)
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
		printf("%s, %d\n", register_to_load, (signed char)second_byte);
	}
}

void decode_immediate_register_memory_op(unsigned char first_byte, FILE* fp, int is_mov)
{
	unsigned char second_byte = getc(fp);

	if (is_mov)
		printf("mov ");
	else
		printf("%s ", Arith_Ops[second_byte >> 3 & 7]);
	

	//Check MOD value (if we're moving the immediate value to register or to memory with displacement)
	if ((second_byte >> 6 & 3) == 0) //MOD = 0 (No displacement)
	{
		if ((second_byte & 7) != 6) //RM != 6 (use registers for address)
		{
			char* registers = EffectiveAddressWithDisplacement[second_byte & 7];
			if ((first_byte & 1) == 1) //W=1 (16 bit immediate value)
			{
				unsigned char third_byte = getc(fp);
				short immediate_value = third_byte;

				if (is_mov || (first_byte >> 1 & 1) == 0) //S=0 for arith ops, meaning we should read the high byte value
				{
					unsigned char forth_byte = getc(fp);
					immediate_value = forth_byte << 8 | third_byte;
				}

				printf("%s], word %d\n", registers, immediate_value);
			}
			else if ((first_byte & 1) == 0) //W=0 (8 bit immediate value)
			{
				unsigned char third_byte = getc(fp);
				printf("%s], byte %d\n", registers, third_byte);
			}
		}
		else //RM = 6 (direct address)
		{
			if ((first_byte & 1) == 1) //W=1 (16 bit immediate value)
			{
				unsigned char third_byte = getc(fp);
				unsigned char forth_byte = getc(fp);
				short direct_address = forth_byte << 8 | third_byte;

				unsigned char fifth_byte = getc(fp);
				short immediate_value = fifth_byte;
				if (is_mov || (first_byte >> 1 & 1) == 0) //S=0 for arith ops, meaning we should read the high byte value
				{
					unsigned char sixth_byte = getc(fp);
					immediate_value = sixth_byte << 8 | fifth_byte;
				}

				printf("[%d], word %d\n", direct_address, immediate_value);
			}
			else if ((first_byte & 1) == 0) //W=0 (8 bit immediate value)
			{
				unsigned char third_byte = getc(fp);
				unsigned char forth_byte = getc(fp);
				short direct_address = forth_byte << 8 | third_byte;

				unsigned char fifth_byte = getc(fp);				
				printf("[%d], byte %d\n", direct_address, fifth_byte);
			}
		}
	}
	else if ((second_byte >> 6 & 3) == 1) //MOD = 1 (8-bit displacement)
	{
		char* registers = EffectiveAddressWithDisplacement[second_byte & 7];
		if ((first_byte & 1) == 1) //W=1 (16 bit immediate value)
		{
			unsigned char displacement = getc(fp);
			unsigned char low_value = getc(fp);
			unsigned char high_value = getc(fp);
			short immediate_value = high_value << 8 | low_value;
			printf("%s + %d], word %d\n", registers, displacement, immediate_value);
		}
		else if ((first_byte & 1) == 0) //W=0 (8 bit immediate value)
		{
			unsigned char displacement = getc(fp);
			unsigned char low_value = getc(fp);						
			printf("%s + %d], byte %d\n", registers, displacement, low_value);
		}
	}
	else if ((second_byte >> 6 & 3) == 2) //MOD = 2 (16-bit displacement)
	{
		char* registers = EffectiveAddressWithDisplacement[second_byte & 7];
		if ((first_byte & 1) == 1) // W=1 (16 bit immediate value)			
		{
			unsigned char displacement_low = getc(fp);
			unsigned char displacement_high = getc(fp);
			short displacement = displacement_high << 8 | displacement_low;

			unsigned char low_value = getc(fp);
			short immediate_value = low_value;
			if (is_mov || (first_byte >> 1 & 1) == 0) //S=0 for arith ops means one extra byte for immediate value
			{
				unsigned char high_value = getc(fp);
				immediate_value = high_value << 8 | low_value;	
			}			
			printf("%s + %d], word %d\n", registers, displacement, immediate_value);
		}
		else if ((first_byte & 1) == 0) //W=0 (8 bit immediate value)			
		{
			unsigned char displacement_low = getc(fp);
			unsigned char displacement_high = getc(fp);
			short displacement = displacement_high << 8 | displacement_low;
			unsigned char low_value = getc(fp);						
			printf("%s + %d], byte %d\n", registers, displacement, low_value);
		}
	}
	//This is a weird case that I think it's not used for mov instructions, but only for add/sub/cmp instructions
	else if ((second_byte >> 6 & 3) == 3) //MOD = 3 (register-to-register)
	{
		if ((first_byte & 1) == 1) //W = 1 (destination is word register)
		{
			char register_to_load[] = "KO";
			get_word_register_name(second_byte & 7, register_to_load);
			unsigned char third_byte = getc(fp);
			short immediate_value = third_byte;
			if ((first_byte >> 1 & 1) == 0) //S=0
			{
				unsigned char forth_byte = getc(fp);
				immediate_value = (forth_byte << 8) | third_byte;
			}			
			printf("%s, %d\n", register_to_load, immediate_value);
		}
		else if ((first_byte & 1) == 0) //W = 0 (destination is byte register)
		{
			char register_to_load[] = "KO";
			get_byte_register_name(second_byte & 7, register_to_load);
			unsigned char third_byte = getc(fp);								
			printf("%s, %d\n", register_to_load, third_byte);
		}
	}	
}

void decode_accumulator(unsigned char first_byte, FILE* fp, int is_mov, int to_accumulator)
{
	if (is_mov)
		printf("mov ");
	else
		printf("%s ", Arith_Ops[first_byte >> 3 & 7]);
	
	unsigned char addr_low = getc(fp);
	if ((first_byte & 1) == 1) //W = 1 (16 bit address)
	{
		unsigned char addr_high = getc(fp);
		short addr = addr_high << 8 | addr_low;

		if (is_mov && to_accumulator)
		{
			printf("ax, [%d]\n", addr);
		}
		else if (is_mov)
		{
			printf("[%d], ax\n", addr);
		}
		else //16-bit immediate value to accumulator (ax)
		{			
			printf("ax, %d\n", addr); //addr is actually a 16-bit immediate value
		}
	}
	else if ((first_byte & 1) == 0) //W = 0 (8 bit address)
	{
		if (is_mov && to_accumulator)
		{
			printf("ax, [%d]\n", addr_low);
		}
		else if (is_mov)
		{
			printf("[%d], ax\n", addr_low);
		}
		else //8-bit immediate value to accumulator (al)
		{
			printf("al, %d\n", (signed char)addr_low); //addr_low is actually an 8-bit immediate value (and it can be negative)
		}
	}
}

int is_jump(unsigned char instruction, char* jump)
{
	int is_jump = 1;

	if (instruction == je)
	{
		strcpy_s(jump, MAX_JUMP_NAME, "je");
	}

	return is_jump;
}

int main(int argc, char** argv)
{		
	if (argc < 2)
	{
		printf("Pass the name of the executable\n");
		return -1;
	}

	//FILE *fp = fopen("simple_arith", "rb");
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
		if (first_byte >> 2 == register_memory_mov ||
				first_byte >> 2 == register_memory_add ||
				first_byte >> 2 == register_memory_sub ||
				first_byte >> 2 == register_memory_cmp)
		{
			decode_register_memory_op(first_byte, fp, first_byte >> 2 == register_memory_mov, first_byte >> 3 & 7);
		}
		else if ((first_byte >> 4) == immediate_register_mov)
		{
			decode_immediate_register_mov(first_byte, fp);
		}
		else if ((first_byte >> 1) == immediate_register_memory_mov ||
			 (first_byte >> 2) == immediate_register_mem_arith)
		{
			decode_immediate_register_memory_op(first_byte, fp, (first_byte >> 1) == immediate_register_memory_mov);
		}
		else if ((first_byte >> 1) == memory_to_accumulator_mov || 
			(first_byte >> 1) == accumulator_to_memory_mov || 
			(first_byte >> 1) == immediate_accum_add ||
			(first_byte >> 1) == immediate_accum_sub ||
			(first_byte >> 1) == immediate_accum_cmp)
		{
			decode_accumulator(first_byte, fp, (first_byte >> 1) == memory_to_accumulator_mov || 
			(first_byte >> 1) == accumulator_to_memory_mov, ((first_byte >> 1 & 1) == 0));
		}
		else
		{
			char jump[MAX_JUMP_NAME] = "KO";
			if (is_jump(first_byte, jump))
			{
				printf("%s ", jump);
				//read jump destination in second byte
				signed char offset = (signed char)fgetc(fp);

				//This is something we need to do if we want to create an .asm file that is equivalent to using labels when assembled with NASM
				//je $+0 means to jump 2 bytes before 
				//jnz $+-2 means to jump 4 bytes before
				//Which is equivalent to...
				//label:
				//je label
				//jnz label
				//But this saves the work of having to printf labels (moving up and down)
				signed char final_offset = offset + 2;
				printf("$+%d\n", final_offset);
			}
		}
	}
	
	return 0;	
}

