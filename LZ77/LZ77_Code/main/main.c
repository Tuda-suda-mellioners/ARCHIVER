#define _CRT_SECURE_NO_WARNINGS
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include "../LZ77_Code/Library.h"

int main(int argc, char** argv)
{
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    if (argc == 4 && strcmp(argv[1], "-encode") == 0)
    {
        // compression mode if the -encode flag is specified
        char* inputFile = argv[2];
        char* outputFile = argv[3];
        return CompressFile(inputFile, outputFile) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    else if (argc == 4 && strcmp(argv[1], "-decode") == 0)
    {
        // decompression mode if the -decode flag is specified
        char* inputFile = argv[2];
        char* outputFile = argv[3];
        return DecompressFile(inputFile, outputFile) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    else
    {
        // if the arguments are incorrect, display usage instructions
        PrintUsage(argv[0]);
        return EXIT_FAILURE;
    }
}