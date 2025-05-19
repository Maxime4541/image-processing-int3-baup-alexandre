#include "bmp24.h"
#include <string.h>
#include <math.h>
void file_readdata(unsigned int position, void* buffer, unsigned int size, size_t n, FILE* file) {
    fseek(file, position, SEEK_SET);
    fread(buffer, size, n, file);
}

void file_writedata(unsigned int position, void* buffer, unsigned int size, size_t n, FILE* file) {
    fseek(file, position, SEEK_SET);
    fwrite(buffer, size, n, file);
}

t_pixel** allocatePixelData(int width, int height) {
    t_pixel** data = (t_pixel**)malloc(height * sizeof(t_pixel*));
    for (int i = 0; i < height; i++) {
        data[i] = (t_pixel*)malloc(width * sizeof(t_pixel));
    }
    return data;
}

void freePixelData(t_pixel** data, int height) {
    for (int i = 0; i < height; i++) {
        free(data[i]);
    }
    free(data);
}

float** allocateKernel24(int size) {
    float** kernel = (float**)malloc(size * sizeof(float*));
    for (int i = 0; i < size; i++) {
        kernel[i] = (float*)malloc(size * sizeof(float));
    }
    return kernel;
}

void freeKernel24(float** kernel, int size) {
    for (int i = 0; i < size; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

t_bmp24* bmp24_loadImage(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }

    t_bmp24* img = (t_bmp24*)malloc(sizeof(t_bmp24));
    if (!img) {
        fclose(file);
        printf("Error: Memory allocation failed\n");
        return NULL;
    }

    file_readdata(BITMAP_MAGIC, &img->header.type, sizeof(unsigned short), 1, file);
    if (img->header.type != BMP_TYPE) {
        printf("Error: Not a BMP file\n");
        free(img);
        fclose(file);
        return NULL;
    }

    file_readdata(BITMAP_SIZE, &img->header.size, sizeof(unsigned int), 1, file);
    file_readdata(BITMAP_OFFSET, &img->header.offset, sizeof(unsigned int), 1, file);
    file_readdata(HEADER_SIZE, &img->header_info, sizeof(t_bmp_info), 1, file);
    img->width = img->header_info.width;
    img->height = img->header_info.height;
    img->colorDepth = img->header_info.bits;
    if (img->colorDepth != 24) {
        printf("Error: Image must be 24-bit color\n");
        free(img);
        fclose(file);
        return NULL;
    }

    img->data = allocatePixelData(img->width, img->height);
    if (!img->data) {
        printf("Error: Memory allocation failed for image data\n");
        free(img);
        fclose(file);
        return NULL;
    }

    bmp24_readPixelData(img, file);

    fclose(file);
    return img;
}

void bmp24_saveImage(const char* filename, t_bmp24* img) {
    if (!img) return;

    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Cannot create file %s\n", filename);
        return;
    }

    file_writedata(BITMAP_MAGIC, &img->header.type, sizeof(unsigned short), 1, file);
    file_writedata(BITMAP_SIZE, &img->header.size, sizeof(unsigned int), 1, file);
    file_writedata(BITMAP_OFFSET, &img->header.offset, sizeof(unsigned int), 1, file);
    file_writedata(HEADER_SIZE, &img->header_info, sizeof(t_bmp_info), 1, file);
    bmp24_writePixelData(img, file);

    fclose(file);
}

void bmp24_free(t_bmp24* img) {
    if (img) {
        if (img->data) {
            freePixelData(img->data, img->height);
        }
        free(img);
    }
}

void bmp24_printInfo(t_bmp24* img) {
    if (!img) return;

    printf("Image Info:\n");
    printf("  Width: %d\n", img->width);
    printf("  Height: %d\n", img->height);
    printf("  Color Depth: %d\n", img->colorDepth);
    printf("  Data Size: %u\n", img->header_info.imageSize);
}

void bmp24_readPixelValue(t_bmp24* img, int x, int y, FILE* file) {
    if (!img || !file) return;

    unsigned int position = img->header.offset + ((img->height - 1 - y) * img->width + x) * 3;

    file_readdata(position, &img->data[y][x], sizeof(t_pixel), 1, file);
}

void bmp24_writePixelValue(t_bmp24* img, int x, int y, FILE* file) {
    if (!img || !file) return;
    unsigned int position = img->header.offset + ((img->height - 1 - y) * img->width + x) * 3;
    file_writedata(position, &img->data[y][x], sizeof(t_pixel), 1, file);
}

