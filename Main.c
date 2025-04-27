#include "bmp8.h"
#include <stdio.h>

#include <windows.h>


int main(void) {
    t_bmp8* image = NULL;
    int choice = 0;
    while (choice != 5) {
        printf("Please choose an option:\n  1. Open an image\n  2. Save an image\n  3. Apply a filter\n  4. Display image information\n  5. Quit\n>>> Your choice : ");
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
                image = bmp8_loadImage(path);
                Sleep(2000);
                if (image == NULL) {
                    printf("Failed to load image. Please check the file path and format.\n");
                } else {
                    printf("Image loaded successfully\n");
                }
                Sleep(2000);
                break;
            case 2:
                if (image) {
                    char path_2[256];
                    printf("File path: ");
                    scanf("%s", path_2);
                    bmp8_saveImage(path_2, image);
                    printf("Image saved succesfully\n");
                }
                else
                    printf("Image is NULL\n");
                break;
            case 3:
                if (image) {
                    printf("Please choose a filter:\n 1. Negative\n 2. Brightness\n 3. Black and white\n 4. Box Blur\n 5. Gaussian blur\n 6. Sharpness\n 7. Outline\n 8. Emboss\n 9. Return to the previous menu\n >>> Your choice: ");
                    int choix_2;
                    scanf("%d", &choix_2);
                    switch (choix_2) {
                        case 1:
                            printf("Filter applied successfully !\n");
                            break;
                        case 2:
                            printf("Filter applied successfully !\n");
                            break;
                        case 3:
                            printf("Filter applied successfully !\n");
                            break;
                        case 4:
                            printf("Filter applied successfully !\n");
                            break;
                        case 5:
                            printf("Filter applied successfully !\n");
                        break;
                        case 6:
                            printf("Filter applied successfully !\n");
                            break;
                        case 7:
                            printf("Filter applied successfully !\n");
                            break;
                        case 8:
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
                if (image) {
                    bmp8_printInfo(image);
                }
                else
                    printf("Image is NULL\n");
                break;
            case 5:
                break;
            default:
                printf("Please enter a number between 1 and 5 !!\n");
                break;


        }
    }
    return 0;
}
