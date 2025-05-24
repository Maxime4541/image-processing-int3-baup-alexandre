

#ifndef HISTOGRAM_EQUALIZATION_H
#define HISTOGRAM_EQUALIZATION_H

typedef struct {
    float y;
    float u;
    float v;
} t_yuv;

unsigned int* bmp8_computeHistogram(t_bmp8* img);
unsigned int* bmp8_computeCDF(unsigned int* hist, unsigned int dataSize);
void bmp8_equalize(t_bmp8* img);
void bmp24_equalize(t_bmp24* img);

#endif
