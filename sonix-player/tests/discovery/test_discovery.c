#define _GNU_SOURCE 1
#include "src/system/discovery/discovery.h"
#include "src/system/discovery/download_queue.h"
#include "src/system/net/http.h"
#include "src/system/core/md5.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
static atomic_int opens,mode,woken;
int __real_fstatvfs(int,struct statvfs*);
int __wrap_fstatvfs(int fd,struct statvfs *out){int rc=__real_fstatvfs(fd,out);if(!rc&&atomic_load(&mode)==3)out->f_bavail=0;return rc;}
int __real_fstatvfs64(int,struct statvfs64*);
int __wrap_fstatvfs64(int fd,struct statvfs64 *out){int rc=__real_fstatvfs64(fd,out);if(!rc&&atomic_load(&mode)==3)out->f_bavail=0;return rc;}
static long long position,expected;
static char payload[65536];
static const char *catalog_fixture;
static char catalog_url[2400];
bool http_request(const char *u,const http_req_t*r,char **o,size_t*l,size_t lim,int t,int*s){
    (void)t;(void)s;
    if(!catalog_fixture)return false;
    assert(r && r->https_only && strlen(catalog_fixture)<lim);
    snprintf(catalog_url,sizeof(catalog_url),"%s",u);
    *o=strdup(catalog_fixture);assert(*o);if(l)*l=strlen(*o);return true;
}
const char *http_last_error(void){return "mock network error";}
char *http_url_encode(const char *in,char*out,size_t size){const char *h="0123456789ABCDEF";size_t n=0;while(*in&&n+4<size){unsigned char c=*in++;if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||strchr("-_.~",c))out[n++]=c;else{out[n++]='%';out[n++]=h[c>>4];out[n++]=h[c&15];}}out[n]=0;return out;}
bool http_download_open(http_stream_t *st,const char*url,long long off,long long total,int timeout,long long *accepted){(void)url;(void)timeout;memset(st,0,sizeof(*st));st->fd=42;position=off;expected=total;*accepted=off;atomic_store(&woken,0);atomic_fetch_add(&opens,1);return true;}
int http_stream_read(http_stream_t *s,void *b,int n){(void)s;usleep(10000);if(atomic_load(&woken))return -1;int m=atomic_load(&mode);if(m==1&&atomic_load(&opens)==1&&position>=16384)return 0;if(m==2)usleep(50000);if(position>=expected)return 0;if(n>4096)n=4096;if(n>expected-position)n=(int)(expected-position);memcpy(b,payload+position,(size_t)n);position+=n;return n;}
void http_stream_wake(http_stream_t *s){(void)s;atomic_store(&woken,1);}
void http_stream_close(http_stream_t *s){s->fd=-1;}
static discovery_item_t item(const char *name){discovery_item_t it={0};it.kind=DISCOVERY_TRACK;it.downloadable=true;it.bytes=sizeof(payload);strcpy(it.id,"test-release");strcpy(it.title,name);strcpy(it.artist,"Test Artist");strcpy(it.album,"Test Release");snprintf(it.filename,sizeof(it.filename),"%s.mp3",name);strcpy(it.format,"mp3");strcpy(it.license,"https://creativecommons.org/licenses/by/4.0/");char enc[768];http_url_encode(it.filename,enc,sizeof(enc));snprintf(it.url,sizeof(it.url),"https://archive.org/download/%s/%s",it.id,enc);md5_hex(payload,sizeof(payload),it.md5);return it;}
static download_job_t snapshot(unsigned id){download_job_t j[8];int n=download_snapshot(j,8);for(int i=0;i<n;i++)if(j[i].id==id)return j[i];assert(!"missing job");return j[0];}
static download_job_t wait_terminal(unsigned id){for(int i=0;i<1000;i++){download_job_t j=snapshot(id);if(j.state==DOWNLOAD_DONE||j.state==DOWNLOAD_FAILED||j.state==DOWNLOAD_CANCELLED)return j;usleep(10000);}assert(!"timed out");return snapshot(id);}
static void wait_progress(unsigned id){for(int i=0;i<400;i++){if(snapshot(id).done>0)return;usleep(10000);}assert(!"no progress");}
static void test_md5(void){const char *vectors[]={"","a","abc","message digest","abcdefghijklmnopqrstuvwxyz"};for(int i=0;i<5;i++){char a[33],b[33];md5_hex(vectors[i],strlen(vectors[i]),a);md5_ctx c;md5_init(&c);for(size_t j=0;j<strlen(vectors[i]);j++)md5_update(&c,vectors[i]+j,1);md5_final_hex(&c,b);assert(!strcmp(a,b));}char known[33];md5_ctx c;md5_init(&c);md5_update(&c,"abc",3);md5_final_hex(&c,known);assert(!strcmp(known,"900150983cd24fb0d6963f7d28e17f72"));for(int len=55;len<140;len++){char a[33],b[33];md5_hex(payload,(size_t)len,a);md5_init(&c);md5_update(&c,payload,30);md5_update(&c,payload+30,(size_t)len-30);md5_final_hex(&c,b);assert(!strcmp(a,b));}}
static void test_provider(void){char err[192];discovery_results_t *r=calloc(1,sizeof(*r));assert(r);assert(discovery_license_allowed("https://creativecommons.org/publicdomain/zero/1.0/"));assert(!discovery_license_allowed("https://creativecommons.org.evil/licenses/by/4.0/"));assert(!discovery_license_allowed("https://creativecommons.org/licenses/by/4.0/extra"));assert(!discovery_license_allowed("https://creativecommons.org/licenses/by-nc/4.0/"));assert(!discovery_parse_search("[]",r,err,sizeof(err)));assert(!discovery_parse_search("{",r,err,sizeof(err)));assert(discovery_parse_search("{\"response\":{\"docs\":[{\"identifier\":\"release\",\"title\":\"Music\",\"licenseurl\":\"https://creativecommons.org/licenses/by/4.0/\"},{\"identifier\":\"evil\",\"licenseurl\":\"https://creativecommons.org.evil/licenses/by/4.0/\"}]}}",r,err,sizeof(err)));assert(r->count==1&&r->items[0].kind==DISCOVERY_ALBUM);
const char *body="{\"metadata\":{\"mediatype\":\"audio\",\"title\":\"Release\",\"creator\":[\"Artist\"],\"licenseurl\":\"https://creativecommons.org/licenses/by/4.0/\"},\"files\":[{\"name\":\"track.mp3\",\"size\":\"123\",\"md5\":\"900150983cd24fb0d6963f7d28e17f72\"},{\"name\":\"secret.mp3\",\"private\":\"true\",\"size\":\"123\",\"md5\":\"900150983cd24fb0d6963f7d28e17f72\"},{\"name\":\"bad.exe\",\"size\":\"123\",\"md5\":\"900150983cd24fb0d6963f7d28e17f72\"}]}";
assert(discovery_parse_album("release",body,r,err,sizeof(err)));assert(r->count==1&&discovery_item_valid(&r->items[0]));assert(!discovery_parse_album("../bad",body,r,err,sizeof(err)));r->items[0].url[0]='h';strcpy(r->items[0].url,"https://evil/track.mp3");assert(!discovery_item_valid(&r->items[0]));free(r);
char safe[100];download_safe_name(" ../bad\\name:*?  ",safe,sizeof(safe));assert(!strcmp(safe,"badname"));download_safe_name("...",safe,sizeof(safe));assert(!strcmp(safe,"Track"));
}
static void test_pages(void) {
    discovery_results_t *r=calloc(1,sizeof(*r));assert(r);char err[192];
    catalog_fixture="{\"response\":{\"start\":24,\"numFound\":25,\"docs\":[{\"identifier\":\"last\",\"title\":\"Final release\",\"creator\":\"Artist\",\"licenseurl\":\"https://creativecommons.org/licenses/by/4.0/\"}]}}";
    assert(discovery_search_page("Artist",true,2,r,err,sizeof(err)));
    assert(r->count==1 && !r->truncated);
    assert(strstr(catalog_url,"page=2&") && strstr(catalog_url,"creator%3A%22Artist%22"));
    assert(!strstr(catalog_url,"title%3A"));
    assert(discovery_search_page("x\" OR *:*",false,1,r,err,sizeof(err)));
    assert(strstr(catalog_url,"x%5C%22%20OR%20%2A%3A%2A"));
    assert(!discovery_search_page("artist",false,0,r,err,sizeof(err)));
    assert(!discovery_search_page("artist",false,DISCOVERY_MAX_PAGES+1,r,err,sizeof(err)));
    assert(!discovery_search_page("\n\t",false,1,r,err,sizeof(err)));
    catalog_fixture=NULL;
    assert(discovery_parse_search("{\"response\":{\"start\":0,\"numFound\":50,\"docs\":[{\"identifier\":\"no-license\"}]}}",r,err,sizeof(err)));
    assert(r->count==0 && r->truncated); /* An empty eligible page still has a next page. */
    char *body=calloc(1,16384);assert(body);
    size_t used=(size_t)snprintf(body,16384,"{\"metadata\":{\"mediatype\":\"audio\",\"licenseurl\":\"https://creativecommons.org/licenses/by/4.0/\"},\"files\":[");
    for(int i=0;i<30;i++) used+=(size_t)snprintf(body+used,16384-used,
        "%s{\"name\":\"track%02d.mp3\",\"size\":123,\"md5\":\"900150983cd24fb0d6963f7d28e17f72\"}",i?",":"",i);
    snprintf(body+used,16384-used,"]}");
    assert(discovery_parse_album_page("release",body,1,r,err,sizeof(err)));
    assert(r->count==24 && r->truncated && !strcmp(r->items[23].filename,"track23.mp3"));
    assert(discovery_parse_album_page("release",body,2,r,err,sizeof(err)));
    assert(r->count==6 && !r->truncated && !strcmp(r->items[0].filename,"track24.mp3"));
    assert(!discovery_parse_album_page("release",body,3,r,err,sizeof(err)) && r->count==0);
    assert(!discovery_parse_album_page("release",body,0,r,err,sizeof(err)));
    free(body);free(r);
    puts("PASS paged catalog/artist queries: literal escaping, valid page bounds, final/empty eligible pages, release file pagination");
}
int main(void){memset(payload,'a',sizeof(payload));test_md5();test_provider();test_pages();char temp[]="/tmp/sonix-discovery-test-XXXXXX";assert(mkdtemp(temp));assert(!download_queue_init(""));assert(download_queue_init(temp));assert(download_queue_init(temp));char err[192];discovery_item_t a=item("complete");unsigned id=download_enqueue(&a,err,sizeof(err));assert(id);assert(download_enqueue(&a,err,sizeof(err))==id);download_job_t j=wait_terminal(id);assert(j.state==DOWNLOAD_DONE&&j.done==sizeof(payload));struct stat st;assert(!stat(j.path,&st)&&st.st_size==sizeof(payload));assert(download_revision()==1);
atomic_store(&opens,0);atomic_store(&mode,1);a=item("retry");id=download_enqueue(&a,err,sizeof(err));j=wait_terminal(id);assert(j.state==DOWNLOAD_DONE&&j.attempts==2);assert(atomic_load(&opens)==2);
atomic_store(&mode,2);a=item("pause");id=download_enqueue(&a,err,sizeof(err));wait_progress(id);assert(download_pause(id));usleep(150000);assert(snapshot(id).state==DOWNLOAD_PAUSED);assert(download_resume(id));j=wait_terminal(id);assert(j.state==DOWNLOAD_DONE);
a=item("cancel");id=download_enqueue(&a,err,sizeof(err));wait_progress(id);assert(download_cancel(id));usleep(150000);assert(snapshot(id).state==DOWNLOAD_CANCELLED);assert(access(snapshot(id).path,F_OK)!=0);
atomic_store(&mode,0);a=item("checksum");memset(a.md5,'0',32);id=download_enqueue(&a,err,sizeof(err));j=wait_terminal(id);assert(j.state==DOWNLOAD_FAILED&&strstr(j.error,"Integrity"));assert(access(j.path,F_OK)!=0);
a=item("unsafe");strcpy(a.url,"http://archive.org/download/test-release/unsafe.mp3");assert(!download_enqueue(&a,err,sizeof(err)));
atomic_store(&mode,3);a=item("full-card");id=download_enqueue(&a,err,sizeof(err));j=wait_terminal(id);assert(j.state==DOWNLOAD_FAILED&&strstr(j.error,"free microSD"));assert(access(j.path,F_OK)!=0);
atomic_store(&mode,2);a=item("removed-card");id=download_enqueue(&a,err,sizeof(err));wait_progress(id);char moved[512];snprintf(moved,sizeof(moved),"%s-moved",temp);assert(!rename(temp,moved));j=wait_terminal(id);assert(j.state==DOWNLOAD_FAILED&&strstr(j.error,"removed"));assert(!rename(moved,temp));
/* Another writer creates the final name after transfer began. Never replace
 * it, even if our initial existence check saw no file. */
a=item("concurrent-file");id=download_enqueue(&a,err,sizeof(err));assert(id);wait_progress(id);j=snapshot(id);
int other=open(j.path,O_CREAT|O_EXCL|O_WRONLY,0600);assert(other>=0);assert(write(other,"keep",4)==4);close(other);
j=wait_terminal(id);assert(j.state==DOWNLOAD_FAILED&&strstr(j.error,"finalize"));
char kept[4];other=open(j.path,O_RDONLY);assert(other>=0&&read(other,kept,4)==4&&!memcmp(kept,"keep",4));close(other);
download_queue_shutdown();assert(download_queue_init(temp));download_job_t restored[8];int n=download_snapshot(restored,8);assert(n>=4);for(int i=0;i<n;i++)assert(restored[i].state==DOWNLOAD_PAUSED);download_queue_shutdown();
char root2[]="/tmp/sonix-discovery-symlink-XXXXXX";assert(mkdtemp(root2));char link[512];snprintf(link,sizeof(link),"%s/Downloads",root2);assert(!symlink(temp,link));assert(!download_queue_init(root2));
printf("PASS: provider parsing/license bounds, incremental MD5, sanitization, exact bytes, duplicate queue, retry+resume, pause/resume, cancellation, integrity rejection, insufficient storage, removed card, persisted-paused restoration, symlink rejection\nArtifacts: %s\n",temp);return 0;}
