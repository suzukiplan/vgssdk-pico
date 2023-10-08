#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lz4.h"

void* loadFile(const char* path, size_t* size)
{
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        printf("File not found: %s\n", path);
        return nullptr;
    }
    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    void* result = malloc(*size);
    if (!result) {
        puts("No memory");
        fclose(fp);
        return nullptr;
    }
    if (*size != fread(result, 1, *size, fp)) {
        printf("Read error: %s\n", path);
        fclose(fp);
        free(result);
        return nullptr;
    }
    fclose(fp);
    return result;
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        puts("vgslz4 input output");
        return 1;
    }
    size_t inSize;
    void* inData = loadFile(argv[1], &inSize);
    if (!inData) {
        puts("file open error");
        return -1;
    }
    void* outData = malloc(inSize * 2);
    int outSize = LZ4_compress_default((const char*)inData,
                                       (char*)outData,
                                       (int)inSize,
                                       (int)inSize * 2);
    FILE* fp = fopen(argv[2], "wb");
    fwrite(outData, 1, outSize, fp);
    fclose(fp);
    return 0;
}
