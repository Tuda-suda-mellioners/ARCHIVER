#define _CRT_SECURE_NO_WARNINGS
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include "../LZ77_Code/Library.h"

// defining constants used in the LZ77 compression algorithm
#define HASH_SIZE 65535 // hash table size
#define MAX_OFFSET 65535 // maximum offset for backward references
#define MAX_LEN 127 // maximum match length
#define MAX_LIT 127 // maximum length of literals (uncompressed data)

// function to increase the size of the output buffer to accommodate additional bytes
void AdjustOutputBufferSize(LZ77Context* ctx, int additionalBytesNeeded) {
    int requiredSize = ctx->outputIndex + additionalBytesNeeded;
    if (requiredSize > ctx->outputLength) {
        int newLength = requiredSize * 2; // double the required size for future needs
        char* newOutput = (char*)realloc(ctx->output, newLength);
        if (!newOutput) {
            // if memory reallocation fails, clear and set the output buffer to NULL
            fprintf(stderr, "Failed to reallocate memory\n");
            free(ctx->output);
            ctx->output = NULL;
            ctx->outputLength = 0;
            return;
        }
        ctx->output = newOutput; // update the output buffer pointer
        ctx->outputLength = newLength; // update the output buffer length
    }
}

// function to update the hash table with the current index in the input buffer
void UpdateHash(LZ77Context* ctx, int idx)
{
    if (idx + 2 < ctx->inputLength)
    {
        // calculate the hash value using three consecutive bytes from the input buffer
        ctx->hash[((ctx->input[idx] & 0xFF) << 8) | ((ctx->input[idx + 1] & 0xFF) ^ (ctx->input[idx + 2] & 0xFF))] = idx;
    }
}

// function to find matches in the input buffer
int FindMatch(LZ77Context* ctx, int idx, int* off)
{
    int len = 1; // initialize the match length
    // calculate the offset based on the hash table
    *off = idx - ctx->hash[((ctx->input[idx] & 0xFF) << 8) | ((ctx->input[idx + 1] & 0xFF) ^ (ctx->input[idx + 2] & 0xFF))];
    // check if the offset is valid and the current position matches the position at the offset
    if (*off > 0 && *off < MAX_OFFSET && ctx->input[idx - *off] == ctx->input[idx])
    {
        // find the longest match within the valid range
        while (idx + len < ctx->inputLength && ctx->input[idx - *off + len] == ctx->input[idx + len] && len < MAX_LEN)
        {
            len++;
        }
    }
    return len; // return the match length
}

// function to write a block of literals (uncompressed data) to the output buffer
void WriteLiteral(LZ77Context* ctx, int idx)
{
    int max = ctx->literalCount > MAX_LIT ? MAX_LIT : ctx->literalCount; // determine the maximum number of literals to write
    ctx->literalCount -= max; // reduce the number of literals (uncompressed data) by the number of literals written
    if (ctx->output)
    {
        // write the number of literals (uncompressed data) and the actual characters to the output buffer
        ctx->output[ctx->outputIndex++] = max;
        memcpy(ctx->output + ctx->outputIndex, ctx->input + idx - ctx->literalCount - max, max);
        ctx->outputIndex += max;
    }
    else
    {
        // if the output buffer is NULL, increase the output index
        ctx->outputIndex += 1 + max;
    }
}

// function to write a compressed block to the output buffer
void WriteCompressedBlock(LZ77Context* ctx, int len, int off)
{
    if (ctx->output)
    {
        // write the length and offset of the compressed block to the output buffer
        ctx->output[ctx->outputIndex++] = 0x80 | len;
        ctx->output[ctx->outputIndex++] = off >> 8;
        ctx->output[ctx->outputIndex++] = off & 0xFF;
    }
    else
    {
        // if the output buffer is NULL, increase the output index
        ctx->outputIndex += 3;
    }
}

