//Andreea Rosu
//Informatica
//Anul III, 2025
#include"utils.h"
#include"lexer.h"
#include"parser.h"

#include<stdio.h>

int main() {
	char* inbuf = loadFile("test/1.q");
	puts(inbuf);
	printf("\n");
	printf("Atomii sunt: \n");
	tokenize(inbuf);
	showTokens();
	return 0;
}