#include "bmp8.h"
#include <string.h>
#include <math.h>

t_bmp8* bmp8_loadImage(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }

    t_bmp8* img = (t_bmp8*)malloc(sizeof(t_bmp8));
    if (!img) {
        fclose(file);
        printf("Error: Memory allocation failed\n");
        return NULL;
    }


    if (fread(img->header, sizeof(unsigned char), 54, file) != 54) {
        printf("Error: Invalid BMP file format\n");
        free(img);
        fclose(file);
        return NULL;
    }


    img->width = *(unsigned int*)&img->header[18];
    img->height = *(unsigned int*)&img->header[22];
    img->colorDepth = *(unsigned int*)&img->header[28];
    img->dataSize = *(unsigned int*)&img->header[34];

    // Verify it's an 8-bit image
    if (img->colorDepth != 8) {
        printf("Error: Image must be 8-bit grayscale\n");
        free(img);
        fclose(file);
        return NULL;
    }


    if (fread(img->colorTable, sizeof(unsigned char), 1024, file) != 1024) {
        printf("Error: Could not read color table\n");
        free(img);
        fclose(file);
        return NULL;
    }


    img->data = (unsigned char*)malloc(img->dataSize);
    if (!img->data) {
        printf("Error: Memory allocation failed for image data\n");
        free(img);
        fclose(file);
        return NULL;
    }


    if (fread(img->data, sizeof(unsigned char), img->dataSize, file) != img->dataSize) {
        printf("Error: Could not read image data\n");
        free(img->data);
        free(img);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return img;
}

void bmp8_saveImage(const char* filename, t_bmp8* img) {
    if (!img) return;

    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Cannot create file %s\n", filename);
        return;
    }


    fwrite(img->header, sizeof(unsigned char), 54, file);


    fwrite(img->colorTable, sizeof(unsigned char), 1024, file);


    fwrite(img->data, sizeof(unsigned char), img->dataSize, file);

    fclose(file);
}

void bmp8_free(t_bmp8* img) {
    if (img) {
        if (img->data) {
            free(img->data);
        }
        free(img);
    }
}

void bmp8_printInfo(t_bmp8* img) {
    if (!img) return;

    printf("Image Info:\n");
    printf("  Width: %u\n", img->width);
    printf("  Height: %u\n", img->height);
    printf("  Color Depth: %u\n", img->colorDepth);
    printf("  Data Size: %u\n", img->dataSize);
}


void bmp8_negative(t_bmp8* img) {
    if (!img || !img->data) return;

    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = 255 - img->data[i];
    }
}

void bmp8_brightness(t_bmp8* img, int value) {
    if (!img || !img->data) return;

    for (unsigned int i = 0; i < img->dataSize; i++) {
        int newValue = img->data[i] + value;
        if (newValue > 255) newValue = 255;
        if (newValue < 0) newValue = 0;
        img->data[i] = (unsigned char)newValue;
    }
}

void bmp8_threshold(t_bmp8* img, int threshold) {
    if (!img || !img->data) return;

    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = (img->data[i] >= threshold) ? 255 : 0;
    }
}


float** allocateKernel(int size) {
    float** kernel = (float**)malloc(size * sizeof(float*));
    for (int i = 0; i < size; i++) {
        kernel[i] = (float*)malloc(size * sizeof(float));
    }
    return kernel;
}


void freeKernel(float** kernel, int size) {
    for (int i = 0; i < size; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

void bmp8_applyFilter(t_bmp8* img, float** kernel, int kernelSize) {
    if (!img || !img->data || !kernel) return;

    unsigned char* tempData = (unsigned char*)malloc(img->dataSize);
    if (!tempData) return;
    memcpy(tempData, img->data, img->dataSize);

    int n = kernelSize / 2;


    for (unsigned int y = n; y < img->height - n; y++) {
        for (unsigned int x = n; x < img->width - n; x++) {
            float sum = 0.0f;


            for (int i = -n; i <= n; i++) {
                for (int j = -n; j <= n; j++) {
                    int pixelPos = (y + i) * img->width + (x + j);
                    sum += tempData[pixelPos] * kernel[i + n][j + n];
                }
            }


            if (sum > 255) sum = 255;
            if (sum < 0) sum = 0;

            img->data[y * img->width + x] = (unsigned char)sum;
        }
    }

    free(tempData);
}

void bmp8_boxBlur(t_bmp8* img) {
    float** kernel = allocateKernel(3);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            kernel[i][j] = 1.0f / 9.0f;
        }
    }

    bmp8_applyFilter(img, kernel, 3);
    freeKernel(kernel, 3);
}

void bmp8_gaussianBlur(t_bmp8* img) {
    float** kernel = allocateKernel(3);
    float gaussianKernel[3][3] = {
        {1.0f/16, 2.0f/16, 1.0f/16},
        {2.0f/16, 4.0f/16, 2.0f/16},
        {1.0f/16, 2.0f/16, 1.0f/16}
    };

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            kernel[i][j] = gaussianKernel[i][j];
        }
    }

    bmp8_applyFilter(img, kernel, 3);
    freeKernel(kernel, 3);
}

void bmp8_outline(t_bmp8* img) {
    float** kernel = allocateKernel(3);
    float outlineKernel[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            kernel[i][j] = outlineKernel[i][j];
        }
    }

    bmp8_applyFilter(img, kernel, 3);
    freeKernel(kernel, 3);
}

void bmp8_emboss(t_bmp8* img) {
    float** kernel = allocateKernel(3);
    float embossKernel[3][3] = {
        {-2, -1,  0},
        {-1,  1,  1},
        { 0,  1,  2}
    };

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            kernel[i][j] = embossKernel[i][j];
        }
    }

    bmp8_applyFilter(img, kernel, 3);
    freeKernel(kernel, 3);
}

void bmp8_sharpen(t_bmp8* img) {
    float** kernel = allocateKernel(3);
    float sharpenKernel[3][3] = {
        { 0, -1,  0},
        {-1,  5, -1},
        { 0, -1,  0}
    };

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            kernel[i][j] = sharpenKernel[i][j];
        }
    }

    bmp8_applyFilter(img, kernel, 3);
    freeKernel(kernel, 3);
}