// LZ77 encoding function that compresses the input buffer into the output buffer
int LZ77Encode(LZ77Context* ctx) {
    int idx = 0, len, off; // initialize the current index, match length, and offset
    ctx->outputIndex = 0; // initialize the output index
    ctx->literalCount = 0; // initialize the number of literals (uncompressed data)

    while (idx <= ctx->inputLength) {
        if (idx + 2 < ctx->inputLength) {
            len = FindMatch(ctx, idx, &off); // find the longest match
        }
        else {
            len = 1; // if the end of the input buffer is near, treat it as a single literal
        }

        if (len > 3 || idx == ctx->inputLength) {
            // write all pending characters if a match longer than 3 is found or the end of the input buffer is reached
            while (ctx->literalCount) {
                AdjustOutputBufferSize(ctx, 1 + ctx->literalCount); // ensure enough space in the output buffer
                if (!ctx->output) return -1; // return an error if buffer allocation fails
                WriteLiteral(ctx, idx); // write literals
            }

            if (len > 3) {
                // write a compressed block for a match longer than 3
                AdjustOutputBufferSize(ctx, 3); // ensure enough space in the output buffer
                if (!ctx->output) return -1; // return an error if buffer allocation fails
                WriteCompressedBlock(ctx, len, off); // write the compressed block
                while (len--) {
                    UpdateHash(ctx, idx++); // update the hash table
                }
            }
            else {
                idx++; // move to the next byte
            }
        }
        else {
            idx++; // move to the next byte
            ctx->literalCount++; // increment the number of literals (uncompressed data)
        }
    }

    return ctx->outputIndex; // возвращаем общую длину сжатого выходного буфера
}

// LZ77 decoding function that extracts the input buffer into the output buffer
int LZ77Decode(LZ77Context* ctx) {
    int inIdx = 0, len, offset; // initializing indices and variables for length and offset
    ctx->outputIndex = 0; // initializing the output index
    ctx->outputLength = ctx->inputLength; // assuming the output buffer length is at least the input length

    while (inIdx < ctx->inputLength) {
        len = ctx->input[inIdx++] & 0xFF; // reading the length byte

        if (len & 0x80) { // if the length byte indicates a compressed block
            len &= 0x7F; // extracting the actual length
            offset = ((ctx->input[inIdx] & 0xFF) << 8); // reading the high byte of the offset
            inIdx++;
            offset |= (ctx->input[inIdx] & 0xFF); // reading the low byte of the offset
            inIdx++;

            AdjustOutputBufferSize(ctx, ctx->outputIndex + len); // ensuring enough space in the output buffer
            if (!ctx->output) return -1; // returning an error if buffer allocation failed

            // copying the match from the output buffer to the current position
            for (int i = 0; i < len; ++i) {
                ctx->output[ctx->outputIndex] = ctx->output[ctx->outputIndex - offset];
                ctx->outputIndex++;
            }
        }
        else { // if the length byte indicates a literal block (uncompressed data)
            AdjustOutputBufferSize(ctx, ctx->outputIndex + len); // ensuring enough space in the output buffer
            if (!ctx->output) return -1; // returning an error if buffer allocation failed

            // copying literals (uncompressed data) from the input buffer to the output buffer
            for (int i = 0; i < len; ++i) {
                ctx->output[ctx->outputIndex++] = ctx->input[inIdx++];
            }
        }
    }

    return ctx->outputIndex; // returning the total length of the extracted output buffer
}

// function to load a file into a dynamically allocated buffer
long LoadFile(const char* fileName, char** buffer)
{
    FILE* file = fopen(fileName, "rb"); // opening the file in binary read mode
    if (!file)
    {
        // displaying an error message if the file cannot be opened
        fprintf(stderr, "Cannot open file %s: %s\n", fileName, strerror(errno));
        return -1;
    }

    fseek(file, 0, SEEK_END); // moving to the end of the file to determine its length
    long length = ftell(file); // getting the file length
    fseek(file, 0, SEEK_SET); // returning to the beginning of the file

    *buffer = (char*)malloc(length); // allocating memory for the buffer
    if (!*buffer)
    {
        // displaying an error message if memory allocation fails
        fprintf(stderr, "Failed to allocate memory\n");
        fclose(file);
        return -1;
    }

    size_t bytesRead = fread(*buffer, 1, length, file); // reading the file into the buffer
    if (bytesRead != length)
    {
        // displaying an error message if the file cannot be read completely
        fprintf(stderr, "Failed to read file %s\n", fileName);
        free(*buffer);
        fclose(file);
        return -1;
    }

    fclose(file); // closing the file
    return length; // returning the file length
}

