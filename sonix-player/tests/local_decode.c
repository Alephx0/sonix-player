#include "src/system/decode/decode.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv) {
    assert(argc==2);
    const char *names[]={"lossless.flac","cbr.mp3","cbr-no-header.mp3","vbr.mp3"};
    short pcm[8192];
    for(int i=0;i<4;i++) {
        char path[1024];snprintf(path,sizeof(path),"%s/%s",argv[1],names[i]);
        decoder_t *d=decoder_open(path,decode_detect_format(path));assert(d);
        int rate=decoder_sample_rate(d),channels=decoder_channels(d);
        assert(rate==48000 && channels==2);
        uint64_t length=decoder_total_pcm_frames(d);
        assert(length>3*rate && length<5*rate);
        assert(decoder_read_pcm_frames_s16(d,1024,pcm)==1024);
        assert(decoder_seek_to_frame(d,2*rate));
        assert(decoder_read_pcm_frames_s16(d,1024,pcm)==1024);
        assert(decoder_seek_to_frame(d,0));
        uint64_t total=0,got;
        while((got=decoder_read_pcm_frames_s16(d,4096,pcm))) {
            total+=got;assert(total<6*rate);
        }
        assert(total>3*rate && total<5*rate);
        assert(decoder_read_pcm_frames_s16(d,1024,pcm)==0);
        decoder_close(d);
    }
    puts("PASS local decoding: FLAC, CBR/Xing, CBR without Xing and VBR MP3; duration, forward/back seek, EOF");
}
