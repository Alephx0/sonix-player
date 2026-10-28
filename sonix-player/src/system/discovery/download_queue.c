#define _GNU_SOURCE 1
#include "download_queue.h"
#include "src/system/net/http.h"
#include "src/system/core/json.h"
#include "src/system/core/md5.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
/* Rockbox's userspace headers predate renameat2. The R1 kernel is 4.4;
 * Linux v4.4 arch/mips/include/uapi/asm/unistd.h assigns o32 syscall 4351.
 * Do not reuse this number for n32/n64 or another architecture. */
#if !defined(SYS_renameat2) && defined(__mips__) && _MIPS_SIM == 1
#define SYS_renameat2 4351
#endif
#define CHUNK 16384
#define STORAGE_MARGIN (1024 * 1024)
#define QUEUE_FILE ".sonix-downloads.json"
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t lifecycle=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t changed=PTHREAD_COND_INITIALIZER;
static pthread_t worker;
static bool started, stopping, dirty;
static download_job_t jobs[DOWNLOAD_MAX_JOBS];
static int count, root_fd=-1, dir_fd=-1;
static bool cleaned[DOWNLOAD_MAX_JOBS];
static unsigned next_id=1, revision;
static char card_root[512];
static download_options_t options={true,3,0,0};
void download_configure(const download_options_t *value) {
    if(!value)return;
    pthread_mutex_lock(&lock);options=*value;
    if(options.attempts<1 || options.attempts>3)options.attempts=3;
    if(options.rate_kib && (options.rate_kib<64 || options.rate_kib>1024))options.rate_kib=0;
    if(options.naming>2)options.naming=0;
    pthread_cond_signal(&changed);pthread_mutex_unlock(&lock);
}
download_options_t download_options(void) {
    pthread_mutex_lock(&lock);download_options_t out=options;pthread_mutex_unlock(&lock);return out;
}
static http_stream_t *active_stream;
static unsigned active_id, processing_id;
static struct stat card_identity;
static void error_text(char *out,size_t n,const char *s){if(out&&n)snprintf(out,n,"%s",s);}
const char *download_state_name(download_state_t s) {
    static const char *names[]={"Queued","Downloading","Paused","Downloaded","Failed","Cancelled"};
    return (unsigned)s<sizeof(names)/sizeof(*names)?names[s]:"Unknown";
}
void download_safe_name(const char *text,char *out,size_t size) {
    if(!out||!size)return;
    size_t used=0,cap=size-1;if(cap>96)cap=96;
    const unsigned char *p=(const unsigned char*)(text?text:"");
    while(*p && used<cap) {
        unsigned char c=*p;
        if(c<32 || c==127 || strchr("/\\:*?\"<>|",c)){p++;continue;}
        size_t bytes=c<128?1:(c>=0xc2&&c<=0xdf?2:(c>=0xe0&&c<=0xef?3:(c>=0xf0&&c<=0xf4?4:0)));
        if(!bytes){p++;continue;}
        bool valid=true;for(size_t i=1;i<bytes;i++)if(!p[i] || (p[i]&0xc0)!=0x80){valid=false;break;}
        if(!valid){p++;continue;}
        if(used+bytes>cap)break;
        memcpy(out+used,p,bytes);used+=bytes;p+=bytes;
    }
    while(used && (out[used-1]=='.'||out[used-1]==' '))used--;
    out[used]=0;size_t leading=strspn(out,". ");if(leading)memmove(out,out+leading,used-leading+1);
    if(!*out)snprintf(out,size,"Track");
}
static void filename(const discovery_item_t *it,unsigned naming,char *out,size_t n) {
    char title[100],artist[100];download_safe_name(it->title,title,sizeof(title));
    download_safe_name(it->artist,artist,sizeof(artist));
    if(naming) {
        /* Include full source identity in the digest, never rely on title alone. */
        char identity[33];md5_hex(it->url,strlen(it->url),identity);
        /* Bound on UTF-8 code points before formatting, never split a name. */
        char short_title[81],short_artist[81];
        download_safe_name(title,short_title,sizeof(short_title));download_safe_name(artist,short_artist,sizeof(short_artist));
        snprintf(out,n,"%.80s - %.80s-%.12s-%.12s.%s",naming==1?short_title:short_artist,
                 naming==1?short_artist:short_title,identity,it->md5,it->format);
    } else snprintf(out,n,"%s-%.12s-%s.%s",it->id,it->md5,title,it->format);
}
static bool destination_valid(const char *name) {
    return name && *name && name[0]!='.' && !strpbrk(name,"/\\") && strlen(name)<256;
}
static bool card_present(void) {
    struct stat st;
    return stat(card_root,&st)==0 && st.st_dev==card_identity.st_dev && st.st_ino==card_identity.st_ino;
}
static bool free_space(long long need) {
    struct statvfs v;
    if(fstatvfs(dir_fd,&v))return false;
    unsigned long long available=(unsigned long long)v.f_bavail*v.f_frsize;
    return need>=0 && available>=(unsigned long long)need+STORAGE_MARGIN;
}
static int index_for(unsigned id){for(int i=0;i<count;i++)if(jobs[i].id==id)return i;return -1;}
static bool interrupted(unsigned id) {
    pthread_mutex_lock(&lock);int i=index_for(id);
    bool stop=stopping || i<0 || jobs[i].state!=DOWNLOAD_RUNNING;
    pthread_mutex_unlock(&lock);return stop;
}
static void quote(FILE *f,const char *s) {
    fputc('"',f);
    for(const unsigned char *p=(const unsigned char*)s;*p;p++) {
        if(*p=='"'||*p=='\\'){fputc('\\',f);fputc(*p,f);}
        else if(*p<32)fprintf(f,"\\u%04x",*p);
        else fputc(*p,f);
    } fputc('"',f);
}
static void item_write(FILE *f,const discovery_item_t *it) {
    fprintf(f,"{\"bytes\":%lld",it->bytes);
#define FIELD(name) fputs(",\"" #name "\":",f);quote(f,it->name)
    FIELD(id);FIELD(title);FIELD(artist);FIELD(album);FIELD(filename);FIELD(url);FIELD(artwork);FIELD(license);FIELD(format);FIELD(md5);
#undef FIELD
    fputc('}',f);
}
/* The only caller is the worker (or shutdown after it has joined). The UI only
 * sets dirty; neither queue persistence nor audio I/O blocks the UI thread. */
