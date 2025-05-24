#include <stddef.h>
#include <stdlib.h>
#include <tgmath.h>

#include "bmp24.h"
#include "bmp8.h"
#include "Histogram_equalization.h"

unsigned int* bmp8_computeHistogram(t_bmp8* img) {
    if (!img || !img->data) return NULL;
    unsigned int* hist = (unsigned int*)calloc(256, sizeof(unsigned int));
    if (!hist) return NULL;
    for (unsigned int i = 0; i < img->dataSize; i++) {
        hist[img->data[i]]++;
    }
    return hist;
}

unsigned int* bmp8_computeCDF(unsigned int* hist, unsigned int dataSize) {
    if (!hist) return NULL;
    unsigned int* hist_eq = (unsigned int*)malloc(256 * sizeof(unsigned int));
    if (!hist_eq) return NULL;

    unsigned int cdf[256] = {0};
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + hist[i];
    }

    unsigned int cdf_min = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] != 0) {
            cdf_min = cdf[i];
            break;
        }
    }

    for (int i = 0; i < 256; i++) {
        hist_eq[i] = round(((float)(cdf[i] - cdf_min) / (dataSize - cdf_min)) * 255);
    }

    return hist_eq;
}

void bmp8_equalize(t_bmp8* img) {
    if (!img || !img->data) return;

    unsigned int* hist = bmp8_computeHistogram(img);
    if (!hist) return;

    unsigned int* hist_eq = bmp8_computeCDF(hist, img->dataSize);
    free(hist);
    if (!hist_eq) return;

    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = (unsigned char)hist_eq[img->data[i]];
    }

    free(hist_eq);
}

void bmp24_equalize(t_bmp24* img) {
    if (!img || !img->data) return;
    int w = img->width;
    int h = img->height;
    int size = w * h;

    t_yuv** yuv = (t_yuv**)malloc(h * sizeof(t_yuv*));
    for (int i = 0; i < h; i++) {
        yuv[i] = (t_yuv*)malloc(w * sizeof(t_yuv));
    }

    unsigned int hist[256] = {0};

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            t_pixel px = img->data[y][x];
            float r = px.red, g = px.green, b = px.blue;
            float Y = 0.299f * r + 0.587f * g + 0.114f * b;
            float U = -0.14713f * r - 0.28886f * g + 0.436f * b;
            float V = 0.615f * r - 0.51499f * g - 0.10001f * b;
            yuv[y][x].y = Y;
            yuv[y][x].u = U;
            yuv[y][x].v = V;
            int y_int = (int)round(Y);
            if (y_int < 0) y_int = 0;
            if (y_int > 255) y_int = 255;
            hist[y_int]++;
        }
    }

    unsigned int cdf[256] = {0};
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + hist[i];
    }
    unsigned int cdf_min = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] != 0) {
            cdf_min = cdf[i];
            break;
        }
    }
    unsigned int hist_eq[256];
    for (int i = 0; i < 256; i++) {
        hist_eq[i] = round(((float)(cdf[i] - cdf_min) / (size - cdf_min)) * 255);
    }

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int y_int = (int)round(yuv[y][x].y);
            if (y_int < 0) y_int = 0;
            if (y_int > 255) y_int = 255;
            float Y = hist_eq[y_int];
            float U = yuv[y][x].u;
            float V = yuv[y][x].v;

            float r = Y + 1.13983f * V;
            float g = Y - 0.39465f * U - 0.58060f * V;
            float b = Y + 2.03211f * U;

            if (r < 0) r = 0; if (r > 255) r = 255;
            if (g < 0) g = 0; if (g > 255) g = 255;
            if (b < 0) b = 0; if (b > 255) b = 255;

            img->data[y][x].red = (unsigned char)round(r);
            img->data[y][x].green = (unsigned char)round(g);
            img->data[y][x].blue = (unsigned char)round(b);
        }
        free(yuv[y]);
    }
    free(yuv);
}