// function to save a buffer to a file
int SaveFile(const char* fileName, const char* buffer, int count)
{
    FILE* file = fopen(fileName, "wb"); // opening the file in binary write mode
    if (!file)
    {
        // displaying an error message if the file cannot be opened for writing
        fprintf(stderr, "Cannot open file %s for writing: %s\n", fileName, strerror(errno));
        return -1;
    }

    fwrite(buffer, 1, count, file); // writing the buffer to the file
    fclose(file); // closing the file
    return count; // returning the number of bytes written
}

// function to compress a file
int CompressFile(const char* inputFileName, const char* outputFileName)
{
    LZ77Context ctx; // creating a context for compression
    ctx.inputLength = LoadFile(inputFileName, &ctx.input); // loading the input file into the context
    if (ctx.inputLength < 0)
        return -1;

    clock_t start, end;
    double cpu_time_used, speed;

    start = clock();

    ctx.hash = (int*)calloc(HASH_SIZE, sizeof(int)); // allocating memory for the hash table
    if (!ctx.hash)
    {
        free(ctx.input);
        fprintf(stderr, "Failed to allocate memory for the hash\n");
        return -1;
    }

    ctx.output = NULL; // initializing the output buffer pointer to NULL
    ctx.outputLength = ctx.inputLength; // setting the initial output buffer length to the input length

    ctx.output = (char*)malloc(ctx.outputLength); // allocating memory for the output buffer
    if (!ctx.output)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        free(ctx.input);
        free(ctx.hash);
        return -1;
    }

    int outputLength = LZ77Encode(&ctx); // compressing the input buffer into the output buffer
    SaveFile(outputFileName, ctx.output, outputLength); // saving the compressed data to the output file

    end = clock(); // останавливаем таймер
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    speed = (ctx.inputLength / (1024.0 * 1024.0)) / cpu_time_used;

    printf("Original size: %d\n", ctx.inputLength);
    printf("Size after compression: %d\n", outputLength);
    printf("Compression ratio: %.2f\n", (double)ctx.inputLength / outputLength);
    printf("Speed: %.2f МБ/с\n", speed);

    // freeing the allocated memory
    free(ctx.input);
    free(ctx.output);
    free(ctx.hash);
    return 0;
}

// function to decompress a file
int DecompressFile(const char* inputFileName, const char* outputFileName)
{
    LZ77Context ctx; // creating a context for decompression

    ctx.inputLength = LoadFile(inputFileName, &ctx.input); // loading the input file into the context
    if (ctx.inputLength < 0)
    {
        fprintf(stderr, "Failed to load the input file\n");
        return -1;
    }

    clock_t start, end;
    double cpu_time_used, speed;

    start = clock();

    ctx.output = (char*)malloc(ctx.inputLength); // allocating memory for the output buffer
    if (!ctx.output)
    {
        fprintf(stderr, "Failed to allocate memory for the output buffer\n");
        free(ctx.input);
        return -1;
    }

    ctx.outputLength = LZ77Decode(&ctx); // extracting the input buffer into the output buffer

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC; 
    speed = (ctx.inputLength / (1024.0 * 1024.0)) / cpu_time_used;

    SaveFile(outputFileName, ctx.output, ctx.outputLength); // saving the extracted data to the output file

    printf("Compressed file size: %d\n", ctx.inputLength);
    printf("Size after decompression: %d\n", ctx.outputLength);
    printf("Speed: %.2f МБ/с\n", speed);

    // freeing the allocated memory
    free(ctx.input);
    free(ctx.output);
    return 0;
}

// function to print the program usage instructions
void PrintUsage(const char* programName)
{
    // printing the program usage instructions with command-line syntax
    printf("Usage for compression: %s -encode <input file> <output file>\n", programName);
    printf("Usage for decompression: %s -decode <input file> <output file>\n", programName);
}