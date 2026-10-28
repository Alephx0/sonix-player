/* Native metadata adapter. Source audit: SpotiFLAC's songlink.go separates
 * catalog identity from downloadable audio; see docs/custom-firmware. */
#include "catalog.h"
#include "src/system/core/json.h"
#include "src/system/net/http.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LIMIT (256*1024)
static bool fail(char *error,size_t size,const char *text) {
    if(error && size)snprintf(error,size,"%s",text);
    return false;
}
static bool digits(const char *id) {
    if(!id || !*id || strlen(id)>18)return false;
    for(const unsigned char *p=(const unsigned char *)id;*p;p++)if(*p<'0' || *p>'9')return false;
    return strspn(id,"0")!=strlen(id);
}
static bool field(const json_doc_t *d,int obj,const char *key,char *out,size_t size) {
    int t=json_get(d,obj,key);
    if(t<0 || d->toks[t].type!=JSON_STRING || (size_t)(d->toks[t].end-d->toks[t].start)>=size)return false;
    return json_str(d,t,out,size);
}
static bool id_field(const json_doc_t *d,int obj,char *out,size_t size) {
    int t=json_get(d,obj,"id");
    if(t<0 || (d->toks[t].type!=JSON_NUMBER && d->toks[t].type!=JSON_STRING))return false;
    if(d->toks[t].type==JSON_NUMBER) {
        size_t n=(size_t)(d->toks[t].end-d->toks[t].start);
        if(n>=size)return false;
        memcpy(out,d->text+d->toks[t].start,n);out[n]=0;
    } else if(!json_str(d,t,out,size))return false;
    if(!digits(out)){out[0]=0;return false;}return true;
}
static bool parse(const char *body,json_doc_t *d,char *error,size_t size) {
    if(!body || strlen(body)>=LIMIT)return fail(error,size,"Catalog response exceeds the device limit.");
    unsigned tokens=0;bool quote=false,escape=false;
    for(const unsigned char *p=(const unsigned char *)body;*p;p++) {
        if(quote){if(escape)escape=false;else if(*p=='\\')escape=true;else if(*p=='"')quote=false;}
        else {if(*p=='"'){quote=true;tokens++;}else if(strchr("{[:,",*p))tokens++;}
        if(tokens>8190)return fail(error,size,"Catalog response is too complex.");
    }
    if(!json_parse(body,d))return fail(error,size,"Invalid catalog response.");
    int root=json_root(d);
    if(root<0 || d->toks[root].type!=JSON_OBJECT || json_get(d,root,"error")>=0) {
        json_free(d);return fail(error,size,"Catalog request unavailable. Try again later.");
    }
    return true;
}
static bool entry(const json_doc_t *d,int obj,discovery_kind_t kind,discovery_item_t *it) {
    memset(it,0,sizeof(*it));it->catalog=1;it->kind=kind;
    char id[32];if(!id_field(d,obj,id,sizeof(id)) || !field(d,obj,"title",it->title,sizeof(it->title)))return false;
    snprintf(it->id,sizeof(it->id),"deezer:%s:%s",kind==DISCOVERY_ALBUM?"album":"track",id);
    snprintf(it->url,sizeof(it->url),"https://www.deezer.com/%s/%s",kind==DISCOVERY_ALBUM?"album":"track",id);
    int artist=json_get(d,obj,"artist");field(d,artist,"name",it->artist,sizeof(it->artist));id_field(d,artist,it->artist_id,sizeof(it->artist_id));
    if(kind==DISCOVERY_ALBUM) {
        snprintf(it->album,sizeof(it->album),"%s",it->title);snprintf(it->album_id,sizeof(it->album_id),"%s",id);
        field(d,obj,"cover_medium",it->artwork,sizeof(it->artwork));
    } else {
        int album=json_get(d,obj,"album");field(d,album,"title",it->album,sizeof(it->album));id_field(d,album,it->album_id,sizeof(it->album_id));
        field(d,album,"cover_medium",it->artwork,sizeof(it->artwork));
        long duration=json_obj_long(d,obj,"duration",0);if(duration>0 && duration<86400)it->duration=(unsigned)duration;
        char isrc[32]={0};field(d,obj,"isrc",isrc,sizeof(isrc));
        bool valid=strlen(isrc)==12;
        for(int i=0;valid && i<12;i++) {
            unsigned char c=(unsigned char)toupper((unsigned char)isrc[i]);
            valid=i<2?(c>='A'&&c<='Z'):i<5?((c>='A'&&c<='Z')||(c>='0'&&c<='9')):(c>='0'&&c<='9');
            isrc[i]=(char)c;
        }
        if(valid)snprintf(it->isrc,sizeof(it->isrc),"%.12s",isrc);
    }
    if(strncmp(it->artwork,"https://cdn-images.dzcdn.net/images/cover/",42) &&
       strncmp(it->artwork,"https://e-cdns-images.dzcdn.net/images/cover/",45))it->artwork[0]=0;
    /* Never interpret playable previews, readable=true or provider links as
     * full-recording download authorization. The download validator rejects it. */
    return true;
}
bool catalog_parse_deezer(const char *body,discovery_kind_t kind,discovery_results_t *out,char *error,size_t size) {
    if(!out)return fail(error,size,"Missing catalog result storage.");
    memset(out,0,sizeof(*out));if(error && size)error[0]=0;
    json_doc_t d;if(!parse(body,&d,error,size))return false;
    int root=json_root(&d),array=json_get(&d,root,"data");
    if(array<0) {
        bool ok=entry(&d,root,kind,&out->items[0]);out->count=ok?1:0;json_free(&d);
        return ok?true:fail(error,size,"Catalog entry has no usable identifier or title.");
    }
    if(d.toks[array].type!=JSON_ARRAY){json_free(&d);return fail(error,size,"Invalid catalog list.");}
    int length=json_len(&d,array);
    for(int i=0;i<length && out->count<DISCOVERY_MAX_RESULTS;i++)
        if(entry(&d,json_at(&d,array,i),kind,&out->items[out->count]))out->count++;
    int next=json_get(&d,root,"next");
    out->truncated=length>DISCOVERY_MAX_RESULTS || (next>=0 && d.toks[next].type==JSON_STRING && d.toks[next].end>d.toks[next].start);
    json_free(&d);return true;
}
static bool fetch(const char *path,char **body,char *error,size_t size) {
    char url[2048];if(snprintf(url,sizeof(url),"https://api.deezer.com/%s",path)>=(int)sizeof(url))return fail(error,size,"Catalog request is too long.");
    http_req_t request={0};request.https_only=true;int status=0;size_t bytes=0;
    if(!http_request(url,&request,body,&bytes,LIMIT,12,&status))return fail(error,size,status==429?"Catalog rate limit. Wait before searching again.":"Could not reach Deezer catalog.");
    if(status!=200 || bytes>=LIMIT){free(*body);*body=NULL;return fail(error,size,"Catalog response unavailable or too large.");}
    return true;
}
static bool page_valid(unsigned page) {return page>0 && page<=DISCOVERY_MAX_PAGES;}
static bool query(const char *text,bool artist,unsigned page,bool albums,discovery_results_t *out,char *error,size_t size) {
    if(!out)return false;
    memset(out,0,sizeof(*out));
    if(!text || !*text || strlen(text)>255 || !page_valid(page))return fail(error,size,"Invalid catalog query.");
    char path[1600],*body=NULL;discovery_kind_t kind=albums?DISCOVERY_ALBUM:DISCOVERY_TRACK;
    if(!strncmp(text,"deezer:track:",13)) {
        if(!digits(text+13))return fail(error,size,"Invalid track identifier.");
        snprintf(path,sizeof(path),"track/%s",text+13);kind=DISCOVERY_TRACK;
    } else if(artist) {
        const char *id=!strncmp(text,"deezer:artist:",14)?text+14:text;
        if(!digits(id))return fail(error,size,"Artist browsing requires a catalog artist ID.");
        snprintf(path,sizeof(path),"artist/%s/albums?index=%u&limit=%d",id,(page-1)*DISCOVERY_MAX_RESULTS,DISCOVERY_MAX_RESULTS);kind=DISCOVERY_ALBUM;
    } else {
        char escaped[768];http_url_encode(text,escaped,sizeof(escaped));
        snprintf(path,sizeof(path),"search%s?q=%s&index=%u&limit=%d",albums?"/album":"",escaped,(page-1)*DISCOVERY_MAX_RESULTS,DISCOVERY_MAX_RESULTS);
    }
    if(!fetch(path,&body,error,size))return false;
    bool ok=catalog_parse_deezer(body,kind,out,error,size);free(body);
    if(ok && !strncmp(text,"deezer:track:",13) && (out->count!=1 || strcmp(out->items[0].id,text))) {
        memset(out,0,sizeof(*out));return fail(error,size,"Catalog returned a different recording.");
    }
    return ok;
}
static bool tracks(const char *q,bool artist,unsigned page,discovery_results_t *out,char *e,size_t n){return query(q,artist,page,false,out,e,n);}
static bool albums(const char *q,bool artist,unsigned page,discovery_results_t *out,char *e,size_t n){return query(q,artist,page,true,out,e,n);}
static bool album(const char *key,unsigned page,discovery_results_t *out,char *error,size_t size) {
    if(!out)return false;
    memset(out,0,sizeof(*out));
    const char *id=key && !strncmp(key,"deezer:album:",13)?key+13:NULL;
    if(!digits(id) || !page_valid(page))return fail(error,size,"Invalid release identifier.");
    char path[128],*body=NULL;snprintf(path,sizeof(path),"album/%s",id);
    if(!fetch(path,&body,error,size))return false;
    json_doc_t doc;if(!parse(body,&doc,error,size)){free(body);return false;}
    discovery_item_t metadata;bool valid=entry(&doc,json_root(&doc),DISCOVERY_ALBUM,&metadata) && !strcmp(metadata.album_id,id);json_free(&doc);free(body);
    if(!valid)return fail(error,size,"Album metadata is incomplete.");
    snprintf(path,sizeof(path),"album/%s/tracks?index=%u&limit=%d",id,(page-1)*DISCOVERY_MAX_RESULTS,DISCOVERY_MAX_RESULTS);
    if(!fetch(path,&body,error,size))return false;
    bool ok=catalog_parse_deezer(body,DISCOVERY_TRACK,out,error,size);free(body);
    if(ok)for(int i=0;i<out->count;i++) {
        discovery_item_t *it=&out->items[i];snprintf(it->album,sizeof(it->album),"%s",metadata.title);
        snprintf(it->album_id,sizeof(it->album_id),"%s",metadata.album_id);
        snprintf(it->artwork,sizeof(it->artwork),"%s",metadata.artwork);
        if(!it->artist[0])snprintf(it->artist,sizeof(it->artist),"%s",metadata.artist);
        if(!it->artist_id[0])snprintf(it->artist_id,sizeof(it->artist_id),"%s",metadata.artist_id);
    }
    return ok;
}
const discovery_provider_t *catalog_provider(unsigned selection) {
    static const discovery_provider_t track={1,"deezer","Deezer catalog",DISCOVERY_SEARCH_TRACKS|DISCOVERY_ALBUM_TRACKS|DISCOVERY_ARTIST_RELEASES|DISCOVERY_PAGED_RESULTS,tracks,album};
    static const discovery_provider_t release={1,"deezer","Deezer catalog",DISCOVERY_SEARCH_ALBUMS|DISCOVERY_ALBUM_TRACKS|DISCOVERY_ARTIST_RELEASES|DISCOVERY_PAGED_RESULTS,albums,album};
    return selection==1?&track:selection==2?&release:discovery_provider();
}
bool catalog_link(const char *url,char *key,size_t size,bool *is_album) {
    if(key && size)key[0]=0;
    if(is_album)*is_album=false;
    if(!url || !key || !size || !is_album)return false;
    const char *p=NULL;
    if(!strncmp(url,"https://www.deezer.com/",23))p=url+23;
    else if(!strncmp(url,"https://deezer.com/",19))p=url+19;
    else return false;
    for(const unsigned char *c=(const unsigned char *)url;*c;c++)if(*c<=32 || *c==127)return false;
    if(strlen(p)>3 && isalpha((unsigned char)p[0]) && isalpha((unsigned char)p[1]) && p[2]=='/')p+=3;
    bool release=!strncmp(p,"album/",6);
    if(!release && strncmp(p,"track/",6))return false;
    p+=6;
    size_t len=strcspn(p,"/?#");char id[32];if(!len || len>=sizeof(id))return false;
    memcpy(id,p,len);id[len]=0;if(!digits(id))return false;
    const char *tail=p+len;if(*tail=='/' && tail[1] && tail[1]!='?' && tail[1]!='#')return false;
    int wrote=snprintf(key,size,"deezer:%s:%s",release?"album":"track",id);
    if(wrote<0 || (size_t)wrote>=size){key[0]=0;return false;}*is_album=release;return true;
}
