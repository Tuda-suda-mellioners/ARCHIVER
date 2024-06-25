#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h> 
#include "../OF/OtherFunctions.h"
#include "../T/Tree.h"
#include "../SAT/StringsAndTrees.h"

void Dearchiving(char* before, char* after)
{
    FILE* Archive, * NewFile;
    // Check if the source file for decompressing exists and can be opened
    if (!fopen(before, "rb"))
    {
        printf("Unfortunately, file for decompressing can't be opened! Check if it exists and locates in the folders 'Debug' and 'arc2'.");
        getchar();
        return;
    }
    // Check if the target file for decompressed data can be opened
    if (!fopen(after, "rb"))
    {
        printf("Unfortunately, file in which you are going to decompress previous file can't be opened! Check if it exists and locates in the folder 'Debug' and 'arc2'.");
        getchar();
        return;
    }
    Archive = fopen(before, "rb");
    NewFile = fopen(after, "wb");
    printf("Decompressing...Please, wait...\n");

    int length;
    unsigned long long int counter = 0;
    unsigned char byte;
    char rest;
    time_t Time;

    // Count the total number of bytes in the archive
    while (fread(&byte, 1, 1, Archive))
        counter++;

    // Check if the file has content to decompress
    if (counter == 0)
    {
        printf("File exists but has no information (nothing to dearchive).");
        getchar();
        return;
    }

    rest = byte;
    rewind(Archive);
    fread(&Time, sizeof(time_t), 1, Archive);
    fread(&length, 4, 1, Archive);

    counter = counter - length - 1 - sizeof(time_t);
    int b, i, j, k;
    unsigned char C[65536];
    k = 0;

    // Read the tree structure from the archive
    for (i = 0; i < length; i++)
    {
        fread(&byte, 1, 1, Archive);
        for (j = 0; j < 8; j++)
        {
            b = (byte & 1);
            byte >>= 1;
            C[k] = (unsigned char)(b + '0');
            k++;
        }
    }
    C[k] = '\0';

    unsigned char TreeString[65536];
    TreeReconstruction1(length * 8, TreeString, C);

    tree* new = NULL, * head;
    new = (tree*)malloc(sizeof(tree));
    head = new;
    int step = 0;
    step = TreeReconstruction2(new, step, TreeString);
    head = new;
    tree* root = head;
    char end = 8;

    // Special case if the root has no children
    if (root->right == NULL || root->left == NULL)
    {
        fwrite(&root->symbol, 1, 1, NewFile);
        fclose(Archive);
        fclose(NewFile);
        return;
    }

    // Decompress the data based on the tree structure
    for (k = 0; k < counter - 5; k++)
    {
        fread(&byte, 1, 1, Archive);
        for (i = 0; i < end; i++)
        {
            if ((byte & (1 << i)) == 0)
            {
                root = root->left;
                if (root->left == NULL && root->left == NULL)
                {
                    fwrite(&root->symbol, 1, 1, NewFile);
                    root = head;
                }
            }
            else
            {
                root = root->right;
                if (root->left == NULL && root->left == NULL)
                {
                    fwrite(&root->symbol, 1, 1, NewFile);
                    root = head;
                }
            }
        }
    }
    fread(&byte, 1, 1, Archive);
    for (i = 0; i < 8 - rest; i++)
    {
        if ((byte & (1 << i)) == 0)
        {
            root = root->left;
            if (root->left == NULL && root->left == NULL)
            {
                fwrite(&root->symbol, 1, 1, NewFile);
                root = head;
            }
        }
        else
        {
            root = root->right;
            if (root->left == NULL && root->left == NULL)
            {
                fwrite(&root->symbol, 1, 1, NewFile);
                root = head;
            }
        }
    }
    printf("\nDecompressed successfully!\n");
    fclose(Archive);
    fclose(NewFile);
}

