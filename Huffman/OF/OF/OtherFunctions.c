#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <math.h>
#include "../OF/OtherFunctions.h"

// Converts a symbol to its binary representation as a string of '0's and '1's
unsigned char* BinarySymbolString(unsigned char symbol)
{
    unsigned char result[9] = { '\0' }; // Temporary array to store binary representation
    unsigned char r[9] = { '\0' }; // Final array to store the correct order of binary digits
    int c, i = 0, length, j = 0;
    c = (int)symbol; // Convert the symbol to its integer representation

    // Convert the integer to binary
    while (c != 0)
    {
        result[i] = c % 2; // Get the least significant bit
        result[i] = result[i] + '0'; // Convert bit to character '0' or '1'
        c = c / 2; // Shift right
        i++;
    }

    length = i; // Length of the binary representation

    // Pad with leading zeros if the binary representation is less than 8 bits
    if (length < 8)
    {
        for (j = 0; j < 8 - length; j++)
            r[j] = '0';
    }

    // Reverse the result to get the correct binary string
    for (i = length - 1; i >= 0; i--)
    {
        r[j] = result[i];
        j++;
    }

    r[8] = '\0'; // Null-terminate the string
    return r; // Return the binary string
}

// Converts a binary string (ASCII) to its corresponding character
unsigned char ASCII_BinaryCodeToChar(unsigned char a[9])
{
    int i = 0, k, st, sum = 0, x = 0;
    unsigned char c;

    // Find the length of the binary string
    while (a[i] != '\0')
    {
        i++;
    }

    k = i; // Number of binary digits

    // Convert the binary string to a character
    int count = 0;
    for (i = k - 1; i >= 0; i--)
    {
        st = pow(2, i); // Calculate the power of 2 for the current bit position
        x = (a[count] - '0') * st; // Convert the binary digit to an integer
        sum += x; // Add to the sum
        count++;
    }

    c = sum; // Convert the sum to a character
    return c; // Return the character
}

// Packs an array of '0' and '1' characters into a single byte
unsigned char PackingByte(unsigned char eight[9])
{
    unsigned char byte = 0;
    int i;

    // Pack each bit into the byte
    for (i = 0; i < 8; i++)
    {
        byte = byte | ((eight[i] - '0') << i); // Shift and combine bits
    }

    return byte; // Return the packed byte
}

// Shell sort for sorting a list of tree nodes by frequency in descending order
void ShellSort(int N, tree* list[256])
{
    int i, j, k;
    tree* t;

    // Shell sort algorithm
    for (k = N / 2; k > 0; k /= 2)
    {
        for (i = k; i < N; i++)
        {
            t = list[i];
            for (j = i; j >= k; j -= k)
            {
                if (t->freq > list[j - k]->freq) // Compare frequencies
                    list[j] = list[j - k];
                else
                    break;
            }
            list[j] = t;
        }
    }
}

// Calculates the frequencies of each character in a file and sorts them
int CalculatingFrequences(FILE* from, tree* list[256], Codes* codes[256])
{
    int i, n = 0;
    unsigned char c;

    // Initialize frequency list and codes
    for (i = 0; i < 256; i++)
    {
        codes[i] = (Codes*)malloc(sizeof(Codes)); // Allocate memory for code
        list[i] = (tree*)malloc(sizeof(tree)); // Allocate memory for tree node
        list[i]->freq = 0; // Initialize frequency to 0
        list[i]->right = list[i]->left = NULL; // Initialize tree pointers
    }

    // Read the file and count frequencies
    while (fread(&c, 1, 1, from) == 1)
    {
        if (list[c]->freq == 0)
            n++; // Count unique characters
        list[c]->freq++; // Increment frequency
        list[c]->symbol = c; // Store the character
    }

    // Rewind the file for further processing
    rewind(from);

    // Sort the list based on frequencies
    ShellSort(256, list);
    return n; // Return the number of unique characters
}

// Encodes a file using the provided codes and writes the encoded data to another file
void Coding(FILE* before, FILE* after, Codes* codes[256])
{
    unsigned char byte = 0;
    unsigned char c;
    int i = 0, sum = 0;
    unsigned char eight[9];
    int counter = 0;

    // Read the file and encode the data
    while (fread(&c, 1, 1, before))
    {
        for (i = 0; i < strlen(codes[c]->SymbolCode); i++)
        {
            eight[sum] = codes[c]->SymbolCode[i]; // Add the code to the buffer
            sum++;
            if (sum == 8) // If buffer is full
            {
                sum = 0;
                eight[8] = '\0'; // Null-terminate the string
                byte = PackingByte(eight); // Pack the bits into a byte
                fwrite(&byte, 1, 1, after); // Write the byte to the file
                counter++;
                byte = 0;
                memset(eight, 0, 9); // Clear the buffer
            }
        }
    }

    // Handle any remaining bits
    for (i = sum; i < 8; i++)
        eight[i] = '0'; // Pad with zeros
    eight[8] = '\0';
    byte = PackingByte(eight); // Pack the remaining bits
    fwrite(&byte, 1, 1, after); // Write the byte to the file
    byte = 0;

    // Write the number of unused bits in the last byte
    char end;
    end = (char)(8 - sum);
    fwrite(&end, 1, 1, after);
}

// Displays information about the archive file
void Info(char* archive)
{
    FILE* Archive, * InfoFile;
    Archive = fopen(archive, "rb");
    unsigned long long int FileSize = 0;
    time_t Time = 0;
    unsigned char byte;

    // Calculate the size of the archive
    while (fread(&byte, 1, 1, Archive))
        FileSize++;
    rewind(Archive);

    // Print the size of the archive
    printf("\nSize of archive: %llu bytes.\n", FileSize);

    // Read and print the time of archiving
    fread(&Time, sizeof(time_t), 1, Archive);
    printf("Time of archiving: %s", ctime(&Time));

    fclose(Archive);
    return;
}

// Checks if the file has the correct format
int RightFileFormat(char* file)
{
    int n = strlen(file);

    // Check the file extension
    if (file[n] != 'f' && file[n - 1] != 'f' && file[n - 2] != 'u' && file[n - 3] != 'h')
        return 0; // Return false if format is incorrect
    return 1; // Return true if format is correct
}
