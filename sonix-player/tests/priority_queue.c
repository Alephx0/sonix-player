#include "src/system/playback/playlist.h"
#include "src/system/core/config.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "src/system/db/sqlite3.h"

static const char *album[] = {"/A.mp3","/B.mp3","/C.mp3","/D.mp3","/E.mp3"};
static void next_is(const char *wanted) {
    char path[512], peek[512];
    assert(playlist_peek_auto(peek,sizeof(peek)));
    assert(!strcmp(peek,wanted));
    assert(playlist_next(path,sizeof(path)));
    assert(!strcmp(path,wanted));
}
int main(int argc, char **argv) {
    assert(argc==2);
    char config[768], path[512], before[5][512];
    snprintf(config,sizeof(config),"%s/config.ini",argv[1]);config_init(config);
    playlist_set_card_root(argv[1]);
    config_set_int("player","remember_track",1);
    playlist_load_paths(album,5,0);
    assert(playlist_enqueue("/X.mp3") && playlist_enqueue("/Y.mp3"));
    assert(playlist_context_count()==5 && playlist_count()==7);
    playlist_set_shuffle(true);
    assert(playlist_priority_count()==2 && playlist_is_priority(1));
    next_is("/X.mp3");
    assert(playlist_enqueue("/Z.mp3"));
    next_is("/Y.mp3");next_is("/Z.mp3");
    playlist_set_shuffle(false);
    next_is("/B.mp3");next_is("/C.mp3");
    assert(playlist_prev(path,sizeof(path)) && !strcmp(path,"/B.mp3"));
    assert(playlist_prev(path,sizeof(path)) && !strcmp(path,"/Z.mp3"));
    next_is("/B.mp3");next_is("/C.mp3");next_is("/D.mp3");next_is("/E.mp3");
    assert(!playlist_next(path,sizeof(path)));
    playlist_set_repeat(1);next_is("/A.mp3");
    assert(playlist_priority_count()==0); /* Manual entries are not repeated. */
    /* Reference layout: "Add to queue" always appends to the manual queue,
     * even with the legacy "add at end of context" preference set. */
    config_set_int("screen","player_layout_alt",3);config_set_int("player","queue_add_end",1);
    {int ctx=playlist_context_count();assert(playlist_enqueue("/Q.mp3"));
     assert(playlist_priority_count()==1 && playlist_context_count()==ctx);
     playlist_priority_clear();}
    config_set_int("player","queue_add_end",0);
    playlist_set_shuffle(true);playlist_set_repeat(2);
    assert(playlist_shuffle_enabled() && playlist_repeat_mode()==2);
    assert(playlist_advance_auto(path,sizeof(path)) && !strcmp(path,"/A.mp3"));
    playlist_set_repeat(0);
    for(int i=0;i<5;i++) assert(playlist_path_at(i,before[i],sizeof(before[i])));
    playlist_load_paths(album,5,2); /* Same context selection preserves deal. */
    for(int i=0;i<5;i++) {assert(playlist_path_at(i,path,sizeof(path)));assert(!strcmp(path,before[i]));}
    /* An explicit shuffled Play re-deals with the playing track first, so the
     * rest of the context is still to come. */
    playlist_shuffle_restart();
    assert(playlist_current_index()==0 && playlist_context_count()==5);
    {char cur[512];assert(playlist_current_path(cur,sizeof(cur)) && playlist_path_at(0,path,sizeof(path)) && !strcmp(cur,path));}
    assert(playlist_priority_add("/X.mp3") && playlist_priority_add("/Y.mp3"));
    const char *other[]={"/Q.mp3","/R.mp3"};
    playlist_load_paths(other,2,0);
    assert(playlist_priority_count()==2);
    playlist_set_shuffle(false);
    assert(playlist_move(1,2));
    next_is("/Y.mp3");
    playlist_priority_clear();
    assert(playlist_context_count()==2);next_is("/R.mp3");
    playlist_clear();playlist_load_paths(album,5,0);
    /* The context's kind ("PLAYING FROM PLAYLIST") is stored with its name,
     * never without one, and a new context clears both. */
    playlist_set_context("PLAYLIST","Night Drive");
    assert(!strcmp(playlist_context_kind(),"PLAYLIST") && !strcmp(playlist_context_name(),"Night Drive"));
    assert(!strcmp(config_get("player","context_kind",""),"PLAYLIST"));
    playlist_set_context("ARTIST","");assert(!playlist_context_kind()[0]);
    playlist_set_context("PLAYLIST","Night Drive");playlist_load_paths(other,2,0);
    assert(!playlist_context_kind()[0] && !playlist_context_name()[0] && !config_get("player","context_kind","")[0]);
    playlist_clear();playlist_load_paths(album,5,0);
    playlist_set_shuffle(true);
    assert(playlist_priority_add("/X.mp3") && playlist_priority_add("/Y.mp3"));
    next_is("/X.mp3");
    playlist_session_save();
    char session[900];snprintf(session,sizeof(session),"%s/.local/playback-session.db",argv[1]);
    assert(!unlink(session));playlist_session_save(); /* Recreate an externally removed sidecar. */
    int saved_count=playlist_count();
    char saved[8][512];for(int i=0;i<saved_count;i++) assert(playlist_path_at(i,saved[i],sizeof(saved[i])));
    playlist_clear();playlist_load_paths(album,5,0);playlist_session_restore();
    assert(playlist_count()==saved_count && playlist_priority_count()==1);
    assert(playlist_current_path(path,sizeof(path)) && !strcmp(path,"/X.mp3"));
    for(int i=0;i<saved_count;i++) {assert(playlist_path_at(i,path,sizeof(path)));assert(!strcmp(path,saved[i]));}
    next_is("/Y.mp3");
    sqlite3 *session_db=NULL;assert(sqlite3_open(session,&session_db)==SQLITE_OK);
    assert(sqlite3_exec(session_db,"UPDATE sequence SET value='99999999' WHERE kind=0 AND slot=0",NULL,NULL,NULL)==SQLITE_OK);
    sqlite3_close(session_db);playlist_session_restore();
    assert(playlist_current_path(path,sizeof(path)) && !strcmp(path,"/Y.mp3"));
    int remove=playlist_current_index()+1;
    assert(playlist_remove(&remove,1));assert(playlist_context_count()==4);
    playlist_clear();
    assert(playlist_priority_add("/alone.mp3"));next_is("/alone.mp3");
    assert(!playlist_next(path,sizeof(path)));
    playlist_clear();
    puts("PASS priority queue: FIFO, shuffle isolation, context replacement, repeat isolation, previous history, stable deal selection, edit/remove, exact session restore, standalone queue");
    return 0;
}
