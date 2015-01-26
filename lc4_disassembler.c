#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lc4_memory.h"
/*
struct row_of_memory {
	short unsigned int address ;
	char * label ;
	short unsigned int contents ;
	char * assembly ;
	struct row_of_memory *next ;
} ;
*/


unsigned short int extract_bits(unsigned short int num, unsigned short int i, unsigned short int j) {
	/*i, j: start and end bits. Extract the bits from i to j of num as unsigned value*/
	
	unsigned short int size = sizeof(unsigned short int) * 8; // should be 16 for most systems
	if(i < 0 || j < 0 || i >= size || j >= size || i > j) {
		fprintf(stderr, "Wrong bits positions!\n");
		return 65535; //invalid, either bits out of range or i > j
	}
	
	num = num << (size - 1 - j);
	num = num >> (size - 1 - j + i);
	return num;
}

short int extract_signed_bits(short int num, unsigned short int i, unsigned short int j) {
	/*i, j: start and end bits. Extract the bits from i to j of num as signed value */
	
	unsigned short int size = sizeof(unsigned short int) * 8; // should be 16 for most systems
	if(i < 0 || j < 0 || i >= size || j >= size || i > j) {
		fprintf(stderr, "Wrong bits positions!\n");
		return 65535; //invalid, either bits out of range or i > j
	}
	
	num = num << (size - 1 - j);
	num = num >> (size - 1 - j + i); //bitwise shift will fill 1 in the beginning
	return num;
}

/*return 0 if address is .DATA, 1 if it's .CODE, 2 if exception*/
int code_or_data(unsigned short int address) {
	if((address >= 0x2000 && address < 0x8000) || address >= 0xA000) {
		return 0;
	} else if (address <= 0xFFFF) {
		return 1;
	} else { //may happen if short int has more than 16 bits
		return 2;
	}
}



void switch_br_nop(unsigned short int contents, char* assembly, unsigned short int address, struct row_of_memory* memory) {
	/* switch case: opcode 0000, BRnzp or NOP*/
	short int i;
	unsigned short int code;
	struct row_of_memory* temp = NULL;

	if (assembly == NULL) {
		fprintf(stderr, "Null pointer error! Debug it.\n");
		return;
	}
	
	
	code = extract_bits(contents, 9, 11);
	
	if( code == 0b000) {
		sprintf(assembly, "\tNOP");
		return;
	}
	
	i = extract_signed_bits(contents, 0, 8);
	temp = search_address (memory, i + address + 1); 
	
	switch(code) { //inspect 9 - 11 bits
		case 0b100:
			sprintf(assembly, "\tBRn %s", temp->label);
			break;
		case 0b110:
			sprintf(assembly, "\tBRnz %s", temp->label);
			break;
		case 0b101:
			sprintf(assembly, "\tBRnp %s", temp->label);
			break;
		case 0b010:
			sprintf(assembly, "\tBRz %s", temp->label);
			break;
		case 0b011:
			sprintf(assembly, "\tBRzp %s", temp->label);
			break;
		case 0b001:
			sprintf(assembly, "\tBRp %s", temp->label);
			break;
		case 0b111:
			sprintf(assembly, "\tBRnzp %s", temp->label);
			break;
		default:
			fprintf(stderr, "Exception! No assembly code matching. Debug it.\n");
			sprintf(assembly, "NOP");
	}
	return;
}

void switch_arith(unsigned short int contents, char* assembly) {
	if (assembly == NULL) {
		fprintf(stderr, "Null pointer error! Debug it.\n");
		return;
	}
	
	short unsigned int s, t, d;
	short int i;
	
	s = extract_bits(contents, 6, 8);
	t = extract_bits(contents, 0, 2);
	d = extract_bits(contents, 9, 11);
	i = extract_signed_bits(contents, 0, 4);
	
	switch(extract_bits(contents, 3, 5)) {
		case 0b000://ADD
			sprintf(assembly, "\tADD R%u R%u R%u", d, s, t);
			break;
		case 0b001:
			sprintf(assembly, "\tMUL R%u R%u R%u", d, s, t);
			break;
		case 0b010:
			sprintf(assembly, "\tSUB R%u R%u R%u", d, s, t);
			break;
		case 0b011:
			sprintf(assembly, "\tDIV R%u R%u R%u", d, s, t);
			break;
		default: //ADD const
			sprintf(assembly, "\tADD R%u R%u #%d", d, s, i);
	
	}
}

void switch_cmp(unsigned short int contents, char* assembly)  {
	if (assembly == NULL) {
		fprintf(stderr, "Null pointer error! Debug it.\n");
		return;
	}
	
	short unsigned int s, t, u;
	short int i;
	s = extract_bits(contents, 9, 11);
	t = extract_bits(contents, 0, 2);
	u = extract_bits(contents, 0, 6);
	i = extract_signed_bits(contents, 0, 6);
		
	switch(extract_bits(contents, 7, 8)) {
		case 0b00://CMP Rs, Rt
			sprintf(assembly, "\tCMP R%u R%u", s, t);
			break;
		case 0b01:
			sprintf(assembly, "\tCMPU R%u R%u", s, t);
			break;
		case 0b10:
			sprintf(assembly, "\tCMPI R%u #%d", s, i);
			break;
		case 0b11:
			sprintf(assembly, "\tCMPIU R%u #%u", s, u);
			break;
		default: //error
			fprintf(stderr, "Exception! No assembly code matching. Debug it. (switch_cmp)\n");
			sprintf(assembly, "NOP");
	
	}
}


