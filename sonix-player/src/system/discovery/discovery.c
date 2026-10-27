#include "discovery.h"
#include "src/system/core/json.h"
#include "src/system/net/http.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#define RESPONSE_LIMIT (256 * 1024)
static bool fail(char *out, size_t n, const char *message) {
    if (out && n) snprintf(out,n,"%s",message);
    return false;
}
unsigned discovery_capabilities(void) { return DISCOVERY_SEARCH_ALBUMS|DISCOVERY_ALBUM_TRACKS|DISCOVERY_DOWNLOAD|
    DISCOVERY_ARTIST_RELEASES|DISCOVERY_PAGED_RESULTS; }
const char *discovery_provider_name(void) { return "Internet Archive · openly licensed audio"; }
static bool identifier_ok(const char *s) {
    if (!s || !*s || strlen(s)>=128 || *s=='.') return false;
    for (;*s;s++) if (!(isalnum((unsigned char)*s)||*s=='-'||*s=='_'||*s=='.')) return false;
    return true;
}
bool discovery_license_allowed(const char *url) {
    if (!url) return false;
    const char *p=NULL;
    if (!strncmp(url,"https://creativecommons.org/",28)) p=url+28;
    else if (!strncmp(url,"http://creativecommons.org/",27)) p=url+27;
    if (!p) return false;
    const char *allowed[]={"publicdomain/zero/1.0/","publicdomain/mark/1.0/",
        "licenses/by/4.0/","licenses/by/3.0/","licenses/by/2.5/","licenses/by/2.0/","licenses/by/1.0/",
        "licenses/by-sa/4.0/","licenses/by-sa/3.0/","licenses/by-sa/2.5/","licenses/by-sa/2.0/","licenses/by-sa/1.0/"};
    for (size_t i=0;i<sizeof(allowed)/sizeof(*allowed);i++) if (!strcmp(p,allowed[i])) return true;
    return false;
}
static bool field(const json_doc_t *d,int obj,const char *key,char *out,size_t n) {
    int t=json_get(d,obj,key);
    if (t>=0 && d->toks[t].type==JSON_ARRAY) t=json_at(d,t,0);
    if (t<0 || d->toks[t].type!=JSON_STRING || (size_t)(d->toks[t].end-d->toks[t].start)>=n) {
        if(n)out[0]=0;
        return false;
    }
    return json_str(d,t,out,n);
}
static long long size_field(const json_doc_t *d,int obj) {
    int t=json_get(d,obj,"size");
    if(t>=0 && d->toks[t].type==JSON_NUMBER) return json_llong(d,t,0);
    char s[32],*end;
    if(!field(d,obj,"size",s,sizeof(s)) || !isdigit((unsigned char)s[0]))return 0;
    errno=0; long long n=strtoll(s,&end,10);
    return errno || *end ? 0:n;
}
static bool truthy(const json_doc_t *d,int obj,const char *key) {
    int t=json_get(d,obj,key); if(t<0)return false;
    char s[16];
    return json_bool(d,t,false) || (json_str(d,t,s,sizeof(s)) && (!strcasecmp(s,"true")||!strcmp(s,"1")));
}
static bool hash_ok(const char *s) {
    if(strlen(s)!=32)return false;
    for(;*s;s++)if(!isxdigit((unsigned char)*s))return false;
    return true;
}
static const char *format_for(const char *filename) {
    const char *p=strrchr(filename,'.');
    if(!p)return NULL;
    const char *exts[]={"mp3","flac","ogg","opus","m4a","aac","wav","aiff","aif","wv"};
    for(size_t i=0;i<sizeof(exts)/sizeof(*exts);i++)if(!strcasecmp(p+1,exts[i]))return exts[i];
    return NULL;
}
bool discovery_item_valid(const discovery_item_t *it) {
    if(!it || it->kind!=DISCOVERY_TRACK || !it->downloadable ||
       !identifier_ok(it->id) || !discovery_license_allowed(it->license) ||
       it->bytes<=0 || it->bytes>DISCOVERY_MAX_FILE_BYTES || !hash_ok(it->md5) || !it->filename[0])return false;
    const char *fmt=format_for(it->filename);
    if(!fmt || strcmp(it->format,fmt))return false;
    char file[768],url[1024];
    if(strlen(it->filename)*3+1>sizeof(file))return false;
    http_url_encode(it->filename,file,sizeof(file));
    int n=snprintf(url,sizeof(url),"https://archive.org/download/%s/%s",it->id,file);
    return n>0 && (size_t)n<sizeof(url) && !strcmp(url,it->url);
}
static bool parse(const char *body,json_doc_t *d,char *err,size_t n) {
    if(!body || strlen(body)>=RESPONSE_LIMIT)return fail(err,n,"Provider returned an oversized response.");
    /* Bound token expansion before the shared parser allocates. Each token
     * requires an opening string/container or a primitive following a colon,
     * comma or bracket. Counting those (conservatively) caps token memory at
     * 8192 tokens = 160 KiB, plus the response and result page. */
    size_t structural=0;bool quoted=false,escaped=false;
    for(const unsigned char *p=(const unsigned char*)body;*p;p++) {
        if(quoted) {
            if(escaped)escaped=false;
            else if(*p=='\\')escaped=true;
            else if(*p=='"')quoted=false;
        } else {
            if(*p=='"'){quoted=true;structural++;}
            else if(strchr("{[:,",*p))structural++;
            if(structural>8190)return fail(err,n,"Release metadata is too complex for this device.");
        }
    }
    if(!json_parse(body,d))return fail(err,n,"Provider returned an invalid response.");
    if(json_root(d)<0 || d->toks[json_root(d)].type!=JSON_OBJECT){json_free(d);return fail(err,n,"Provider response is not an object.");}
    return true;
}
bool discovery_parse_search(const char *body,discovery_results_t *out,char *err,size_t n) {
    if(!out)return fail(err,n,"Missing result storage.");
    memset(out,0,sizeof(*out)); if(err&&n)err[0]=0;
    json_doc_t d; if(!parse(body,&d,err,n))return false;
    int docs=json_path(&d,json_root(&d),"response.docs");
    if(docs<0 || d.toks[docs].type!=JSON_ARRAY){json_free(&d);return fail(err,n,"Search is unavailable. Try again later.");}
    int count=json_len(&d,docs);
    for(int i=0;i<count;i++) {
        discovery_item_t item={0}; int obj=json_at(&d,docs,i);
        if(!field(&d,obj,"identifier",item.id,sizeof(item.id))||!identifier_ok(item.id) ||
           !field(&d,obj,"licenseurl",item.license,sizeof(item.license))||!discovery_license_allowed(item.license))continue;
        field(&d,obj,"title",item.title,sizeof(item.title));
        field(&d,obj,"creator",item.artist,sizeof(item.artist));
        if(!item.title[0])snprintf(item.title,sizeof(item.title),"%s",item.id);
        snprintf(item.album,sizeof(item.album),"%s",item.title);
        snprintf(item.artwork,sizeof(item.artwork),"https://archive.org/services/img/%s",item.id);
        item.kind=DISCOVERY_ALBUM;
        if(out->count==DISCOVERY_MAX_RESULTS){out->truncated=true;break;}
        out->items[out->count++]=item;
    }
    long long start=json_llong(&d,json_path(&d,json_root(&d),"response.start"),0);
    long long total=json_llong(&d,json_path(&d,json_root(&d),"response.numFound"),count);
    out->truncated=out->truncated || (start>=0 && total>start && total-start>count);
    json_free(&d); return true;
}
bool discovery_parse_album_page(const char *id,const char *body,unsigned page,discovery_results_t *out,char *err,size_t n) {
    if(!out || !identifier_ok(id))return fail(err,n,"Invalid release identifier.");
    memset(out,0,sizeof(*out)); if(err&&n)err[0]=0;
    if(page<1 || page>DISCOVERY_MAX_PAGES)return fail(err,n,"Invalid source page.");
    json_doc_t d; if(!parse(body,&d,err,n))return false;
    int root=json_root(&d), meta=json_get(&d,root,"metadata"), files=json_get(&d,root,"files");
    discovery_item_t base={0};
    char type[32]; field(&d,meta,"mediatype",type,sizeof(type));
    if(strcmp(type,"audio") || truthy(&d,meta,"is_dark") || truthy(&d,meta,"access-restricted-item") || truthy(&d,meta,"nodownload") ||
       truthy(&d,root,"is_dark") || !field(&d,meta,"licenseurl",base.license,sizeof(base.license)) || !discovery_license_allowed(base.license)) {
        json_free(&d);return fail(err,n,"This release does not offer supported openly licensed downloads.");
    }
    if(files<0 || d.toks[files].type!=JSON_ARRAY){json_free(&d);return fail(err,n,"Release files are unavailable.");}
    snprintf(base.id,sizeof(base.id),"%s",id);base.kind=DISCOVERY_TRACK;
    field(&d,meta,"title",base.album,sizeof(base.album));field(&d,meta,"creator",base.artist,sizeof(base.artist));
    snprintf(base.artwork,sizeof(base.artwork),"https://archive.org/services/img/%s",id);
    unsigned accepted=0, skip=(page-1)*DISCOVERY_MAX_RESULTS;
    for(int i=0;i<json_len(&d,files);i++) {
        discovery_item_t item=base; int f=json_at(&d,files,i);
        if(truthy(&d,f,"private") || !field(&d,f,"name",item.filename,sizeof(item.filename)))continue;
        const char *format=format_for(item.filename);if(!format)continue;
        snprintf(item.format,sizeof(item.format),"%s",format);item.bytes=size_field(&d,f);
        if(item.bytes<=0 || item.bytes>DISCOVERY_MAX_FILE_BYTES || !field(&d,f,"md5",item.md5,sizeof(item.md5)) || !hash_ok(item.md5))continue;
        for(char *p=item.md5;*p;p++)*p=(char)tolower((unsigned char)*p);
        field(&d,f,"title",item.title,sizeof(item.title));
        if(!item.title[0])snprintf(item.title,sizeof(item.title),"%.191s",item.filename);
        char artist[128];if(field(&d,f,"artist",artist,sizeof(artist)))snprintf(item.artist,sizeof(item.artist),"%s",artist);
        char file[768];http_url_encode(item.filename,file,sizeof(file));
        int wrote=snprintf(item.url,sizeof(item.url),"https://archive.org/download/%s/%s",id,file);
        if(wrote<=0 || (size_t)wrote>=sizeof(item.url))continue;
        item.downloadable=true;
        if(accepted++<skip)continue;
        if(out->count==DISCOVERY_MAX_RESULTS){out->truncated=true;break;}
        out->items[out->count++]=item;
    }
    json_free(&d);
    return out->count>0 || fail(err,n,"No supported downloadable audio with integrity metadata was found.");
}
bool discovery_parse_album(const char *id,const char *body,discovery_results_t *out,char *err,size_t n) {
    return discovery_parse_album_page(id,body,1,out,err,n);
}
static bool get(const char *url,char **body,char *err,size_t n) {
    size_t len=0;
    const http_req_t request={.https_only=true};
    if(!http_request(url,&request,body,&len,RESPONSE_LIMIT,15,NULL)) return fail(err,n,http_last_error()?http_last_error():"Network unavailable. Check Wi-Fi and retry.");
    if(len>=RESPONSE_LIMIT){free(*body);*body=NULL;return fail(err,n,"Release is too large for this device. Choose a smaller release.");}
    return true;
}
bool discovery_search_page(const char *query,bool artist_only,unsigned page,discovery_results_t *out,char *err,size_t n) {
    if(out)memset(out,0,sizeof(*out));
    if(!out || page<1 || page>DISCOVERY_MAX_PAGES)return fail(err,n,"Invalid search page.");
    if(!query || !*query || strlen(query)>127)return fail(err,n,"Enter a search of 1 to 127 bytes.");
    /* Quote a literal phrase; never let input inject a license-filter expression. */
    char literal[255];size_t j=0;
    for(const unsigned char *p=(const unsigned char*)query;*p;p++) {
        if(*p<32)continue;
        if(*p=='"'||*p=='\\')literal[j++]='\\';
        literal[j++]=(char)*p;
    } literal[j]=0;
    if(!j)return fail(err,n,"Enter an artist, album or track name.");
    char q[640],encoded[1920],url[2300];
    if(artist_only)snprintf(q,sizeof(q),"mediatype:audio AND licenseurl:* AND creator:\"%s\"",literal);
    else snprintf(q,sizeof(q),"mediatype:audio AND licenseurl:* AND (title:\"%s\" OR creator:\"%s\")",literal,literal);
    http_url_encode(q,encoded,sizeof(encoded));
    snprintf(url,sizeof(url),"https://archive.org/advancedsearch.php?q=%s&fl[]=identifier&fl[]=title&fl[]=creator&fl[]=licenseurl&rows=24&page=%u&sort[]=identifier%%20asc&output=json",encoded,page);
    char *body=NULL;if(!get(url,&body,err,n))return false;
    bool ok=discovery_parse_search(body,out,err,n);free(body);return ok;
}
bool discovery_search(const char *query,discovery_results_t *out,char *err,size_t n) {
    return discovery_search_page(query,false,1,out,err,n);
}
bool discovery_album_page(const char *id,unsigned page,discovery_results_t *out,char *err,size_t n) {
    if(out)memset(out,0,sizeof(*out));
    if(!out || !identifier_ok(id) || page<1 || page>DISCOVERY_MAX_PAGES)return fail(err,n,"Invalid release or page.");
    char url[256];snprintf(url,sizeof(url),"https://archive.org/metadata/%s",id);
    char *body=NULL;if(!get(url,&body,err,n))return false;
    bool ok=discovery_parse_album_page(id,body,page,out,err,n);free(body);return ok;
}
bool discovery_album(const char *id,discovery_results_t *out,char *err,size_t n) {
    return discovery_album_page(id,1,out,err,n);
}
