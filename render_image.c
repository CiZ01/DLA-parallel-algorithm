#include <gd.h>
#include "support_functions.c"

void createImage(gdImagePtr* p_img, int width, int height, cell** matrix) {

    int black = gdImageColorAllocate(img, 0, 0, 0);
    int white = gdImageColorAllocate(img, 255, 255, 255);

    for (int y = 0; y<height; y++) {
        for (int x = 0; x < width; x++) {
            int color = matrix[y][x].value == 0 ? white : black;
            gdImageSetPixel(img, x, y, color);
        }
    }
}

void saveImage(gdImagePtr img){
    // Salva l'immagine
    FILE *out = fopen("image.bmp", "wb");
    gdImageBmp(img, out, -1);
    fclose(out);

    // Liberare la memoria
    gdImageDestroy(img);

}