void switch_jsr(unsigned short int contents, char* assembly, unsigned short int address, struct row_of_memory* memory)  {
	if (assembly == NULL) {
		fprintf(stderr, "Null pointer error! Debug it.\n");
		return;
	}
	short unsigned int s;
	short int i;
	struct row_of_memory* temp = NULL;
	s = extract_bits(contents, 6, 8);
	i = extract_signed_bits(contents, 0, 10);

			
	switch(extract_bits(contents, 11, 11)) {
		case 0b0:
			sprintf(assembly, "\tJSRR R%u", s);
			break;
		case 0b1:
			temp = search_address (memory, (address&0x8000)|(i<<4)); //PC = (PC&x8000) | (IMM11<<4)
			if(temp == NULL) {
				printf("Error, temp == NULL, in switch_jsr\n");
				return;
			}
			sprintf(assembly, "\tJSR %s", temp->label);
			break;
		default: 
			fprintf(stderr, "Exception! No assembly code matching. Debug it. (switch_cmp)\n");
			sprintf(assembly, "NOP");
	}
}


void switch_logic(unsigned short int contents, char* assembly)  {
	if (assembly == NULL) {
		fprintf(stderr, "Null pointer error! Debug it.\n");
		return;
	}
	short unsigned int s, t, d;
	short int i;

	s = extract_bits(contents, 6, 8);
	t = extract_bits(contents, 0, 2);
	d = extract_bits(contents, 9, 11);
	i = extract_signed_bits(contents, 0, 4);
	
	switch(extract_bits(contents, 3, 5)) {
		case 0b000://AND
			sprintf(assembly, "\tAND R%u R%u R%u", d, s, t);
			break;
		case 0b001:
			sprintf(assembly, "\tNOT R%u R%u", d, s);
			break;
		case 0b010:
			sprintf(assembly, "\tOR R%u R%u R%u", d, s, t);
			break;
		case 0b011:
			sprintf(assembly, "\tXOR R%u R%u R%u", d, s, t);
			break;
		default: //AND const
			sprintf(assembly, "\tAND R%u R%u #%d", d, s, i);
	}
}


void switch_shift(unsigned short int contents, char* assembly)  {
	if (assembly == NULL) {
		fprintf(stderr, "Null pointer error! Debug it.\n");
		return;
	}
	short unsigned int s, t, d, u;

	s = extract_bits(contents, 6, 8);
	t = extract_bits(contents, 0, 2);
	d = extract_bits(contents, 9, 11);
	u = extract_signed_bits(contents, 0, 3);
	
	switch(extract_bits(contents, 4, 5)) {
		case 0b00://AND
			sprintf(assembly, "\tSLL R%u R%u #%u", d, s, u);
			break;
		case 0b01:
			sprintf(assembly, "\tSRA R%u R%u #%u", d, s, u);
			break;
		case 0b10:
			sprintf(assembly, "\tSRL R%u R%u #%u", d, s, u);
			break;
		case 0b11:
			sprintf(assembly, "\tMOD R%u R%u R%u", d, s, t);
			break;
		default: //AND const
			fprintf(stderr, "Exception! No assembly code matching. Debug it. (switch_cmp)\n");
			sprintf(assembly, "NOP");
	}
}


void switch_jmp(unsigned short int contents, char* assembly, unsigned short int address, struct row_of_memory* memory)  {
	if (assembly == NULL) {
		fprintf(stderr, "Null pointer error! Debug it.\n");
		return;
	}
	short unsigned int s;
	short int i;
	struct row_of_memory* temp = NULL;

	s = extract_bits(contents, 6, 8);
	i = extract_signed_bits(contents, 0, 10);
			
	switch(extract_bits(contents, 11, 11)) {
		case 0b0:
			sprintf(assembly, "\tJMPR R%u", s);
			break;
		case 0b1:
			temp = search_address (memory, i + address + 1); 
			if(temp == NULL) {
				return;
			}
			sprintf(assembly, "\tJMP %s", temp->label);
			break;
		default: 
			fprintf(stderr, "Exception! No assembly code matching. Debug it. (switch_cmp)\n");
			sprintf(assembly, "NOP");
	}
}
		
