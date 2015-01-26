#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lc4_memory.h"


/* declarations of functions that must defined in lc4_loader.c */

char* get_file_name(char* file_name, unsigned long int length) {
	unsigned long int count;
	
	printf("Please enter the name of the .obj file you would like to open:");
	if(fgets(file_name, length, stdin) == NULL) {
		fprintf(stderr, "Input is too long!\n");
		return NULL;
	}
	count = strlen(file_name);
	file_name[--count] = '\0';
	return file_name;
}

FILE* get_file(char * file_name) 
{
	FILE* file = NULL;
	file = fopen(file_name, "rb");

	if(file == NULL) {
		fprintf(stderr, "Can't open the file!\n");
	}
	return file;
}

/*read two bytes from file and convert to HEX*/
unsigned short int read_word(FILE* my_obj_file) {
	unsigned short int content = 0;
	content = (fgetc(my_obj_file) << 8);
	content = content + (fgetc(my_obj_file) & 0x00ff);
	return content;
}

int parse_file (FILE* my_obj_file, struct row_of_memory** memory) 
{
    unsigned short int header[2];
    unsigned short int i, j; //for indexing
    unsigned short int n; //length
	unsigned short int addr, content; //address and content
	struct row_of_memory** current;
//	struct row_of_memory* memory = NULL;
//	memory_pointer = &memory;
	
	if(my_obj_file == NULL) {
		fprintf(stderr, "Input file is null.\n");
		return 1;
	}
	
	current = malloc(sizeof(struct row_of_memory*));
	if(current == NULL) {
		fprintf(stderr, "Dynamic memory allocation for 'current' pointer failed.\n");
		return 1;
	}
	*current = NULL;
	
    for(i = 0; /*true*/; i = (i + 1) % 2) { /* i switch between 0 and 1*/
	
		header[i] = fgetc(my_obj_file) & 0x00ff; //overwrite all higher bits to be 0
		if(feof(my_obj_file)) break; 
		
		if(i == 1) { //a header read
			// read 2 bytes address code, then 2 bytes length
			addr = read_word(my_obj_file);
			n = read_word(my_obj_file);
			
			//figure out what endianness header it is
			if(header[0] == 0xC3 && header[1] == 0xB7) { //is symbol
				/*find the correct node to write in*/
				*current = insert_to_list(memory, addr, 0); //0 is just temporary meaningless content
				
				if(*current == NULL) {
					fprintf(stderr, "Dynamic address allocation error.\n");
					free(current); //the list that 'current' points to is freed inside insert_to_list, so no worry here
					return 1;
				}
				
				/*read in the label as a string*/
				if((*current)->label != NULL) free((*current)->label); //overwrite existing label, this is unlikely to happen
				(*current)->label = malloc((n + 1) * sizeof(char));
				for(j = 0; j < n; j++) {
					(*current)->label[j] = fgetc(my_obj_file);
				}
				(*current)->label[n] = '\0'; 
				continue; // won't execute the following code
			} 
			
			else if(header[0] == 0xCA && header[1] == 0xDE) { //is code
				//do nothing
			} else if(header[0] == 0xDA && header[1] == 0xDA) { //is data
				//do nothing
			} else { // not any of them, mistake in program?
				fprintf(stderr, "Mistake! Not any endianness header!\n");
				delete_list(*memory);
				free(current); 
				return 1;
			}
			
			//handlings for cases of DADA and CADE
			//find the correct node to write to
			content = read_word(my_obj_file);			
			*current = insert_to_list(memory, addr, content);

			if(*current == NULL) {
				fprintf(stderr, "Dynamic address allocation error (2).\n");
				free(current); //the list that 'current' points to is freed inside insert_to_list, so no worry here
				return 1;
			}
			
			//continue to write more nodes consecutively
			for(j = 1; j < n; j++) {
				content = read_word(my_obj_file);
				*current = insert_to_list(current, addr + j, content);
				if(*current == NULL) {
					fprintf(stderr, "Dynamic address allocation error (2).\n");
					free(current); //the list that 'current' points to is freed inside insert_to_list, so no worry here
					return 1;
				}
			}
			
		}
    }
	
	free(current);
	return 0 ;
}


