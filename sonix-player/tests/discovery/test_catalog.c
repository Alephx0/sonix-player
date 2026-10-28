#define _GNU_SOURCE 1
#include "src/system/discovery/catalog.h"
#include "src/system/net/http.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *reply;
static char requested[2048];
static int response_status=200;
#ifndef CATALOG_LIVE
bool http_request(const char *url,const http_req_t *r,char **out,size_t *len,size_t limit,int timeout,int *status) {
    (void)timeout;assert(r && r->https_only && reply && strlen(reply)<limit);
    snprintf(requested,sizeof(requested),"%s",url);*out=strdup(reply);*len=strlen(reply);*status=response_status;return true;
}
const char *http_last_error(void){return "test";}
char *http_url_encode(const char *input,char *out,size_t size) {
    static const char hex[]="0123456789ABCDEF";size_t used=0;
    for(const unsigned char *p=(const unsigned char *)input;*p && used+4<size;p++) {
        if((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')||(*p>='0'&&*p<='9'))out[used++]=*p;
        else {out[used++]='%';out[used++]=hex[*p>>4];out[used++]=hex[*p&15];}
    }out[used]=0;return out;
}
#else
const char *tr(const char *s){return s;}
const char *config_get(const char *s,const char *k,const char *f){(void)s;(void)k;return f;}
#endif
static const char *track="{\"id\":3171002981,\"title\":\"NUEVAYoL\",\"duration\":183,\"isrc\":\"QMFMF2447055\",\"readable\":true,\"artist\":{\"id\":10583405,\"name\":\"Bad Bunny\"},\"album\":{\"id\":689224201,\"title\":\"Release\",\"cover_medium\":\"https://cdn-images.dzcdn.net/images/cover/test/250x250.jpg\"}}";
int main(void) {
    discovery_results_t *out=calloc(1,sizeof(*out));assert(out);char error[192],key[128];bool album;
    assert(catalog_link("https://www.deezer.com/en/track/3171002981?utm_source=test",key,sizeof(key),&album));
    assert(!album && !strcmp(key,"deezer:track:3171002981"));
    assert(catalog_link("https://deezer.com/album/123/",key,sizeof(key),&album)&&album);
    const char *bad[]={"http://www.deezer.com/album/123","https://www.deezer.com.evil/album/123","https://www.deezer.com@evil/album/123","https://www.deezer.com/album/../123","https://www.deezer.com/album/12/extra","https://www.deezer.com/track/1e3","https://www.deezer.com/track/0","https://www.deezer.com/track/123\n"};
    for(size_t i=0;i<sizeof(bad)/sizeof(*bad);i++)assert(!catalog_link(bad[i],key,sizeof(key),&album));
    assert(!catalog_link("https://deezer.com/track/123",key,8,&album));
    assert(catalog_parse_deezer(track,DISCOVERY_TRACK,out,error,sizeof(error)) && out->count==1);
    discovery_item_t *it=&out->items[0];assert(it->catalog==1 && !strcmp(it->isrc,"QMFMF2447055") && it->duration==183);
    assert(!strcmp(it->artist_id,"10583405") && !strcmp(it->album_id,"689224201") && it->artwork[0]);
    assert(!it->downloadable && !discovery_item_valid(it));
    assert(!catalog_parse_deezer("{\"error\":{\"code\":4}}",DISCOVERY_TRACK,out,error,sizeof(error)));
    assert(!catalog_parse_deezer("[]",DISCOVERY_TRACK,out,error,sizeof(error)));
    assert(!catalog_parse_deezer("{\"data\":{}}",DISCOVERY_TRACK,out,error,sizeof(error)));
    assert(catalog_parse_deezer("{\"data\":[{\"id\":1e3,\"title\":\"Bad\"},{\"id\":\"../../x\",\"title\":\"Bad\"}]}",DISCOVERY_TRACK,out,error,sizeof(error)) && out->count==0);
    assert(catalog_parse_deezer("{\"id\":1,\"title\":\"Test\",\"isrc\":\"BAD\",\"album\":{\"cover_medium\":\"http://127.0.0.1/private\"}}",DISCOVERY_TRACK,out,error,sizeof(error)));
    assert(!out->items[0].isrc[0] && !out->items[0].artwork[0]);
    char *large=malloc(256*1024+1);assert(large);memset(large,'x',256*1024);large[256*1024]=0;
    assert(!catalog_parse_deezer(large,DISCOVERY_TRACK,out,error,sizeof(error)));free(large);
#ifndef CATALOG_LIVE
    reply="{\"data\":[],\"next\":\"http://127.0.0.1/private\"}";
    assert(catalog_provider(1)->search("Bad Bunny",false,2,out,error,sizeof(error)) && out->truncated);
    assert(strstr(requested,"https://api.deezer.com/search?q=Bad%20Bunny&index=24&limit=24"));
    assert(catalog_provider(2)->search("Night",false,1,out,error,sizeof(error)));assert(strstr(requested,"/search/album?"));
    assert(catalog_provider(1)->search("deezer:artist:123",true,1,out,error,sizeof(error)));assert(strstr(requested,"/artist/123/albums?"));
    assert(!catalog_provider(1)->search("../123",true,1,out,error,sizeof(error)));
    assert(!catalog_provider(1)->search("test",false,0,out,error,sizeof(error)));
    reply=track;assert(catalog_provider(1)->search("deezer:track:3171002981",false,1,out,error,sizeof(error)));
    assert(!strcmp(requested,"https://api.deezer.com/track/3171002981"));
    assert(!catalog_provider(1)->search("deezer:track:999",false,1,out,error,sizeof(error)) && out->count==0);
    response_status=401;assert(!catalog_provider(1)->search("test",false,1,out,error,sizeof(error)));
#else
    assert(catalog_provider(1)->search("Bad Bunny",false,1,out,error,sizeof(error)) && out->count>0);
    printf("PASS live track search: %d entries; first %s\n",out->count,out->items[0].title);
    char id[128];snprintf(id,sizeof(id),"deezer:album:%s",out->items[0].album_id);
    assert(catalog_provider(1)->album(id,1,out,error,sizeof(error)) && out->count>0);
    printf("PASS live release: %s / %d tracks\n",out->items[0].album,out->count);
    snprintf(id,sizeof(id),"deezer:artist:%s",out->items[0].artist_id);
    assert(catalog_provider(1)->search(id,true,1,out,error,sizeof(error)) && out->count>0);
    assert(catalog_provider(2)->search("Debi Tirar",false,1,out,error,sizeof(error)) && out->count>0);
    puts("PASS live artist releases and album search; no audio transferred");
#endif
    free(out);puts("PASS catalog IDs/ISRC, URL boundaries, non-downloadable entries, paging, hostile responses and bounded parsing");return 0;
}
