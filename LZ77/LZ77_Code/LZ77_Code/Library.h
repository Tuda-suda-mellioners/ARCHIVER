#pragma once
// structure for storing the LZ77 compression/decompression context
typedef struct
{
    int* hash; // hash table for quick match searching
    char* input; // input buffer containing data for compression/decompression
    char* output; // output buffer for storing compressed/decompressed data
    int inputLength; // length of the input buffer
    int outputLength; // length of the output buffer
    int outputIndex; // current position in the output buffer
    int literalCount; // number of literals (uncompressed data) to write
} LZ77Context;

// function prototypes for the main compression and decompression procedures
int CompressFile(const char* inputFileName, const char* outputFileName);
int DecompressFile(const char* inputFileName, const char* outputFileName);

int FindMatch(LZ77Context* ctx, int idx, int* off); // function to find the longest match in the input buffer
void WriteLiteral(LZ77Context* ctx, int idx); // function to write literals (uncompressed data) to the output buffer
void WriteCompressedBlock(LZ77Context* ctx, int len, int off); // function to write compressed blocks to the output buffer
void UpdateHash(LZ77Context* ctx, int idx); // function to update the hash table based on the current index in the input buffer
int LZ77Encode(LZ77Context* ctx); // function to compress data using the LZ77 algorithm
int LZ77Decode(LZ77Context* ctx); // function to decompress data using the LZ77 algorithm
void AdjustOutputBufferSize(LZ77Context* ctx, int additionalBytesNeeded); // function to increase the size of the output buffer if necessary
long LoadFile(const char* fileName, char** buffer); // function to load the file contents into a dynamically allocated buffer
int SaveFile(const char* fileName, const char* buffer, int count); // function to save the data buffer to a file
void PrintUsage(const char* programName); // function to print the program usage instructions to the console