void Archiving(char* before, char* after)
{
    FILE* FileToArc, * Archive;
    // Check if the source file for compressing exists and can be opened
    if (!fopen(before, "rb"))
    {
        printf("Unfortunately, file for compressing can't be opened! Check if it exists and locates in the folder 'Debug' and 'arc2'.");
        return;
    }
    // Check if the target archive file can be opened
    if (!fopen(after, "rb"))
    {
        printf("Unfortunately, file in which you are going to compress previous file can't be opened! Check if it exists and locates in the folder 'Debug' and 'arc2'.");
        return;
    }
    FileToArc = fopen(before, "rb");
    Archive = fopen(after, "wb");
    printf("Compressing...Please, wait...\n");

    tree* list[256];
    Codes* codes[256];
    int n, i;
    int length = 0;
    char rest = 0;
    unsigned char* CodingTreeString;
    unsigned char c;
    time_t Time = time(NULL);
    fwrite(&Time, sizeof(time_t), 1, Archive);
    tree* CodingTree;

    // Calculate frequencies of symbols and create the Huffman tree
    n = CalculatingFrequences(FileToArc, list, codes);
    if (n == 0)
    {
        printf("File exists but has no information (nothing to archive).");
        getchar;
        return;
    }

    CodingTreeString = (unsigned char*)calloc(n * 256, sizeof(unsigned char));
    CodingTree = CreatingCodingTree(n, list);
    CreatingCodingTreeString(CodingTreeString, CodingTree);

    // Generate codes for the symbols
    if (n != 1)
    {
        strcpy(CodingTree->left->code, "0");
        strcpy(CodingTree->right->code, "1");
        CreatingCodes(CodingTree->left, codes);
        CreatingCodes(CodingTree->right, codes);
    }
    else
    {
        strcpy(codes[CodingTree->symbol]->SymbolCode, "0");
    }

    length = strlen(CodingTreeString);
    rest = (char)length % 8;
    for (i = length; i < length + (8 - rest); i++)
        CodingTreeString[i] = '0';

    length = length + (8 - rest);
    length = (int)length / 8;
    fwrite(&length, 4, 1, Archive);
    RecordingTree(Archive, CodingTreeString);

    // Encode the file content and write to the archive
    Coding(FileToArc, Archive, codes);
    printf("\nCompressed successfully!\n");
    fclose(Archive);
    fclose(FileToArc);
}

