#include "src/system/audio/waveform.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

static long quiet = 10000;
long __wrap_audio_quiet_ms(void) { return __atomic_load_n(&quiet, __ATOMIC_ACQUIRE); }
static int wait_shape(const char *path, int ms) {
    uint8_t bars[WAVEFORM_BARS];
    for (int i=0;i<ms;i+=10) {
        if (waveform_get(path,bars)==WAVEFORM_BARS) return i;
        usleep(10000);
    }
    return -1;
}
int main(int argc, char **argv) {
    assert(argc==3);
    char a[1024],b[1024],cache[1024];
    snprintf(a,sizeof(a),"%s/a.flac",argv[1]);
    snprintf(b,sizeof(b),"%s/b.flac",argv[1]);
    snprintf(cache,sizeof(cache),"%s/.local/waveform.dat",argv[1]);
    waveform_set_cache_dir(argv[1]);
    waveform_start();
    if (!strcmp(argv[2],"prepare")) {
        waveform_prepare(a,b); /* No drawing or waveform_get to trigger work. */
        uint32_t header[4]={0};
        for(int i=0;i<1000 && header[2]<2;i++) {
            usleep(10000);
            FILE *f=fopen(cache,"rb");
            if(f){if(fread(header,sizeof(header),1,f)!=1)memset(header,0,sizeof(header));fclose(f);}
        }
        assert(header[2]==2);
        assert(wait_shape(a,100)>=0);
        __atomic_store_n(&quiet,-1,__ATOMIC_RELEASE);
        waveform_prepare(b,NULL);
        int ms=wait_shape(b,1000);
        assert(ms>=0); /* Cache bypasses the audio-start quiet wait. */
        printf("PASS background current + next prefetch; warm advance %d ms\n",ms);
    } else {
        __atomic_store_n(&quiet,-1,__ATOMIC_RELEASE);
        waveform_prepare(b,NULL);
        int ms=wait_shape(b,1000);
        assert(ms>=0);
        printf("PASS persisted waveform ready during audio startup after restart: %d ms\n",ms);
    }
    return 0;
}
