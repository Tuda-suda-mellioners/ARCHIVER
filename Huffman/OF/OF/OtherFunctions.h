#pragma once
typedef struct node
{
	unsigned char symbol;
	int freq;
	char code[256];
	struct	node* left;
	struct  node* right;
}tree;
typedef struct codes_s
{
	unsigned char c;
	char SymbolCode[256];
}Codes;
unsigned char* BinarySymbolString(unsigned char symbol);
unsigned char  ASCII_BinaryCodeToChar(unsigned char a[9]);
unsigned char PackingByte(unsigned char eight[9]);
void ShellSort(int N, tree* list[256]);
int CalculatingFrequences(FILE* from, tree* list[256], Codes* codes[256]);
void Coding(FILE* before, FILE* after, Codes* codes[256]);
void Info(char* archive);
int RightFileFormat(char* file);