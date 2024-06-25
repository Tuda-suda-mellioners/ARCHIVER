#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../OF/OtherFunctions.h"
#include "../T/Tree.h"

// Function to create the coding tree using the list of nodes
tree* CreatingCodingTree(int n, tree* list[256])
{
    int i, j;
    tree* NewNode = NULL;

    // If there's only one node, create the root node
    if (n == 1)
    {
        NewNode = (tree*)malloc(sizeof(tree));
        NewNode->freq = list[0]->freq;
        NewNode->right = NULL;
        NewNode->left = NULL;
        NewNode->symbol = list[0]->symbol;
    }

    // While more than one node exists, combine the two nodes with the smallest frequencies
    while (n != 1)
    {
        // Create a new internal node
        NewNode = (tree*)malloc(sizeof(tree));
        NewNode->freq = list[n - 1]->freq + list[n - 2]->freq;
        NewNode->right = list[n - 1];
        NewNode->left = list[n - 2];

        // Insert the new node back into the list
        for (i = 0; i < n; i++)
        {
            if (NewNode->freq > list[i]->freq)
            {
                for (j = n - 1; j > i; j--)
                    list[j] = list[j - 1]; // Shift elements to the right
                list[i] = NewNode; // Insert the new node
                break;
            }
        }
        n--;
    }
    return NewNode;
}

// Recursive function to create Huffman codes for each symbol
void CreatingCodes(tree* CodingTree, Codes* codes[256])
{
    if (CodingTree->left != NULL)
    {
        strcpy(CodingTree->left->code, CodingTree->code); // Copy parent code
        strcat(CodingTree->left->code, "0"); // Append '0' for left child
        CreatingCodes(CodingTree->left, codes); // Recur for left child
    }
    if (CodingTree->right != NULL)
    {
        strcpy(CodingTree->right->code, CodingTree->code); // Copy parent code
        strcat(CodingTree->right->code, "1"); // Append '1' for right child
        CreatingCodes(CodingTree->right, codes); // Recur for right child
    }
    else
    {
        strcpy(codes[CodingTree->symbol]->SymbolCode, CodingTree->code); // Assign code to symbol
        codes[CodingTree->symbol]->c = CodingTree->symbol; // Assign symbol to code structure
    }
}

// Function to create a string representation of the coding tree
void CreatingCodingTreeString(unsigned char* CodingTreeString, tree* CodingTree)
{
    int length;
    if (CodingTree->left != NULL)
    {
        length = strlen(CodingTreeString);
        CodingTreeString[length] = '0'; // Append '0' for left child
        CodingTreeString[length + 1] = '\0';
        CreatingCodingTreeString(CodingTreeString, CodingTree->left); // Recur for left child
    }
    if (CodingTree->right != NULL)
    {
        //length = strlen(CodingTreeString);
        //CodingTreeString[length] = '0';
        //CodingTreeString[length + 1] = '\0';
        CreatingCodingTreeString(CodingTreeString, CodingTree->right); // Recur for right child
    }
    else
    {
        length = strlen(CodingTreeString);
        CodingTreeString[length] = '1'; // Append '1' for leaf node
        /*CodingTreeString[length + 1] = CodingTree->symbol;
        CodingTreeString[length + 2] = '\0';*/
        CodingTreeString[length + 1] = '\0';
        strcat(CodingTreeString, BinarySymbolString(CodingTree->symbol)); // Append binary representation of symbol
        length = strlen(CodingTreeString);
        CodingTreeString[length + 1] = '\0';
    }
}
