#include "bmp8.h"
#include <string.h>

t_bmp8 * bmp8_loadImage(const char * filename) {
    printf("Attempting to open file: %s\n", filename);

    FILE * f = fopen(filename, "rb");
    if (!f) {
        printf("Error: Cannot open file.\n");
        return NULL;
    }

    unsigned char buffer[54];
    if (fread(buffer, 1, 54, f) != 54) {
        printf("Error: Cannot read BMP header.\n");
        fclose(f);
        return NULL;
    }

    unsigned int width = *(unsigned int*)(&buffer[18]);
    unsigned int height = *(unsigned int*)(&buffer[22]);
    unsigned short int colorDepth = *(unsigned short int*)(&buffer[28]);
    unsigned int dataSize = *(unsigned int*)(&buffer[34]);

    printf("Image properties: Width=%u, Height=%u, ColorDepth=%u, DataSize=%u\n",
           width, height, colorDepth, dataSize);

    if (colorDepth != 8) {
        printf("Error: Image is not 8-bit BMP.\n");
        fclose(f);
        return NULL;
    }

    unsigned char colortable[1024];
    if (fread(colortable, 1, 1024, f) != 1024) {
        printf("Error: Cannot read color table.\n");
        fclose(f);
        return NULL;
    }

    unsigned char * data = (unsigned char*)malloc(dataSize);
    if (!data) {
        printf("Error: Memory allocation failed.\n");
        fclose(f);
        return NULL;
    }

    if (fread(data, 1, dataSize, f) != dataSize) {
        printf("Error: Cannot read image data.\n");
        free(data);
        fclose(f);
        return NULL;
    }

    fclose(f);

    t_bmp8 * image = (t_bmp8 *)malloc(sizeof(t_bmp8));
    if (!image) {
        printf("Error: Memory allocation for image struct failed.\n");
        free(data);
        return NULL;
    }

    memcpy(image->header, buffer, 54);
    memcpy(image->colorTable, colortable, 1024);
    image->data = data;
    image->width = width;
    image->height = height;
    image->colorDepth = colorDepth;
    image->dataSize = dataSize;

    return image;
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

// Image processing functions
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

// Helper function to allocate 2D array for kernel
float** allocateKernel(int size) {
    float** kernel = (float**)malloc(size * sizeof(float*));
    for (int i = 0; i < size; i++) {
        kernel[i] = (float*)malloc(size * sizeof(float));
    }
    return kernel;
}

// Helper function to free kernel
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

    // Apply convolution
    for (unsigned int y = n; y < img->height - n; y++) {
        for (unsigned int x = n; x < img->width - n; x++) {
            float sum = 0.0f;

            // Apply kernel
            for (int i = -n; i <= n; i++) {
                for (int j = -n; j <= n; j++) {
                    int pixelPos = (y + i) * img->width + (x + j);
                    sum += tempData[pixelPos] * kernel[i + n][j + n];
                }
            }

            // Clamp values
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