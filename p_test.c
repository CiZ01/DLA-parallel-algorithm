#include <stdio.h>
#include <gd.h>

gdImagePtr im, im2, im;
int black, white, trans;
FILE *out;

int main(){
    im = gdImageCreate(100, 100);     // Create the image
    white = gdImageColorAllocate(im, 255, 255, 255); // Allocate background
    black = gdImageColorAllocate(im, 0, 0, 0); // Allocate drawing color
    trans = gdImageColorAllocate(im, 1, 1, 1); // trans clr for compression
    gdImageRectangle(im, 0, 0, 10, 10, black); // Draw rectangle

    out = fopen("anim.gif", "wb");// Open output file in binary mode
    gdImageGifAnimBegin(im, out, 1, 3);// Write GIF hdr, global clr map,loops
    // Write the first frame.  No local color map.  Delay = 1s
    gdImageGifAnimAdd(im, out, 0, 0, 0, 100, 1, NULL);

    // construct the second frame
    im2 = gdImageCreate(100, 100);
    (void)gdImageColorAllocate(im2, 255, 255, 255); // White background
    gdImagePaletteCopy (im2, im);  // Make sure the palette is identical
    gdImageRectangle(im2, 0, 0, 15, 15, black);    // Draw something
    // Allow animation compression with transparent pixels
    gdImageColorTransparent (im2, trans);
    gdImageGifAnimAdd(im2, out, 0, 0, 0, 100, 1, im);  // Add second frame

    // construct the third frame
    im = gdImageCreate(100, 100);
    (void)gdImageColorAllocate(im, 255, 255, 255); // white background
    gdImageRectangle(im, 0, 0, 15, 20, black); // Draw something
    // Allow animation compression with transparent pixels
    gdImageColorTransparent (im, trans);
    // Add the third frame, compressing against the second one
    gdImageGifAnimAdd(im, out, 0, 0, 0, 100, 1, im2);
    gdImageGifAnimEnd(out);  // End marker, same as putc(';', out);
    fclose(out); // Close file

    // Destroy images
    gdImageDestroy(im);
    gdImageDestroy(im2);
}