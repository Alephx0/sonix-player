#include "src/system/image/webp_scaled.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static unsigned char *read_file(const char *path,size_t *length) {
    FILE *f=fopen(path,"rb");assert(f);assert(!fseek(f,0,SEEK_END));long n=ftell(f);assert(n>0);rewind(f);
    unsigned char *data=malloc((size_t)n);assert(data && fread(data,1,(size_t)n,f)==(size_t)n);fclose(f);*length=(size_t)n;return data;
}
int main(int argc,char **argv) {
    assert(argc==5);size_t length;int w=0,h=0;
    unsigned char *data=read_file(argv[1],&length);
    unsigned char *pixels=webp_scaled_decode(data,length,220,&w,&h);
    assert(pixels && w==220 && h==110);
    assert(pixels[0]==255 && pixels[1]==0 && pixels[2]==0);free(pixels);
    assert(!webp_scaled_decode(data,12,220,&w,&h));
    assert(!webp_scaled_decode(data,length,0,&w,&h));
    assert(!webp_scaled_decode(data,length,1025,&w,&h));free(data);
    data=read_file(argv[2],&length);pixels=webp_scaled_decode(data,length,58,&w,&h);
    assert(pixels && w==58 && h==58 && pixels[2]>240);free(pixels);free(data);
    for(int i=3;i<5;i++){data=read_file(argv[i],&length);assert(!webp_scaled_decode(data,length,58,&w,&h));free(data);}
    puts("PASS WebP lossy/lossless colors, scaled dimensions, truncated/oversized/animated rejection");
}
