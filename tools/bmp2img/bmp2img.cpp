#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct BitmapHeader_ {
    int isize;             /* 情報ヘッダサイズ */
    int width;             /* 幅 */
    int height;            /* 高さ */
    unsigned short planes; /* プレーン数 */
    unsigned short bits;   /* 色ビット数 */
    unsigned int ctype;    /* 圧縮形式 */
    unsigned int gsize;    /* 画像データサイズ */
    int xppm;              /* X方向解像度 */
    int yppm;              /* Y方向解像度 */
    unsigned int cnum;     /* 使用色数 */
    unsigned int inum;     /* 重要色数 */
} BitmapHeader;

int main(int argc, char* argv[])
{
    int tW = 0;
    int tH = 0;
    char* path = nullptr;
    bool isError = false;
    bool isMisaki = false;

    for (int i = 1; !isError && i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 't':
                    i++;
                    if (argc <= i) {
                        isError = true;
                    } else {
                        tW = atoi(argv[i]);
                        if (tW < 1) {
                            isError = true;
                        } else {
                            const char* cp = argv[i];
                            while (isdigit(*cp)) cp++;
                            if (*cp != 'x') {
                                isError = true;
                            } else {
                                cp++;
                                tH = atoi(cp);
                                if (tH < 1) {
                                    isError = true;
                                }                                
                            }
                        }
                    }
                    break;
                case 'm':
                    isMisaki = true;
                    break;
                default:
                    fprintf(stderr, "unknown option: %s\n", argv[i]);
                    isError = true;
                    break;
            }
        } else {
            path = argv[i];
        }
    }

    if (!path || isError) {
        fprintf(stderr, "bmp2img [-t WxH]\n");
        fprintf(stderr, "        /path/to/image.bmp\n");
        return -1;
    }

    char imgName[1024];
    char* cp = strrchr(path, '/');
    if (cp) {
        strcpy(imgName, cp + 1);
    } else {
        cp = strrchr(argv[1], '\\');
        if (cp) {
            strcpy(imgName, cp + 1);
        } else {
            strcpy(imgName, path);
        }
    }
    cp = strchr(imgName, '.');
    if (cp) *cp = 0;

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "file open error\n");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int size = (int)ftell(fp);
    if (size < 64) {
        fprintf(stderr, "invalid file format\n");
        fclose(fp);
        return -1;
    }
    fseek(fp, 0, SEEK_SET);
    unsigned char* bmp = (unsigned char*)malloc(size);
    fread(bmp, 1, size, fp);
    fclose(fp);

    if (0 != memcmp(bmp, "BM", 2)) {
        fprintf(stderr, "invalid file format (BM)\n");
        free(bmp);
        return -1;
    }
    BitmapHeader head;
    memcpy(&head, &bmp[14], sizeof(head));
    if (8 != head.bits) {
        fprintf(stderr, "invalid file format (not 8bit color mode)\n");
        free(bmp);
        return -1;
    }
    if (0 != head.ctype) {
        fprintf(stderr, "invalid file format (not uncompressed)\n");
        free(bmp);
        return -1;
    }
    if (tW && head.width % tW) {
        fprintf(stderr, "invalid width (not divisible by tile width)\n");
        free(bmp);
        return -1;
    }
    if (tH && head.height % tH) {
        fprintf(stderr, "invalid height (not divisible by tile height)\n");
        free(bmp);
        return -1;
    }

    unsigned int offset;
    memcpy(&offset, &bmp[0xA], 4);
    unsigned char* palPtr = &bmp[14 + 40];
    unsigned short pal565[256];
    for (int i = 0; i < 256; i++, palPtr += 4) {
        pal565[i] = (palPtr[2] & 0b11111000) >> 3;
        pal565[i] <<= 6;
        pal565[i] |= (palPtr[1] & 0b11111100) >> 2;
        pal565[i] <<= 5;
        pal565[i] |= (palPtr[0] & 0b11111000) >> 3;
        unsigned short work = (pal565[i] & 0xFF00) >> 8;
        pal565[i] &= 0xFF;
        pal565[i] <<= 8;
        pal565[i] |= work;
    }
    int imgSize = head.width * head.height * 2;
    unsigned short* img = (unsigned short*)malloc(imgSize);
    unsigned short* imgPtr = img;
    unsigned char* bmpPtr = &bmp[offset];
    for (int y = 0; y < head.height; y++) {
        for (int x = 0; x < head.width; x++) {
            imgPtr[y * head.width + x] = pal565[bmpPtr[(head.height - y - 1) * head.width + x]];
        }
    }
    free(bmp);
    if (tW && tH) {
        unsigned short* imgTmp = (unsigned short*)malloc(imgSize);
        int cur = 0;
        for (int y = 0; y < head.height; y+= tH) {
            for (int x = 0; x < head.width; x+= tW) {
                for (int yy = 0; yy < tH; yy++) {
                    for (int xx = 0; xx < tW; xx++) {
                        imgTmp[cur++] = img[(y + yy) * head.width + x + xx];
                    }
                }
            }
        }
        free(img);
        img = imgTmp;
    }
    if (isMisaki) {
        int num = head.width / tW * head.height / tH;
        imgSize = num * 12;
        unsigned char* imgTmp = (unsigned char*)malloc(imgSize);
        int imgIdx = 0;
        int tmpIdx = 0;
        for (int i = 0; i < num; i++) {
            for (int y = 0; y < tH; y++) {
                unsigned char bit = 0;
                for (int x = 0; x < tW; x++) {
                    bit <<= 1;
                    bit |= img[imgIdx++] ? 0 : 1;
                }
                if (4 == tW) {
                    bit <<= 4; // 半角は左詰めにする
                }
                imgTmp[tmpIdx++] = bit;
            }
        }
        printf("const unsigned char rom_%s[%d] = {\n", imgName, imgSize);
        bool firstLine = true;
        unsigned char* cptr = (unsigned char*)img;
        int left = imgSize;
        for (int i = 0; i < imgSize; i += 32, left -= 32) {
            if (firstLine) {
                firstLine = false;
            } else {
                printf(",\n");
            }
            printf("    ");
            for (int j = 0; j < (left < 32 ? left : 32); j++) {
                if (j) {
                    printf(", 0x%02X", imgTmp[i + j]);
                } else {
                    printf("0x%02X", imgTmp[i + j]);
                }
            }
        }
        printf("\n};\n");
    } else {
        imgSize /= 2;
        printf("const unsigned short rom_%s[%d] = {\n", imgName, imgSize);
        bool firstLine = true;
        unsigned char* cptr = (unsigned char*)img;
        int left = imgSize;
        for (int i = 0; i < imgSize; i += 16, left -= 16) {
            if (firstLine) {
                firstLine = false;
            } else {
                printf(",\n");
            }
            printf("    ");
            for (int j = 0; j < (left < 16 ? left : 16); j++) {
                if (j) {
                    printf(", 0x%04X", img[i + j]);
                } else {
                    printf("0x%04X", img[i + j]);
                }
            }
        }
        printf("\n};\n");
    }
    free(img);
    return 0;
}
