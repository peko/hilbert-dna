#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./stb_image_write.h"

typedef unsigned char byte;

//rotate/flip a quadrant appropriately
void rot(uint32_t n, uint32_t *x, uint32_t *y, uint32_t rx, uint32_t ry) {
    if (ry == 0) {
        if (rx == 1) {
            *x = n-1 - *x;
            *y = n-1 - *y;
        }
 
        //Swap x and y
        uint32_t t  = *x;
        *x = *y;
        *y = t;
    }
}

//convert (x,y) to d
uint32_t xy2d (uint32_t n, uint32_t x, uint32_t y) {
    uint32_t rx, ry, s, d=0;
    for (s=n/2; s>0; s/=2) {
        rx = (x & s) > 0;
        ry = (y & s) > 0;
        d += s * s * ((3 * rx) ^ ry);
        rot(s, &x, &y, rx, ry);
    }
    return d;
}
 
//convert d to (x,y)
void d2xy(uint32_t n, uint32_t d, uint32_t *x, uint32_t *y) {
    uint32_t rx, ry, s, t=d;
    *x = *y = 0;
    for (s=1; s<n; s*=2) {
        rx = 1 & (t/2);
        ry = 1 & (t ^ rx);
        rot(s, x, y, rx, ry);
        *x += s * rx;
        *y += s * ry;
        t /= 4;
    }
}

#define IMG_SIZE 16384
#define BUF_SIZE 65536
int main(int argc, char** argv){

    // Nucleotide position
    uint32_t p = 0;
    // Bufs for genome chunk an image
    byte buf[BUF_SIZE];
    byte png[BUF_SIZE*3];
    // Output tile name 
    char filename[256];
    // Size of readed chunk
    size_t s;
    
    for(int k=1; k<argc; k++) {

        printf("read file %s\n", argv[k]);
        // Read genome file
        FILE* f = fopen(argv[k], "r");
        if(!f) continue;
        
        printf("Start %u\n",p);

        // Iterate through chunks
        while((s = fread(buf, 1,  BUF_SIZE, f)) > 0) {
            // x,y of pixel
            uint32_t x, y, tx, ty, px, py;
            // hilbert pos to x, y
            d2xy(IMG_SIZE, p, &x, &y);
            // printf("%u - %u %u\n", p, x, y);
            // tile num
            tx = x/256;
            ty = y/256;
            sprintf(filename, "map/10-%u-%u.png", tx, ty);
            for(uint32_t i=0; i<BUF_SIZE; i++) {
                d2xy(IMG_SIZE, p, &x, &y);
                // pixel pos
                px = x%256;
                py = y%256;
                uint32_t j = px+py*256;
                byte n = i<s ? buf[i] : 0;
                if     (n=='A') { png[j*3+0] = 0xD9; png[j*3+1] = 0x16; png[j*3+2] = 0x0C; }
                else if(n=='G') { png[j*3+0] = 0xAF; png[j*3+1] = 0xCE; png[j*3+2] = 0x39; }
                else if(n=='C') { png[j*3+0] = 0xF6; png[j*3+1] = 0x8C; png[j*3+2] = 0x13; }
                else if(n=='T') { png[j*3+0] = 0x16; png[j*3+1] = 0x76; png[j*3+2] = 0xAE; }
                else if(n=='N') { png[j*3+0] = 0x20; png[j*3+1] = 0x20; png[j*3+2] = 0x20; }
                else            { png[j*3+0] = 0x00; png[j*3+1] = 0x00; png[j*3+2] = 0x00; }
                p++;
            }
            
            printf("%s\n", filename);
            stbi_write_png(filename, 256, 256, 3, png, 256*3);
        };
        fclose(f);
        
        printf("End %u\n",p);
        // Start file from an new 256x256 block
        // p = ((p>>16)+1)<<16;
        
    }
}

