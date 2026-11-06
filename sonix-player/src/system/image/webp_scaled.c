#include "webp_scaled.h"
#include <webp/decode.h>
#include <stdlib.h>

uint8_t *webp_scaled_decode(const uint8_t *data,size_t size,int max_box,int *w,int *h) {
    if(!data || !w || !h || !size || size>4*1024*1024 || max_box<1 || max_box>1024)return NULL;
    WebPDecoderConfig cfg;
    if(!WebPInitDecoderConfig(&cfg) || WebPGetFeatures(data,size,&cfg.input)!=VP8_STATUS_OK)return NULL;
    /* Reject animation and large source surfaces before decoder allocation.
     * Lossless WebP can retain full source pixels even with scaled output. */
    if(cfg.input.has_animation || cfg.input.width<1 || cfg.input.height<1 ||
       cfg.input.width>2048 || cfg.input.height>2048 ||
       (size_t)cfg.input.width*cfg.input.height>1024*1024)return NULL;
    int ow=cfg.input.width,oh=cfg.input.height;
    if(ow>max_box || oh>max_box) {
        if(ow>=oh){oh=(int)((long long)oh*max_box/ow);ow=max_box;}
        else {ow=(int)((long long)ow*max_box/oh);oh=max_box;}
        if(ow<1)ow=1;
        if(oh<1)oh=1;
        cfg.options.use_scaling=1;cfg.options.scaled_width=ow;cfg.options.scaled_height=oh;
    }
    size_t bytes=(size_t)ow*oh*3;
    uint8_t *pixels=malloc(bytes);if(!pixels)return NULL;
    cfg.output.colorspace=MODE_RGB;cfg.output.is_external_memory=1;
    cfg.output.u.RGBA.rgba=pixels;cfg.output.u.RGBA.stride=ow*3;cfg.output.u.RGBA.size=bytes;
    VP8StatusCode status=WebPDecode(data,size,&cfg);
    WebPFreeDecBuffer(&cfg.output);
    if(status!=VP8_STATUS_OK){free(pixels);return NULL;}
    *w=ow;*h=oh;return pixels;
}