static void persist(void) {
    download_job_t snapshot[DOWNLOAD_MAX_JOBS];int n;
    pthread_mutex_lock(&lock);n=count;memcpy(snapshot,jobs,(size_t)n*sizeof(*jobs));dirty=false;pthread_mutex_unlock(&lock);
    if(!card_present())return;
    int fd=openat(dir_fd,QUEUE_FILE ".part",O_WRONLY|O_CREAT|O_TRUNC|O_NOFOLLOW|O_CLOEXEC,0600);
    if(fd<0)return;
    FILE *f=fdopen(fd,"w");if(!f){close(fd);return;}
    fprintf(f,"{\"version\":1,\"jobs\":[");
    for(int i=0;i<n;i++){if(i)fputc(',',f);fprintf(f,"{\"id\":%u,\"state\":%d,\"item\":",snapshot[i].id,snapshot[i].state);item_write(f,&snapshot[i].item);fputs(",\"destination\":",f);quote(f,snapshot[i].destination);fputc('}',f);}
    fputs("]}\n",f);
    bool ok=!ferror(f)&&fflush(f)==0&&fsync(fd)==0;
    if(fclose(f)!=0)ok=false;
    if(ok){if(renameat(dir_fd,QUEUE_FILE ".part",dir_fd,QUEUE_FILE)==0)fsync(dir_fd);}
    else unlinkat(dir_fd,QUEUE_FILE ".part",0);
}
static bool get_field(const json_doc_t *d,int obj,const char *name,char *out,size_t size) {
    int t=json_get(d,obj,name);
    return t>=0 && d->toks[t].type==JSON_STRING && (size_t)(d->toks[t].end-d->toks[t].start)<size && json_str(d,t,out,size);
}
static void restore(void) {
    int fd=openat(dir_fd,QUEUE_FILE,O_RDONLY|O_NOFOLLOW|O_CLOEXEC);if(fd<0)return;
    struct stat st;
    if(fstat(fd,&st)||!S_ISREG(st.st_mode)||st.st_size<=0||st.st_size>262144){close(fd);return;}
    char *body=malloc((size_t)st.st_size+1);if(!body){close(fd);return;}
    size_t got=0;while(got<(size_t)st.st_size){ssize_t n=read(fd,body+got,(size_t)st.st_size-got);if(n<=0)break;got+=(size_t)n;}close(fd);body[got]=0;
    json_doc_t d;
    if(got!=(size_t)st.st_size || !json_parse(body,&d)){free(body);return;}
    int root=json_root(&d),arr=json_get(&d,root,"jobs");
    if(json_obj_long(&d,root,"version",0)!=1 || arr<0 || d.toks[arr].type!=JSON_ARRAY || json_len(&d,arr)>DOWNLOAD_MAX_JOBS){json_free(&d);free(body);return;}
    for(int i=0;i<json_len(&d,arr);i++) {
        int obj=json_at(&d,arr,i),item=json_get(&d,obj,"item");download_job_t job={0};
        if(json_obj_long(&d,obj,"state",DOWNLOAD_PAUSED)==DOWNLOAD_CANCELLED)continue;
        bool ok=true;
#define FIELD(name) ok=get_field(&d,item,#name,job.item.name,sizeof(job.item.name))&&ok
        FIELD(id);FIELD(title);FIELD(artist);FIELD(album);FIELD(filename);FIELD(url);FIELD(artwork);FIELD(license);FIELD(format);FIELD(md5);
#undef FIELD
        job.item.bytes=json_obj_llong(&d,item,"bytes",0);job.item.kind=DISCOVERY_TRACK;job.item.downloadable=true;
        if(!ok || !discovery_item_valid(&job.item))continue;
        /* Never start network work merely because a removable card contains a
         * manifest. Restored work requires an explicit Resume action. */
        job.state=DOWNLOAD_PAUSED;job.id=next_id++;job.total=job.item.bytes;
        char name[256];
        int dest=json_get(&d,obj,"destination");
        if(dest>=0) {if(!get_field(&d,obj,"destination",name,sizeof(name)) || !destination_valid(name))continue;}
        else filename(&job.item,0,name,sizeof(name)); /* legacy manifest */
        snprintf(job.destination,sizeof(job.destination),"%s",name);
        int wrote=snprintf(job.path,sizeof(job.path),"%.480s/Downloads/%s",card_root,name);
        if(wrote<=0||(size_t)wrote>=sizeof(job.path))continue;
        jobs[count++]=job;
    }
    json_free(&d);free(body);
}
static bool checksum_fd(int fd,long long bytes,md5_ctx *hash,unsigned id) {
    unsigned char buf[CHUNK];long long have=0;md5_init(hash);
    while(have<bytes) {
        if(interrupted(id)||!card_present())return false;
        size_t want=(size_t)((bytes-have)>CHUNK?CHUNK:(bytes-have));
        ssize_t n=read(fd,buf,want);if(n<0&&errno==EINTR)continue;
        if(n<=0)return false;
        md5_update(hash,buf,(size_t)n);have+=n;
    } return true;
}
static void progress(unsigned id,long long bytes,unsigned speed) {
    pthread_mutex_lock(&lock);int i=index_for(id);
    if(i>=0){jobs[i].done=bytes;jobs[i].bytes_per_second=speed;}
    pthread_mutex_unlock(&lock);
}
static void error_job(unsigned id,const char *message) {
    pthread_mutex_lock(&lock);int i=index_for(id);if(i>=0)snprintf(jobs[i].error,sizeof(jobs[i].error),"%s",message);pthread_mutex_unlock(&lock);
}
/* Returns 1 success, 0 retryable network failure, -1 terminal/storage/checksum. */
static int transfer(download_job_t *job) {
    unsigned id=job->id;char name[256],part[272];snprintf(name,sizeof(name),"%s",job->destination);snprintf(part,sizeof(part),".%s.part",name);
    if(!card_present()){error_job(id,"microSD card was removed or changed.");return -1;}
    int final=openat(dir_fd,name,O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
    if(final>=0) {
        struct stat st;md5_ctx h;char sum[33];
        bool ok=fstat(final,&st)==0 && S_ISREG(st.st_mode) && st.st_size==job->total && checksum_fd(final,st.st_size,&h,id);
        if(ok){md5_final_hex(&h,sum);ok=!strcmp(sum,job->item.md5);}close(final);
        if(ok){progress(id,job->total,0);return 1;}
        error_job(id,"A different or damaged file already exists. Rename it before retrying.");return -1;
    } else if(errno!=ENOENT){error_job(id,"Cannot safely open destination file.");return -1;}
    int fd=openat(dir_fd,part,O_RDWR|O_CREAT|O_NOFOLLOW|O_CLOEXEC,0600);
    if(fd<0){error_job(id,"Cannot create download on the microSD card.");return -1;}
    struct stat st;long long offset=0;md5_ctx hash;
    if(fstat(fd,&st)||!S_ISREG(st.st_mode)||st.st_size>job->total){close(fd);error_job(id,"Invalid partial download. Remove it before retrying.");return -1;}
    offset=st.st_size;
    if(!checksum_fd(fd,offset,&hash,id)){close(fd);return -1;}
    if(!free_space(job->total-offset)){close(fd);error_job(id,"Not enough free microSD space (1 MiB reserve required).");return -1;}
    http_stream_t stream;memset(&stream,0,sizeof(stream));stream.fd=-1;
    bool ok=true;int result=0;
    if(offset<job->total) {
        long long accepted=0;
        if(!http_download_open(&stream,job->item.url,offset,job->total,10,&accepted)) {
            error_job(id,http_last_error()?http_last_error():"Network failed. Retrying when connected.");close(fd);return 0;
        }
        if(accepted!=offset) {
            if(accepted!=0 || ftruncate(fd,0) || lseek(fd,0,SEEK_SET)<0){http_stream_close(&stream);close(fd);error_job(id,"Cannot restart partial download.");return -1;}
            offset=0;md5_init(&hash);
        }
        pthread_mutex_lock(&lock);active_stream=&stream;active_id=id;pthread_mutex_unlock(&lock);
        unsigned char buf[CHUNK];struct timespec begin,now;clock_gettime(CLOCK_MONOTONIC,&begin);long long initial=offset;
        unsigned limit_rate=download_options().rate_kib;
        struct timespec limit_begin=begin;long long limit_initial=offset;
        while(offset<job->total && !interrupted(id)) {
            if(!card_present()){error_job(id,"microSD card was removed or changed.");ok=false;result=-1;break;}
            int want=(int)((job->total-offset)>CHUNK?CHUNK:(job->total-offset));
            int n=http_stream_read(&stream,buf,want);
            if(n<=0){error_job(id,"Connection ended before the file was complete.");ok=false;break;}
            if(!free_space(n)){error_job(id,"microSD card is full.");ok=false;result=-1;break;}
            ssize_t written=0;
            while(written<n) {
                ssize_t wrote=write(fd,buf+written,(size_t)(n-written));
                if(wrote<0&&errno==EINTR)continue;
                if(wrote<=0){ok=false;result=-1;break;}written+=wrote;
            }
            if(!ok){error_job(id,"Writing to the microSD card failed.");break;}
            md5_update(&hash,buf,(size_t)n);offset+=n;
            clock_gettime(CLOCK_MONOTONIC,&now);
            double elapsed=(double)(now.tv_sec-begin.tv_sec)+(now.tv_nsec-begin.tv_nsec)/1e9;
            progress(id,offset,elapsed>0?(unsigned)((offset-initial)/elapsed):0);
            /* Short interruptible waits keep Pause/Cancel responsive while limited. */
            unsigned rate=download_options().rate_kib;
            if(rate!=limit_rate){limit_rate=rate;limit_begin=now;limit_initial=offset;}
            if(rate) {
                double target=(double)(offset-limit_initial)/(rate*1024.0);
                double limited_elapsed=(double)(now.tv_sec-limit_begin.tv_sec)+(now.tv_nsec-limit_begin.tv_nsec)/1e9;
                while(limited_elapsed<target && !interrupted(id)) {
                    struct timespec until;clock_gettime(CLOCK_REALTIME,&until);
                    until.tv_nsec+=50000000;if(until.tv_nsec>=1000000000){until.tv_sec++;until.tv_nsec-=1000000000;}
                    pthread_mutex_lock(&lock);
                    if(!stopping && options.rate_kib==rate)pthread_cond_timedwait(&changed,&lock,&until);
                    bool same=options.rate_kib==rate;pthread_mutex_unlock(&lock);
                    if(!same)break;
                    clock_gettime(CLOCK_MONOTONIC,&now);
                    limited_elapsed=(double)(now.tv_sec-limit_begin.tv_sec)+(now.tv_nsec-limit_begin.tv_nsec)/1e9;
                }
            }

        }
        pthread_mutex_lock(&lock);active_stream=NULL;active_id=0;pthread_mutex_unlock(&lock);
        http_stream_close(&stream);
    }
    if(interrupted(id)){fsync(fd);close(fd);return -1;}
    if(ok && offset==job->total) {
        char sum[33];md5_final_hex(&hash,sum);
        if(strcmp(sum,job->item.md5)){error_job(id,"Integrity check failed. Partial file discarded.");close(fd);unlinkat(dir_fd,part,0);return -1;}
        if(fsync(fd)){error_job(id,"Could not flush download to the microSD card.");close(fd);return -1;}
        if(close(fd)){error_job(id,"Could not close the completed file.");return -1;}fd=-1;
        /* Atomic no-replace commit; a separate existence check followed by
         * rename would overwrite a file created concurrently by file transfer.
         * Use the syscall to retain the device's glibc 2.22 ABI. Unsupported
         * card drivers fail safely, retaining the verified partial file. */
        if(!card_present() || syscall(SYS_renameat2,dir_fd,part,dir_fd,name,1U)) {
            error_job(id,"Could not finalize download without replacing another file.");return -1;
        }
        fsync(dir_fd);
        /* Source audio bytes (and embedded tags) are preserved exactly. The
         * sidecar retains attribution and provider metadata without retagging. */
        char meta[280];snprintf(meta,sizeof(meta),"%s.source.json",name);
        int m=openat(dir_fd,meta,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
        if(m>=0){FILE *f=fdopen(m,"w");if(f){item_write(f,&job->item);fputc('\n',f);fclose(f);}else close(m);}
        progress(id,job->total,0);return 1;
    }
    fsync(fd);close(fd);return result;
}
static void *run(void *unused) {
    (void)unused;
    /* Linux nice is per thread. Keep transfer/checksum work below playback. */
    setpriority(PRIO_PROCESS, (id_t)syscall(SYS_gettid), 10);
    for(;;) {
        pthread_mutex_lock(&lock);
        int cleanup=-1;
        for(int i=0;i<count;i++)if(jobs[i].state==DOWNLOAD_CANCELLED && !cleaned[i] && jobs[i].id!=processing_id){cleanup=i;break;}
        if(cleanup>=0) {
            char name[256],part[272];snprintf(name,sizeof(name),"%s",jobs[cleanup].destination);snprintf(part,sizeof(part),".%s.part",name);
            pthread_mutex_unlock(&lock);unlinkat(dir_fd,part,0);pthread_mutex_lock(&lock);
            cleaned[cleanup]=true;pthread_mutex_unlock(&lock);continue;
        }
        int selected=-1;
        for(int i=0;i<count;i++)if(jobs[i].state==DOWNLOAD_QUEUED){selected=i;break;}
        if(stopping){pthread_mutex_unlock(&lock);break;}
        if(dirty){pthread_mutex_unlock(&lock);persist();continue;}
        if(selected<0){pthread_cond_wait(&changed,&lock);pthread_mutex_unlock(&lock);continue;}
        jobs[selected].state=DOWNLOAD_RUNNING;jobs[selected].attempts=0;jobs[selected].error[0]=0;
        download_job_t job=jobs[selected];processing_id=job.id;pthread_mutex_unlock(&lock);persist();
        int result=-1;
        unsigned max_attempts=download_options().attempts;
        for(unsigned attempt=1;attempt<=max_attempts;attempt++) {
            pthread_mutex_lock(&lock);jobs[selected].attempts=attempt;pthread_mutex_unlock(&lock);
            if(interrupted(job.id))break;
            result=transfer(&job);
            if(result!=0 || interrupted(job.id))break;
            /* Bounded 1,2 second retry delay, interruptible by pause/cancel. */
            if(attempt<max_attempts) {
                struct timespec until;clock_gettime(CLOCK_REALTIME,&until);until.tv_sec+=(1U<<(attempt-1));
                pthread_mutex_lock(&lock);
                if(!stopping && jobs[selected].state==DOWNLOAD_RUNNING)pthread_cond_timedwait(&changed,&lock,&until);
                pthread_mutex_unlock(&lock);
            }
        }
        pthread_mutex_lock(&lock);
        if(jobs[selected].state==DOWNLOAD_RUNNING) {
            jobs[selected].state=stopping?DOWNLOAD_PAUSED:(result==1?DOWNLOAD_DONE:DOWNLOAD_FAILED);
            if(result==1){jobs[selected].error[0]=0;revision++;}
            else if(!jobs[selected].error[0])snprintf(jobs[selected].error,sizeof(jobs[selected].error),"Download interrupted.");
        }
        bool cancelled=jobs[selected].state==DOWNLOAD_CANCELLED;
        jobs[selected].bytes_per_second=0;processing_id=0;pthread_mutex_unlock(&lock);
        if(cancelled){char name[256],part[272];snprintf(name,sizeof(name),"%s",job.destination);snprintf(part,sizeof(part),".%s.part",name);unlinkat(dir_fd,part,0);}
        persist();
    }
    pthread_mutex_lock(&lock);
    for(int i=0;i<count;i++)if(jobs[i].state==DOWNLOAD_RUNNING || jobs[i].state==DOWNLOAD_QUEUED)jobs[i].state=DOWNLOAD_PAUSED;
    pthread_mutex_unlock(&lock);persist();return NULL;
}
bool download_queue_init(const char *root) {
    if(!root || !*root || root[0]!='/' || strlen(root)>480)return false;
    /* Serialize hotplug/USB shutdown against a UI opening the queue. */
    if(pthread_mutex_trylock(&lifecycle)!=0)return false;
    pthread_mutex_lock(&lock);
    if(started){bool same=!strcmp(root,card_root);pthread_mutex_unlock(&lock);pthread_mutex_unlock(&lifecycle);return same;}
    root_fd=open(root,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    if(root_fd<0){pthread_mutex_unlock(&lock);pthread_mutex_unlock(&lifecycle);return false;}
    if(fstat(root_fd,&card_identity) || (mkdirat(root_fd,"Downloads",0755)!=0 && errno!=EEXIST))goto bad;
    dir_fd=openat(root_fd,"Downloads",O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    if(dir_fd<0)goto bad;
    snprintf(card_root,sizeof(card_root),"%s",root);count=0;stopping=false;dirty=false;restore();
    pthread_attr_t attr;pthread_attr_init(&attr);pthread_attr_setstacksize(&attr,256*1024);
    int rc=pthread_create(&worker,&attr,run,NULL);pthread_attr_destroy(&attr);
    if(rc)goto bad;
    started=true;pthread_mutex_unlock(&lock);pthread_mutex_unlock(&lifecycle);return true;
bad:
    if(dir_fd>=0)close(dir_fd);
    close(root_fd);root_fd=dir_fd=-1;
    pthread_mutex_unlock(&lock);pthread_mutex_unlock(&lifecycle);return false;
}
static int duplicate(const discovery_item_t *item) {
    for(int i=0;i<count;i++)if(!strcmp(jobs[i].item.url,item->url) &&
        !strcmp(jobs[i].item.md5,item->md5) && jobs[i].state!=DOWNLOAD_CANCELLED)return i;
    return -1;
}
static bool reusable(int i) {
    return jobs[i].id!=processing_id && (jobs[i].state==DOWNLOAD_DONE ||
        (jobs[i].state==DOWNLOAD_CANCELLED && cleaned[i]));
}
static unsigned create_job(const discovery_item_t *item,int slot) {
    download_job_t *job=&jobs[slot];cleaned[slot]=false;memset(job,0,sizeof(*job));
    job->id=next_id++;job->state=options.start_immediately?DOWNLOAD_QUEUED:DOWNLOAD_PAUSED;job->item=*item;job->total=item->bytes;
    char name[256];filename(item,options.naming,name,sizeof(name));snprintf(job->destination,sizeof(job->destination),"%s",name);snprintf(job->path,sizeof(job->path),"%.480s/Downloads/%s",card_root,name);
    dirty=true;return job->id;
}
static unsigned enqueue_locked(const discovery_item_t *item) {
    int found=duplicate(item);
    if(found>=0)return jobs[found].id;
    int slot=count;
    if(slot==DOWNLOAD_MAX_JOBS) {
        slot=-1;for(int i=0;i<count;i++)if(reusable(i)){slot=i;break;}
        if(slot<0)return 0;
    } else count++;
    return create_job(item,slot);
}

unsigned download_enqueue(const discovery_item_t *item,char *err,size_t size) {
    if(!discovery_item_valid(item)){error_text(err,size,"This source does not authorize a supported download.");return 0;}
    pthread_mutex_lock(&lock);
    unsigned id=started&&!stopping?enqueue_locked(item):0;
    if(id)pthread_cond_signal(&changed);
    pthread_mutex_unlock(&lock);
    error_text(err,size,id?"":"Download queue unavailable or full (32 files).");return id;
}
bool download_lookup(const discovery_item_t *item,download_job_t *out) {
    if(!item || !out)return false;
    pthread_mutex_lock(&lock);int i=duplicate(item);if(i>=0)*out=jobs[i];pthread_mutex_unlock(&lock);
    return i>=0;
}
bool download_enqueue_release(const discovery_results_t *release,char *err,size_t size) {
    if(!release || release->count<1 || release->count>DISCOVERY_MAX_RESULTS || release->truncated) {
        error_text(err,size,"Open the complete release before downloading all files.");return false;
    }
    for(int i=0;i<release->count;i++)if(!discovery_item_valid(&release->items[i]) ||
        strcmp(release->items[0].id,release->items[i].id)) {
        error_text(err,size,"Release contains an invalid or unrelated source.");return false;
    }
    pthread_mutex_lock(&lock);
    int needed=0,available=DOWNLOAD_MAX_JOBS-count;
    bool keep[DOWNLOAD_MAX_JOBS]={0};
    for(int i=0;i<release->count;i++) {
        int found=duplicate(&release->items[i]);
        if(found>=0){keep[found]=true;continue;}
        bool earlier=false;
        for(int j=0;j<i;j++)if(!strcmp(release->items[j].url,release->items[i].url) &&
            !strcmp(release->items[j].md5,release->items[i].md5)){earlier=true;break;}
        if(!earlier)needed++;
    }
    for(int i=0;i<count;i++)if(!keep[i] && reusable(i))available++;
    if(!started || stopping || needed>available) {
        pthread_mutex_unlock(&lock);error_text(err,size,"Not enough download slots. Finish or cancel other transfers first.");return false;
    }
    /* Retain matching completed jobs while reserving all required slots. */
    for(int i=0;i<release->count;i++) {
        int found=duplicate(&release->items[i]);
        if(options.start_immediately && found>=0 && (jobs[found].state==DOWNLOAD_PAUSED || jobs[found].state==DOWNLOAD_FAILED))jobs[found].state=DOWNLOAD_QUEUED;
    }
    for(int i=0;i<release->count;i++) {
        if(duplicate(&release->items[i])>=0)continue;
        int slot=count;
        if(count==DOWNLOAD_MAX_JOBS) {
            /* Keep slot indices stable: the transfer worker owns its active slot. */
            slot=-1;for(int j=0;j<count;j++)if(!keep[j] && reusable(j)){slot=j;break;}
        } else count++;
        if(slot>=0){create_job(&release->items[i],slot);keep[slot]=true;}
    }
    dirty=true;pthread_cond_signal(&changed);pthread_mutex_unlock(&lock);error_text(err,size,"");return true;
}
int download_snapshot(download_job_t *out,int capacity) {
    if(!out||capacity<=0)return 0;
    pthread_mutex_lock(&lock);int n=count<capacity?count:capacity;memcpy(out,jobs,(size_t)n*sizeof(*jobs));pthread_mutex_unlock(&lock);return n;
}
static bool action(unsigned id,download_state_t target) {
    pthread_mutex_lock(&lock);int i=index_for(id);bool ok=i>=0;
    if(ok) {
        download_state_t state=jobs[i].state;
        if(target==DOWNLOAD_QUEUED)ok=state==DOWNLOAD_PAUSED||state==DOWNLOAD_FAILED;
        else ok=state==DOWNLOAD_RUNNING||state==DOWNLOAD_QUEUED||state==DOWNLOAD_PAUSED||state==DOWNLOAD_FAILED;
        if(ok){jobs[i].state=target;dirty=true;if(active_stream&&active_id==id)http_stream_wake(active_stream);pthread_cond_signal(&changed);}
    }
    pthread_mutex_unlock(&lock);return ok;
}
bool download_cancel(unsigned id){return action(id,DOWNLOAD_CANCELLED);}
bool download_pause(unsigned id){return action(id,DOWNLOAD_PAUSED);}
bool download_resume(unsigned id){return action(id,DOWNLOAD_QUEUED);}
unsigned download_revision(void){pthread_mutex_lock(&lock);unsigned r=revision;pthread_mutex_unlock(&lock);return r;}
bool download_busy(void){pthread_mutex_lock(&lock);bool busy=false;for(int i=0;i<count;i++)if(jobs[i].state==DOWNLOAD_RUNNING||jobs[i].state==DOWNLOAD_QUEUED)busy=true;pthread_mutex_unlock(&lock);return busy;}
void download_queue_shutdown(void) {
    pthread_mutex_lock(&lifecycle);
    pthread_mutex_lock(&lock);if(!started){pthread_mutex_unlock(&lock);pthread_mutex_unlock(&lifecycle);return;}
    stopping=true;if(active_stream)http_stream_wake(active_stream);pthread_cond_signal(&changed);pthread_mutex_unlock(&lock);
    pthread_join(worker,NULL);
    pthread_mutex_lock(&lock);close(dir_fd);close(root_fd);dir_fd=root_fd=-1;started=false;pthread_mutex_unlock(&lock);
    pthread_mutex_unlock(&lifecycle);
}
