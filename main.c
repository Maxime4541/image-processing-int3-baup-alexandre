#include "bmp8.h"
#include "bmp24.h"
#include "Histogram_equalization.h"
#include <stdio.h>

#include <windows.h>

int _8bit(const char* filename) {
    FILE* file = fopen(filename, "rb");
    unsigned char header[54];
    fread(header, 1, 54, file);
    unsigned int colorDepth = *(unsigned int*)&header[28];
    fclose(file);
    if (colorDepth == 8) {
        return 1;
    } else if (colorDepth == 24) {
        return 0;
    }
}



int main(void) {
    t_bmp8* image8 = NULL;
    t_bmp24* image24 = NULL;
    int choice = 0;
    while (choice != 6) {
        printf("Please choose an option:\n  1. Open an image\n  2. Save an image\n  3. Apply a filter\n  4. Display image information\n  5. Equalize Historigram\n  6. Quit\n>>> Your choice : ");
        if (scanf(" %d", &choice) != 1) {
            printf("Invalid input! Please enter a number.\n");
            while (getchar() != '\n') {
                choice = 0;
            }
        }
        switch (choice) {
            case 1:
                char path[256];
                printf("File path: ");
                scanf(" %255[^\n]", path);
                if (image8) {
                    free(image8);
                    image8 = NULL;
                }
                if (image24) {
                    free(image24);
                    image24 = NULL;
                }
                if (_8bit(path)) {
                    image8 = bmp8_loadImage(path);
                } else {
                    image24 = bmp24_loadImage(path);
                }
                Sleep(2000);
                if (image8 == NULL && image24 == NULL) {
                    printf("Failed to load image. Please check the file path and format.\n");
                } else {
                    if (image8) {
                        printf("8 bit image loaded successfully\n");
                    } else if (image24) {
                        printf("24 bit image loaded successfully\n");
                    }
                }
                Sleep(2000);
                break;
            case 2:
                if (image8 || image24) {
                    char path_2[256];
                    printf("File path: ");
                    scanf("%s", path_2);
                    if (image8) {
                        bmp8_saveImage(path_2, image8);
                    }

                    if (image24) {
                        bmp24_saveImage(path_2, image24);
                    }
                    printf("Image saved succesfully\n");
                }
                else
                    printf("Image is NULL\n");
                break;
            case 3:
                if (image8 || image24) {
                    printf("Please choose a filter:\n 1. Negative\n 2. Brightness\n 3. Black and white\n 4. Box Blur\n 5. Gaussian blur\n 6. Sharpness\n 7. Outline\n 8. Emboss\n 9. Return to the previous menu\n >>> Your choice: ");
                    int choix_2;
                    scanf("%d", &choix_2);
                    switch (choix_2) {
                        case 1:
                            if (image8) {
                                bmp8_negative(image8);
                            }
                            if (image24) {
                                bmp24_negative(image24);
                            }
                            printf("Filter applied successfully !\n");
                            break;
                        case 2:
                            int value;
                            printf("Enter brightness value (-255 to 255): ");
                            scanf("%d", &value);
                            if (image8) {
                                bmp8_brightness(image8, value);
                            }
                            if (image24) {
                                bmp24_brightness(image24, value);
                            }
                            printf("Filter applied successfully !\n");
                            break;
                        case 3:
                            if (image8) {
                                printf("Enter threshold value (0 to 255): ");
                                scanf("%d", &value);
                                bmp8_threshold(image8, value);
                            }
                            if (image24) {
                                bmp24_grayscale(image24);
                            }
                            printf("Filter applied successfully !\n");
                            break;
                        case 4:
                            if (image8) {
                                bmp8_boxBlur(image8);
                            }
                            if (image24) {
                                bmp24_boxBlur(image24);
                            }
                            printf("Filter applied successfully !\n");
                            break;
                        case 5:
                            if (image8) {
                                bmp8_gaussianBlur(image8);
                            }
                            if (image24) {
                                bmp24_gaussianBlur(image24);
                            }
                            printf("Filter applied successfully !\n");
                            break;
                        case 6:
                            if (image8) {
                                bmp8_sharpen(image8);
                            }
                            if (image24) {
                                bmp24_sharpen(image24);
                            }
                            printf("Filter applied successfully !\n");
                            break;
                        case 7:
                            if (image8) {
                                bmp8_outline(image8);
                            }
                            if (image24) {
                                bmp24_outline(image24);
                            }
                            printf("Filter applied successfully !\n");
                            break;
                        case 8:
                            if (image8) {
                                bmp8_emboss(image8);
                            }
                            if (image24) {
                                bmp24_emboss(image24);
                            }
                            printf("Filter applied successfully !\n");
                            break;
                        case 9:
                            break;
                    }
                }
                else
                    printf("Image is NULL\n");
                break;
            case 4:
                if (image8 || image24) {
                    if (image8) {
                        bmp8_printInfo(image8);
                    }
                    if (image24) {
                        bmp24_printInfo(image24);
                    }
                }
                else
                    printf("Image is NULL\n");
                break;
            case 5:
                if (image8 || image24) {
                    if (image8) {
                        bmp8_equalize(image8);
                        printf("8-bit image equalized successfully\n");
                    }
                    if (image24) {
                        bmp24_equalize(image24);
                        printf("24-bit image equalized successfully\n");
                    }
                }
                else
                    printf("Image is NULL\n");
                break;
            case 6 :
                break;
            default:
                printf("Please enter a number between 1 and 5 !!\n");
                break;


        }
    }
    return 0;
}
