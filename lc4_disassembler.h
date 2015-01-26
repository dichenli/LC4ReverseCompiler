#include <stdio.h>
#include "lc4_memory.h"

/* 
 * declarations of functions that must defined in lc4_disassembler.c
 */



/**
 * translates the hex representation of arithmetic instructions
 * into their assembly equivalent in the given linked list.
 * 
 * returns 0 upon success, and non-zero if an error occurs.
 */
 
unsigned short int extract_bits(unsigned short int num, unsigned short int i, unsigned short int j);

short int extract_signed_bits(short int num, unsigned short int i, unsigned short int j);

int code_or_data(unsigned short int address);

void switch_br_nop(unsigned short int contents, char* assembly, unsigned short int address, struct row_of_memory* memory);

void switch_arith(unsigned short int contents, char* assembly);

void switch_cmp(unsigned short int contents, char* assembly);

void switch_jsr(unsigned short int contents, char* assembly, unsigned short int address, struct row_of_memory* memory);

void switch_logic(unsigned short int contents, char* assembly); 

void switch_shift(unsigned short int contents, char* assembly);

void switch_jmp(unsigned short int contents, char* assembly, unsigned short int address, struct row_of_memory* memory);
 
char* translate(struct row_of_memory* node, char* assembly, struct row_of_memory* memory);

void write_header (FILE* wfile, struct row_of_memory* node);

int reverse_assemble (struct row_of_memory** memory) ;

int write_assembly(struct row_of_memory* memory, char* file_name);


