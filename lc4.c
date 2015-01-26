#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lc4_memory.h"
#include "lc4_loader.h"
#include "lc4_disassembler.h"

/* program to mimic LC4's program & data memory */

int main () {

	/**
	 * main() holds the linked list &
	 * only calls functions in other files 
	 */
	struct row_of_memory* memory = NULL;
	FILE* rfile = NULL;
	char* file_name = NULL;
	
	file_name = malloc(100* sizeof(char));
	if(file_name == NULL) {
		fprintf(stderr, "file_name memory allocation error.\n");
		return 1;
	}
	
	/* step 1: create head pointer to linked list: memory 	*/
	//Unnecessary
	/* step 2: call function: get_file() in lc4_loader.c 	*/
	/*   TODO: call function & check for errorsl		*/
	file_name = get_file_name(file_name, 100);

	if(file_name == NULL) {
		fprintf(stderr, "file_name input error.\n");
		return 1;
	}
	rfile = get_file(file_name);
	if(rfile == NULL) {
		return 1;
	}

	/* step 3: call function: parse_file() in lc4_loader.c 	*/
	/*   TODO: call function & check for errors		*/
	if(parse_file(rfile, &memory) != 0) {
		fclose(rfile);
		return 1;
	}
	
	if(fclose(rfile) != 0) {
		fprintf(stderr, "read file close fail!\n");
		return 1;
	}
	/* step 4: call function: reverse_assemble() in lc4_disassembler.c */
	/*   TODO: call function & check for errors		*/
	if(reverse_assemble (&memory) != 0) {
		return 1;
	}
	/* step 5: call function: print_list() in lc4_memory.c 	*/
	/*   TODO: call function 				*/
	
	print_list(memory);
	if(write_assembly(memory, file_name) != 0) {
		return 1;
	}
	printf("\n===Reverse assembly done. The file %s has been created!===\n", file_name);
	
	/* step 6: call function: delete_list() in lc4_memory.c */
	/*   TODO: call function & check for errors		*/
	memory = delete_list(memory);
	free(file_name);
	
	/* only return 0 if everything works properly */
	return 0 ;
}