void bmp24_readPixelData(t_bmp24* img, FILE* file) {
    if (!img || !file) return;
    int rowSize = ((img->width * 3 + 3) / 4) * 4;
    unsigned char* rowBuffer = (unsigned char*)malloc(rowSize);

    for (int y = 0; y < img->height; y++) {
        unsigned int position = img->header.offset + (img->height - 1 - y) * rowSize;
        file_readdata(position, rowBuffer, 1, rowSize, file);
        for (int x = 0; x < img->width; x++) {
            img->data[y][x].blue = rowBuffer[x * 3];
            img->data[y][x].green = rowBuffer[x * 3 + 1];
            img->data[y][x].red = rowBuffer[x * 3 + 2];
        }
    }
    free(rowBuffer);
}
void bmp24_writePixelData(t_bmp24* img, FILE* file) {
    if (!img || !file) return;
    int rowSize = ((img->width * 3 + 3) / 4) * 4;
    unsigned char* rowBuffer = (unsigned char*)malloc(rowSize);
    for (int y = 0; y < img->height; y++) {
        memset(rowBuffer, 0, rowSize);
        for (int x = 0; x < img->width; x++) {
            rowBuffer[x * 3] = img->data[y][x].blue;
            rowBuffer[x * 3 + 1] = img->data[y][x].green;
            rowBuffer[x * 3 + 2] = img->data[y][x].red;
        }
        unsigned int position = img->header.offset + (img->height - 1 - y) * rowSize;
        file_writedata(position, rowBuffer, 1, rowSize, file);
    }

    free(rowBuffer);
}

void bmp24_negative(t_bmp24* img) {
    if (!img || !img->data) return;
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x].red = 255 - img->data[y][x].red;
            img->data[y][x].green = 255 - img->data[y][x].green;
            img->data[y][x].blue = 255 - img->data[y][x].blue;
        }
    }
}
void bmp24_grayscale(t_bmp24* img) {
    if (!img || !img->data) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            unsigned char gray = (img->data[y][x].red + img->data[y][x].green + img->data[y][x].blue) / 3;
            img->data[y][x].red = gray;
            img->data[y][x].green = gray;
            img->data[y][x].blue = gray;
        }
    }
}
void bmp24_brightness(t_bmp24* img, int value) {
    if (!img || !img->data) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int red = img->data[y][x].red + value;
            int green = img->data[y][x].green + value;
            int blue = img->data[y][x].blue + value;

            img->data[y][x].red = (red > 255) ? 255 : (red < 0) ? 0 : red;
            img->data[y][x].green = (green > 255) ? 255 : (green < 0) ? 0 : green;
            img->data[y][x].blue = (blue > 255) ? 255 : (blue < 0) ? 0 : blue;
        }
    }
}
t_pixel bmp24_convolution(t_bmp24* img, int x, int y, float** kernel, int kernelSize) {
    float sumR = 0.0f, sumG = 0.0f, sumB = 0.0f;
    int n = kernelSize / 2;

    for (int i = -n; i <= n; i++) {
        for (int j = -n; j <= n; j++) {
            int newY = y + i;
            int newX = x + j;
            
            if (newY >= 0 && newY < img->height && newX >= 0 && newX < img->width) {
                sumR += img->data[newY][newX].red * kernel[i + n][j + n];
                sumG += img->data[newY][newX].green * kernel[i + n][j + n];
                sumB += img->data[newY][newX].blue * kernel[i + n][j + n];
            }
        }
    }

    t_pixel result;
    result.red = (sumR > 255) ? 255 : (sumR < 0) ? 0 : (unsigned char)sumR;
    result.green = (sumG > 255) ? 255 : (sumG < 0) ? 0 : (unsigned char)sumG;
    result.blue = (sumB > 255) ? 255 : (sumB < 0) ? 0 : (unsigned char)sumB;

    return result;
}

void bmp24_applyFilter(t_bmp24* img, float** kernel, int kernelSize) {
    if (!img || !img->data || !kernel) return;
    t_pixel** tempData = allocatePixelData(img->width, img->height);
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            tempData[y][x] = img->data[y][x];
        }
    }
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x] = bmp24_convolution(img, x, y, kernel, kernelSize);
        }
    }

    freePixelData(tempData, img->height);
}
void bmp24_boxBlur(t_bmp24* img) {
    float** kernel = allocateKernel24(3);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            kernel[i][j] = 1.0f / 9.0f;
        }
    }
    
    bmp24_applyFilter(img, kernel, 3);
    freeKernel24(kernel, 3);
}

void bmp24_gaussianBlur(t_bmp24* img) {
    float** kernel = allocateKernel24(3);
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
    
    bmp24_applyFilter(img, kernel, 3);
    freeKernel24(kernel, 3);
}

void bmp24_outline(t_bmp24* img) {
    float** kernel = allocateKernel24(3);
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
    
    bmp24_applyFilter(img, kernel, 3);
    freeKernel24(kernel, 3);
}

void bmp24_emboss(t_bmp24* img) {
    float** kernel = allocateKernel24(3);
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
    
    bmp24_applyFilter(img, kernel, 3);
    freeKernel24(kernel, 3);
}

void bmp24_sharpen(t_bmp24* img) {
    float** kernel = allocateKernel24(3);
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
    
    bmp24_applyFilter(img, kernel, 3);
    freeKernel24(kernel, 3);
} 