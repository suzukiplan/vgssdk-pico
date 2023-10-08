/**
 * VGS Fixed NOTE data to Variable NOTE data
 * 東方BGM on VGS for RasberryPi Pico
 * License under GPLv3: https://github.com/suzukiplan/tohovgs-pico/blob/master/LICENSE.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#define NTYPE_ENV1 1
#define NTYPE_ENV2 2
#define NTYPE_VOL 3
#define NTYPE_MVOL 4
#define NTYPE_KEYON 5
#define NTYPE_KEYOFF 6
#define NTYPE_WAIT 7
#define NTYPE_PDOWN 8
#define NTYPE_JUMP 9
#define NTYPE_LABEL 10
#define NTYPE_WAIT_8 11
#define NTYPE_WAIT_16 12
#define NTYPE_WAIT_32 13

#define MAX_NOTES 65536

typedef struct _NOTE {
    unsigned char type;
    unsigned char op1;
    unsigned char op2;
    unsigned char op3;
    unsigned int val;
} NOTE;

static void readWriteNotes(FILE* fpR, FILE* fpW, unsigned int* length, unsigned int* loop)
{
    fseek(fpR, 0, SEEK_SET);
    NOTE note;
    std::map<int, int> indexMap;
    std::map<int, unsigned int> timeMap;
    int indexF = 0;
    int indexV = 0;
    unsigned int time = 0;
    while (sizeof(NOTE) == fread(&note, 1, sizeof(NOTE), fpR)) {
        indexMap[indexF] = indexV;
        timeMap[indexF] = time;
        if (note.type == NTYPE_WAIT) {
            if (note.val < 1) {
                // ignore 0 or minus
            } else if (note.val < 256) {
                unsigned char op1 = NTYPE_WAIT_8 << 4;
                if (fpW) fwrite(&op1, 1, 1, fpW);
                unsigned char v8 = note.val & 0xFF;
                time += v8;
                if (fpW) fwrite(&v8, 1, 1, fpW);
                indexV += 2;
            } else if (note.val < 65536) {
                unsigned char op1 = NTYPE_WAIT_16 << 4;
                if (fpW) fwrite(&op1, 1, 1, fpW);
                unsigned short v16 = note.val & 0xFFFF;
                time += v16;
                if (fpW) fwrite(&v16, 2, 1, fpW);
                indexV += 3;
            } else {
                unsigned char op1 = NTYPE_WAIT_32 << 4;
                if (fpW) fwrite(&op1, 1, 1, fpW);
                time += note.val;
                if (fpW) fwrite(&note.val, 4, 1, fpW);
                indexV += 5;
            }
            indexF++;
            continue;
        }
        unsigned char op1 = note.type & 0x0F;
        op1 <<= 4;
        op1 |= note.op1 & 0x0F;
        if (fpW) fwrite(&op1, 1, 1, fpW);
        indexV++;
        switch (note.type) {
            case NTYPE_JUMP: {
                int a = indexMap[note.val];
                *loop = timeMap[note.val];
                if (fpW) fwrite(&a, 4, 1, fpW);
                indexV += 4;
                break;
            }
            case NTYPE_ENV1:
            case NTYPE_ENV2:
            case NTYPE_PDOWN: {
                short v16 = (short)note.val;
                if (fpW) fwrite(&v16, 2, 1, fpW);
                indexV += 2;
                break;
            }
            case NTYPE_VOL:
            case NTYPE_MVOL: {
                unsigned char v8 = note.val & 0xFF;
                if (fpW) fwrite(&v8, 1, 1, fpW);
                indexV++;
                break;
            }
            case NTYPE_KEYON: {
                if (fpW) {
                    fwrite(&note.op2, 1, 1, fpW);
                    fwrite(&note.op3, 1, 1, fpW);
                }
                indexV += 2;
                break;
            }
            case NTYPE_KEYOFF:
            case NTYPE_LABEL: {
                break;
            }
            default:
                printf("detect invalid type (%d)\n", note.type);
                exit(-1);
        }
        indexF++;
    }
    *length = timeMap[indexF - 1];
}

static char* toTimeStr(unsigned int duration)
{
    static char result[16];
    unsigned int sec = duration / 22050;
    unsigned int min = sec / 60;
    sec %= 60;
    snprintf(result, sizeof(result), "%02d:%02d", min, sec);
    return result;
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        puts("vgsftv input output");
        return 1;
    }
    FILE* fpR = fopen(argv[1], "rb");
    FILE* fpW = fopen(argv[2], "wb");
    if (!fpR || !fpW) {
        puts("file open error");
        if (fpR) fclose(fpR);
        if (fpW) fclose(fpW);
        return -1;
    }

    unsigned int length = 0;
    unsigned int loop = 0;
    readWriteNotes(fpR, nullptr, &length, &loop);
    printf("Song length ... %s\n", toTimeStr(length));
    printf("Loop position ... %s\n", toTimeStr(loop));
    fwrite("VGSBGM-V", 1, 8, fpW);
    fwrite(&length, 4, 1, fpW);
    fwrite(&loop, 4, 1, fpW);
    readWriteNotes(fpR, fpW, &length, &loop);

    fclose(fpR);
    fclose(fpW);
    return 0;
}