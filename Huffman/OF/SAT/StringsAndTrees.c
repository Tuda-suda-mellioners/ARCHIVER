#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../OF/OtherFunctions.h"
#include "../SAT/StringsAndTrees.h"

// Writes the coding tree string to the output file in binary format
void RecordingTree(FILE* after, unsigned char* CodingTreeString)
{
    int i = 0, sum = 0;
    unsigned char eight[9];
    unsigned char byte = 0;
    int length = strlen(CodingTreeString);

    // Iterate through the coding tree string
    for (i = 0; i < length; i++)
    {
        eight[sum] = CodingTreeString[i]; // Add character to buffer
        sum++;

        if (sum == 8) // If buffer is full
        {
            sum = 0;
            eight[8] = '\0'; // Null-terminate the buffer
            byte = PackingByte(eight); // Pack the bits into a byte
            fwrite(&byte, 1, 1, after); // Write the byte to the file
            byte = 0;
            memset(eight, 0, 9); // Clear the buffer
        }
    }
}

// Reconstructs the coding tree string from the given binary data
void TreeReconstruction1(int n, unsigned char result[65536], unsigned char* CodingTreeString)
{
    int i = 0, j = 0, k = 0;
    unsigned char eight[9];

    // Iterate through the coding tree string
    for (i = 0; i < n; i++)
    {
        result[j] = CodingTreeString[i]; // Add character to result
        j++;

        if (CodingTreeString[i] == '1') // If current character is '1'
        {
            i++;
            for (k = 0; k < 8; k++) // Read the next 8 bits
            {
                eight[k] = CodingTreeString[i];
                i++;
            }
            i--;
            eight[8] = '\0'; // Null-terminate the buffer
            result[j] = ASCII_BinaryCodeToChar(eight); // Convert binary to char and add to result
            j++;
            memset(eight, 0, 9); // Clear the buffer
        }
    }
    result[j] = '\0'; // Null-terminate the result string
}

// Recursively reconstructs the tree from the tree string
int TreeReconstruction2(tree* new, int step, unsigned char TreeString[65536])
{
    if (TreeString[step] == '0') // If current character is '0'
    {
        // Create left leaf
        tree* newLeaf1 = (tree*)malloc(sizeof(tree));
        newLeaf1->left = newLeaf1->right = NULL;
        new->left = newLeaf1;

        // Create right leaf
        tree* newLeaf2 = (tree*)malloc(sizeof(tree));
        newLeaf2->left = newLeaf2->right = NULL;
        new->right = newLeaf2;

        // Recursively reconstruct left and right subtrees
        step = TreeReconstruction2(new->left, step + 1, TreeString);
        step = TreeReconstruction2(new->right, step + 1, TreeString);
        return step;
    }
    else // If current character is '1'
    {
        step++;
        new->symbol = TreeString[step]; // Set the symbol of the node
        new->left = NULL;
        new->right = NULL;
        return step;
    }
}