int main(int argc, char* argv[])
{
    if (argv[1] == NULL)
    {
        printf(" Hello! Are you going to use 'arc2.exe'?\n If so, please enter the name of files that you want to compress or decompress.\n If you need more information or help, run the program again, enter 'h' and press 'Enter'.\n Good luck!");
        getchar();
        return 0;
    }
    if (strcmp(argv[1], "h") == 0)
    {
        printf("\n\tINFORMATION ABOUT THE PROGRAM:\n");
        printf(" This program can compress and decompress files. \n (It is based on Huffman algorithm.)\n Here you can read how to use it:\n");
        printf(" 0. First of all, add files that you want to compress or decompress in the folder 'Debug' and 'arc2'.\n(File which you want to compress in or decompress from should also be in this folder and have format '.huff')\n");
        printf(" 1. If you want to compress file, enter the file name and name of a file-archive. \nFor example, 'arc2.exe c book.txt arc.huff'.\n");
        printf(" 2. If you want to decompress file, enter the file-archive name and a name of file-result. \nFor example, 'arc2.exe d arc.huff book2.txt'.\n");
        printf(" 3. If you want to see information about file-archive, enter 'i' before file-archive name and press 'Enter'. \nFor example, 'arc2.exe i arc.huff'.\n");
        printf(" 4. If you want to compress and then decompress file, enter 'c d' before the file name, a file-archive name, and a file-result name. \nFor example, 'arc2.exe c d book.txt arc.huff book2.txt'.\n");
        printf(" Good luck! Have a good day!");
        getchar();
        return 0;
    }
	if (strcmp(argv[1], "c") == 0 && argv[2] != NULL && argv[3] != NULL && RightFileFormat(argv[3]))
		Archiving(argv[2], argv[3]);
	if (strcmp(argv[1], "i") == 0 && argv[2] != NULL && RightFileFormat(argv[2]))
		Info(argv[2]);
	if (strcmp(argv[1], "d") == 0 && argv[2] != NULL && argv[3] != NULL && RightFileFormat(argv[2]))
		Dearchiving(argv[2], argv[3]);
	if (strcmp(argv[1], "c") == 0 && strcmp(argv[2], "d") == 0 && argv[3] != NULL && argv[4] != NULL && argv[5] != NULL)
	{
		Archiving(argv[3], argv[4]);
		Dearchiving(argv[4], argv[5]);
	}

	if (strcmp(argv[1], "c") == 0 && strcmp(argv[2], "d") == 0 && argv[3] != NULL && argv[4] != NULL && argv[5] == NULL)
	{
		printf(" You forgot to enter last file in which you want to decompress your file! Please, try again.\n If you need help, run program againg and enter 'h'.");
		getchar();
		return 0;
	}
	if (strcmp(argv[1], "c") == 0 && strcmp(argv[2], "d") == 0 && argv[3] != NULL && argv[4] == NULL)
	{
		printf(" You forgot to enter file in which you want to compress first entered file! Please, try again.\n If you need help, run program againg and enter 'h'.");
		getchar();
		return 0;
	}
	if (strcmp(argv[1], "c") == 0 && argv[2] != NULL && argv[3] != NULL && !RightFileFormat(argv[3]))
	{
		printf(" Wrong format of second file (it should be'.huff')! Please, try again.\n If you need help, run program againg and enter 'h'.");
		getchar();
		return 0;
	}
	if (strcmp(argv[1], "d") == 0 && argv[2] != NULL && argv[3] != NULL && !RightFileFormat(argv[2]))
	{
		printf(" Wrong format of first file (it should be'.huff')! Please, try again.\n If you need help, run program againg and enter 'h'.");
		getchar();
		return 0;
	}
	if (strcmp(argv[1], "i") == 0 && RightFileFormat(argv[2]) == 0)
	{
		printf(" Wrong format of file (it should be'.huff')! Please, try again.\n If you need help, run program againg and enter 'h'.");
		getchar();
		return 0;
	}

	if (strcmp(argv[1], "d") == 0 && argv[2] != NULL && argv[3] != NULL && !RightFileFormat(argv[2]))
		Dearchiving(argv[2], argv[3]);
	if (strcmp(argv[1], "c") == 0 && strcmp(argv[2], "d") == 0 && argv[3] == NULL)
	{
		printf(" You forgot to enter files! Please, try again.\n If you need help, run program againg and enter 'h'.");
		getchar();
		return 0;
	}
	if (strcmp(argv[1], "c") == 0 && strcmp(argv[2], "d") == 0 && argv[3] == NULL)
	{
		printf(" You forgot to enter file-archive and file-result! Please, try again.\n If you need help, run program againg and enter 'h'.");
		getchar();
		return 0;
	}
	if (strcmp(argv[1], "c") == 0 && argv[2] != NULL && argv[3] == NULL)
	{
		printf(" You forgot to enter file-archive! Please, try again.\n If you need help, run program againg and enter 'h'.");
		getchar();
		return 0;
	}
	if (strcmp(argv[1], "d") == 0 && argv[2] != NULL && argv[3] == NULL)
	{
		printf(" You forgot to enter file-result! Please, try again.\n If you need help, run program againg and enter 'h'.");
		getchar();
		return 0;
	}
	if (strcmp(argv[1], "c") != 0 && strcmp(argv[1], "d") != 0 && strcmp(argv[1], "i") != 0 && strcmp(argv[1], "h") != 0)
	{
		printf(" You forgot to write what to do or operation is unknown! The program can only:\n");
		printf("\t'c' - compress file (example: 'arc2.exe c book.txt arc.huff');\n");
		printf("\t'i' - information about file (only for file-archives, example: 'arc2.exe i arc.huff');\n");
		printf("\t'd' - decompress file (example: 'arc2.exe d arc.huff book.txt')\n");
		printf("\t'c d' - compress and then decompress file (example: 'arc2.exe c book.txt arc.huff book2.txt')\n");
		printf(" Or, maybe, you entered a name of a file without any operations.\n");
		printf(" Please, try again.\n If you need help, run program againg and enter 'h'.");
		getchar();
		return 0;
	}
	return 0;
}
