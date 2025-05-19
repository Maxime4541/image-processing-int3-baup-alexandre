#ifndef BMP24_H
#define BMP24_H

#include <stdio.h>
#include <stdlib.h>


typedef struct {
    unsigned short type;
    unsigned int size;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;
} t_bmp_header;

typedef struct {
    unsigned int size;
    int width;
    int height;
    unsigned short planes;
    unsigned short bits;
    unsigned int compression;
    unsigned int imageSize;
    int xpixels;
    int ypixels;
    unsigned int colors;
    unsigned int importantColors;
} t_bmp_info;


typedef struct {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
} t_pixel;


typedef struct {
    t_bmp_header header;
    t_bmp_info header_info;
    int width;
    int height;
    int colorDepth;
    t_pixel **data;
} t_bmp24;


#define BITMAP_MAGIC 0x0
#define BITMAP_SIZE 0x2
#define BITMAP_OFFSET 0xA
#define BITMAP_WIDTH 0x12
#define BITMAP_HEIGHT 0x16
#define BITMAP_DEPTH 0x1C
#define BITMAP_SIZE_RAW 0x22
#define BMP_TYPE 0x4D42
#define HEADER_SIZE 0x0E
#define INFO_SIZE 0x28
#define DEFAULT_DEPTH 0x18


t_bmp24* bmp24_loadImage(const char* filename);
void bmp24_saveImage(const char* filename, t_bmp24* img);
void bmp24_free(t_bmp24* img);
void bmp24_printInfo(t_bmp24* img);

void bmp24_readPixelValue(t_bmp24* img, int x, int y, FILE* file);
void bmp24_writePixelValue(t_bmp24* img, int x, int y, FILE* file);
void bmp24_readPixelData(t_bmp24* img, FILE* file);
void bmp24_writePixelData(t_bmp24* img, FILE* file);

void bmp24_negative(t_bmp24* img);
void bmp24_grayscale(t_bmp24* img);
void bmp24_brightness(t_bmp24* img, int value);

t_pixel bmp24_convolution(t_bmp24* img, int x, int y, float** kernel, int kernelSize);
void bmp24_applyFilter(t_bmp24* img, float** kernel, int kernelSize);
void bmp24_boxBlur(t_bmp24* img);
void bmp24_gaussianBlur(t_bmp24* img);
void bmp24_outline(t_bmp24* img);
void bmp24_emboss(t_bmp24* img);
void bmp24_sharpen(t_bmp24* img);

float** allocateKernel24(int size);
void freeKernel24(float** kernel, int size);

#endif