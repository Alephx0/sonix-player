#include "src/system/net/http.h"
#include "src/system/discovery/discovery.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const char *tr(const char *s){return s;}
const char *config_get(const char *section,const char *key,const char *fallback){(void)section;(void)key;const char *v=getenv("TEST_CA");return v?v:fallback;}
static void check(const char *base,const char *path,long long offset,bool want,long long accepted_want){char url[512];snprintf(url,sizeof(url),"%s/%s",base,path);http_stream_t stream;long long accepted=-1;bool ok=http_download_open(&stream,url,offset,10,3,&accepted);if(ok!=want){fprintf(stderr,"case %s: got %d want %d: %s\n",path,ok,want,http_last_error());abort();}if(ok){assert(accepted==accepted_want);char body[16];int total=0,n;while((n=http_stream_read(&stream,body+total,10-total))>0){total+=n;if(total==10-accepted)break;}assert(total==10-accepted);assert(!memcmp(body,"0123456789"+accepted,(size_t)total));http_stream_close(&stream);}}
int main(int argc,char **argv){assert(argc>=2);if(!strcmp(argv[1],"--live")){discovery_results_t *r=calloc(1,sizeof(*r));char err[192];if(!discovery_search(argc>2?argv[2]:"test",r,err,sizeof(err))){fprintf(stderr,"LIVE SEARCH FAIL: %s\n",err);free(r);return 2;}printf("LIVE SEARCH: %d licensed releases (truncated=%d)\n",r->count,r->truncated);for(int i=0;i<r->count&&i<4;i++)printf("%s | %s | %s\n",r->items[i].id,r->items[i].title,r->items[i].license);if(r->count){char id[128];snprintf(id,sizeof(id),"%s",r->items[0].id);if(discovery_album(id,r,err,sizeof(err))){printf("LIVE ALBUM: %d downloadable audio variants\n",r->count);if(r->count)printf("%s | %lld bytes | %s\n",r->items[0].title,r->items[0].bytes,r->items[0].format);}else printf("LIVE ALBUM ERROR: %s\n",err);}free(r);return 0;}
check(argv[1],"ok",0,true,0);check(argv[1],"range",4,true,4);check(argv[1],"ignore-range",4,true,0);check(argv[1],"bad-range",4,false,0);check(argv[1],"bad-total",4,false,0);check(argv[1],"length",0,false,0);check(argv[1],"chunked",0,false,0);check(argv[1],"encoded",0,false,0);check(argv[1],"redirect",0,true,0);check(argv[1],"downgrade",0,false,0);check(argv[1],"loop",0,false,0);check(argv[1],"missing",0,false,0);
char url[512];snprintf(url,sizeof(url),"%s/downgrade",argv[1]);char *body=NULL;http_req_t req={.https_only=true};assert(!http_request(url,&req,&body,NULL,1024,3,NULL));assert(!body);
if(argc>2)check(argv[2],"ok",0,false,0);
puts("PASS: actual TLS verification, exact length, valid/invalid ranges, ignored Range restart, encoded/chunked rejection, HTTPS redirects, downgrade refusal, redirect limit, 404, metadata HTTPS-only");return 0;}