char* translate(struct row_of_memory* node, char* assembly, struct row_of_memory* memory) {
		
	unsigned int u;
	short int i;
	unsigned short int s, t, d; //variables in LC4 assembly instruction
	
	if(assembly == NULL) {
		return NULL;
	}
	
	//when it is .DATA
	if(code_or_data(node->address) == 0) {
		sprintf(assembly, "\t.FILL x%02X", node->contents);
		return assembly;
	}
	
	//when it is .CODE
	switch(extract_bits(node->contents, 12, 15)) {
		case 0b0000: //NOP or BRnzp
			switch_br_nop(node->contents, assembly, node->address, memory);
			break;
		case 0b0001: //ADD SUB DIV MUL
			switch_arith(node->contents, assembly);
			break;
		case 0b0010: //CMP
			switch_cmp(node->contents, assembly);
			break;
		case 0b0100: //JSR, JSRR
			switch_jsr(node->contents, assembly, node->address, memory);
			break;
		case 0b0101: //AND, OR, NOT, XOR
			switch_logic(node->contents, assembly);
			break;
		case 0b0110: //LDR
			i = extract_signed_bits(node->contents, 0, 5);
			s = extract_bits(node->contents, 6, 8);
			d = extract_bits(node->contents, 9, 11);			
			sprintf(assembly, "\tLDR R%u R%u #%d", d, s, i);
			break;
		case 0b0111: //STR
			i = extract_signed_bits(node->contents, 0, 5);
			s = extract_bits(node->contents, 6, 8);
			t = extract_bits(node->contents, 9, 11);			
			sprintf(assembly, "\tSTR R%u R%u #%d", t, s, i);
			break;
		case 0b1000: //RTI
			sprintf(assembly, "\tRTI");
			break;
		case 0b1001: //CONST
			i = extract_signed_bits(node->contents, 0, 8);
			d = extract_bits(node->contents, 9, 11);			
			sprintf(assembly, "\tCONST R%u #%d", d, i);
			break;
		case 0b1010: //SLL, SRA, SRL, MOD
			switch_shift(node->contents, assembly);
			break;
		case 0b1100: //JMPR, JMP
			switch_jmp(node->contents, assembly, node->address, memory);
			break;
		case 0b1101: //HICONST
			u = extract_bits(node->contents, 0, 7);
			d = extract_bits(node->contents, 9, 11);			
			sprintf(assembly, "\tHICONST R%u #%u", d, u);			
			break;
		case 0b1111: //TRAP
			u = extract_bits(node->contents, 0, 7);
			sprintf(assembly, "\tTRAP #%u", u);			
			break;
		default:
			fprintf(stderr, "Exception! No assembly code matching. Debug it.\n");
			sprintf(assembly, "\tNOP");
			return NULL;
	}
	return assembly;
	
}


int reverse_assemble (struct row_of_memory** memory) 
{	
	struct row_of_memory* temp = NULL;
	char* assembly = NULL;
	int i = 0;


	for(temp = *memory; temp != NULL; temp = temp->next) {
		assembly = malloc(100 * sizeof(char)); 
		if (assembly == NULL) {
			fprintf(stderr, "Malloc fail in assembly string.\n");
			return 1;
		}
	
		if(translate(temp, assembly, *memory) == NULL) {
			continue;
		}
		for(i = 0; assembly[i] != '\0'; i++);
		temp->assembly = malloc(sizeof(char) * (i + 2));
		
		if(temp->assembly == NULL) {
			fprintf(stderr, "Assembly string memory allocation mistake\n");
			return 1;
		}
		
		for(i = 0; assembly[i] != '\0'; i++) {
			temp->assembly[i] = assembly[i];
		}
		temp->assembly[i] = '\0';
		
		free(assembly);

	}
	return 0 ;
}

//write .DATA, .CODE, 
void write_header (FILE* wfile, struct row_of_memory* node) {
	
	switch(code_or_data(node->address)){
		case 0:
			fprintf(wfile, ".DATA\n");
			break;
		case 1:
			fprintf(wfile, ".CODE\n");
			break;
		default: //do nothing
			break;
	}
	fprintf(wfile, ".ADDR x%04X\n", node->address);
	
	if(node->label != NULL) {
		fprintf(wfile, "%s\n", node->label);
	}
	return;
}

int write_assembly(struct row_of_memory* memory, char* file_name) {
	struct row_of_memory* temp = NULL;
	FILE* wfile; // file to write to
	int consec = 0; // address is consecutive?	
	int i;
	int os = 0;

	if(memory == NULL) {
		return 0;
	}
	
	// manipulate file name, change from foo.obj to foo.asm
	// find the index to the last '.'
	for(i = strlen(file_name) - 1; file_name[i] != '.' && i >=0; i--) {
		//do nothing here
	}
	if(i > 0) { // replace the last '.' char by '\0', so file type extension (.obj) will be erased
		file_name[i] = '\0';
	}
	strcat(file_name, ".asm");
	
	wfile = fopen(file_name, "w");
	if(wfile == NULL) {
		fprintf(stderr, "Can't open the file!\n");
	}
	
	//write to file
	write_header(wfile, memory);
	
	for(temp = memory; temp != NULL; temp = temp->next){
		if(temp->address >= 0x8000 && os == 0) {
			os = 1;
			fprintf(wfile, ".OS\n");
		}
		
		fprintf(wfile, "%s\n", temp->assembly);
		
		if(temp->next != NULL) {
			if( temp->next->label != NULL || temp->next->address - temp->address > 1) {
				write_header(wfile, temp->next);
			}
		}
	}
	
	if(fclose(wfile) != 0) {
		fprintf(stderr, "file close failed.\n");
		return 1;
	}
	return 0;
}














