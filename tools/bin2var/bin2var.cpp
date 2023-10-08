#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum class VarType {
    UnsignedChar,
    UnsignedShort,
    Char,
    Short,
};

int main(int argc, char* argv[])
{
    VarType type = VarType::Char;
    char* path = nullptr;
    for (int i = 1; i < argc; i++) {
        if (0 == strncmp(argv[i], "-b", 2)) {
            type = VarType::UnsignedChar;
        } else if (0 == strncmp(argv[i], "-w", 2)) {
            type = VarType::UnsignedShort;
        } else if (0 == strncmp(argv[i], "-c", 2)) {
            type = VarType::Char;
        } else if (0 == strncmp(argv[i], "-s", 2)) {
            type = VarType::Short;
        } else {
            path = argv[i];
        }
    }
    if (!path) {
        fprintf(stderr, "bin2var [-b|-w|-c|-s] /path/to/binary.rom\n");
        return -1;
    }
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "file open error\n");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int size = (int)ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char varName[1024];
    char* cp = strrchr(path, '/');
    if (cp) {
        strcpy(varName, cp + 1);
    } else {
        cp = strrchr(path, '\\');
        if (cp) {
            strcpy(varName, cp + 1);
        } else {
            strcpy(varName, path);
        }
    }
    cp = strchr(varName, '.');
    if (cp) *cp = 0;
    switch (type) {
        case VarType::UnsignedChar: printf("const unsigned char rom_%s[%d] = {\n", varName, size); break;
        case VarType::UnsignedShort: printf("const unsigned short rom_%s[%d] = {\n", varName, size / 2); break;
        case VarType::Char: printf("const char rom_%s[%d] = {\n", varName, size); break;
        case VarType::Short: printf("const short rom_%s[%d] = {\n", varName, size / 2); break;
    }
    
    bool firstLine = true;
    while (1) {
        unsigned char buf[16];
        int readSize = (int)fread(buf, 1, sizeof(buf), fp);
        if (readSize < 1) {
            printf("\n");
            break;
        }
        if (firstLine) {
            firstLine = false;
        } else {
            printf(",\n");
        }
        printf("    ");
        switch (type) {
            case VarType::UnsignedChar:
            case VarType::Char: {
                for (int i = 0; i < readSize; i++) {
                    if (i) {
                        printf(", 0x%02X", buf[i]);
                    } else {
                        printf("0x%02X", buf[i]);
                    }
                }
                break;
            }
            case VarType::UnsignedShort:
            case VarType::Short: {
                for (int i = 0; i < readSize; i += 2) {
                    unsigned short s;
                    memcpy(&s, &buf[i], 2);
                    if (i) {
                        printf(", 0x%04X", s);
                    } else {
                        printf("0x%04X", s);
                    }
                }
                break;
            }
        }
    }
    printf("};\n");
    fclose(fp);
    return 0;
}
