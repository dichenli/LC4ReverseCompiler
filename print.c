#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short int extract_bits(unsigned short int num, unsigned short int i, unsigned short int j) {
	/*i, j: start and end bits. The number from start int to end int will be */
	
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
	/*i, j: start and end bits. The number from start int to end int will be */
	
	unsigned short int size = sizeof(unsigned short int) * 8; // should be 16 for most systems
	if(i < 0 || j < 0 || i >= size || j >= size || i > j) {
		fprintf(stderr, "Wrong bits positions!\n");
		return 65535; //invalid, either bits out of range or i > j
	}
	
	num = num << (size - 1 - j);
	num = num >> (size - 1 - j + i); //bitwise shift will fill 1 in the beginning
	return num;
}

void add(int* a) {
	a = malloc(sizeof(int));
	*a = 3;
	*a += 1;
}

int main() {

	int** pp;
	pp = malloc(sizeof(int*));
	*pp = malloc(sizeof(int));
	**pp = 1;
	add(*pp);
	int i;
	char file_name[30] = {'o', 'b', 'j','.','o','b','j'};
	
	printf("x%02X\tx%02X\tx%02X\tx%02X\n", extract_bits(0xA234, 12, 15), extract_bits(0x1234, 8, 11), extract_bits(0x1234, 4, 7), extract_bits(0x1234, 0, 3));
	printf("x%02X\n", extract_signed_bits(0xA234, 12, 15));
	printf("%-10XEND\n", 10);
	printf("%-16s%-12s%-16s%-20s\n", "<label>", "<address>", "<contents>", "<assembly>");
	printf("%d\n", **pp);
	printf(".1  .1\n");
	free(*pp);
	free(pp);
	
	printf("%s\t", file_name);
	for(i = strlen(file_name) - 1; file_name[i] != '.' && i >=0; i--) {
		//do nothing
	}
	if(i > 0) { // replace the last '.' char by '\0', so file type extension (.obj) will be erased
		file_name[i] = '\0';
	}
	strcat(file_name, ".asm");
	printf("%s\n", file_name);
	return 0;
}


