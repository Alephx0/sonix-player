#include "src/system/library/library.h"
#include "src/system/core/config.h"
#include "src/gui/nowplaying/cover.h"
#include "src/system/db/sqlite3.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void wait_scan(void) {
    for (int i = 0; i < 1000 && library_scan_running(); i++) usleep(10000);
    assert(!library_scan_running());
}
static void search_result(const library_instant_hit_t *result,void *user) {(void)result;(void)user;}

int main(int argc, char **argv) {
    assert(argc == 2);
    char db[1024], settings[1024], first[1024], second[1024], title[256];
    snprintf(db, sizeof(db), "%s/library.db", argv[1]);
    snprintf(settings, sizeof(settings), "%s/settings.ini", argv[1]);
    snprintf(first, sizeof(first), "%s/first.wav", argv[1]);
    snprintf(second, sizeof(second), "%s/second.wav", argv[1]);
    config_init(settings);
    cover_set_cache_dir(argv[1]);
    cover_image_t picture;
    assert(!cover_thumb_load(first, false, 112, &picture));
    char artwork[1024], destination[1024];
    snprintf(artwork, sizeof(artwork), "%s/art/cover.png", argv[1]);
    snprintf(destination, sizeof(destination), "%s/cover.png", argv[1]);
    assert(!rename(artwork, destination));
    assert(cover_thumb_load(first, false, 112, &picture));
    assert(picture.pixels && picture.dsc.header.w <= 112);
    cover_free(&picture);
    /* Small consumers share the master, but must receive their own bounded
     * image and the same pixels on a warm disk-cache lookup. */
    const int sizes[] = {48, 66, 78};
    for (unsigned i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        cover_image_t warm;
        assert(cover_thumb_load(first, false, sizes[i], &picture));
        assert(picture.pixels && picture.dsc.header.w <= sizes[i] && picture.dsc.header.h <= sizes[i]);
        assert(cover_thumb_cached(first, sizes[i], &warm));
        assert(picture.dsc.data_size == warm.dsc.data_size);
        assert(!memcmp(picture.pixels, warm.pixels, picture.dsc.data_size));
        cover_free(&picture);
        cover_free(&warm);
    }
    assert(library_open(db));
    assert(library_track_count() == 0);
    assert(!library_detect_changes());
    for (int i = 0; i < 80; i++) library_history_add(first, "Recent track", "Artist", 1000 + i);
    library_history_item_t recent[80] = {0};
    assert(library_history_recent(recent, 80) == 64);
    assert(recent[0].played_at == 1079 && recent[63].played_at == 1016);
    assert(recent[64].id == 0);
    assert(library_history_recent(recent, 6) == 6 && recent[5].played_at == 1074);
    assert(library_history_recent(NULL, 6) == 0);
    assert(!library_import_file("relative.wav"));
    assert(!library_import_file("/not-present.wav"));
    assert(library_import_file(first));
    wait_scan();
    assert(library_track_count() == 1);
    /* A separate reader makes COMMIT/outer RELEASE busy. Neither queue-save
     * representation may leave an open transaction that blocks future imports. */
    for(int query=0;query<2;query++) {
        sqlite3 *reader=NULL;assert(sqlite3_open(db,&reader)==SQLITE_OK);
        assert(sqlite3_exec(reader,"BEGIN; SELECT count(*) FROM MEDIA_TABLE",NULL,NULL,NULL)==SQLITE_OK);
        if(query) {
            library_index_spec_t spec={.valid=true,.kind=LIBRARY_LIST_TRACKS};
            assert(!library_queue_save_query(&spec,0,NULL,NULL,0));
        } else {
            const char *paths[]={first};assert(!library_queue_save(paths,1,0,true));
        }
        assert(sqlite3_exec(reader,"ROLLBACK",NULL,NULL,NULL)==SQLITE_OK);sqlite3_close(reader);
        assert(library_import_file(first));wait_scan();
        /* Probe with a new writer: a leaked transaction still owns its lock. */
        assert(sqlite3_open(db,&reader)==SQLITE_OK);
        assert(sqlite3_exec(reader,"BEGIN IMMEDIATE; ROLLBACK",NULL,NULL,NULL)==SQLITE_OK);sqlite3_close(reader);
        if(query) {
            library_index_spec_t spec={.valid=true,.kind=LIBRARY_LIST_TRACKS},loaded={0};int position=-1;
            assert(library_queue_save_query(&spec,0,NULL,NULL,0));
            assert(library_queue_load_query(&loaded,&position) && loaded.valid && position==0);
        } else {
            const char *paths[]={first};char **loaded=NULL;int position=-1;bool custom=false;
            assert(library_queue_save(paths,1,0,true));
            assert(library_queue_load(&loaded,&position,&custom)==1 && position==0 && custom);
            assert(!strcmp(loaded[0],first));library_queue_free(loaded,1);
        }
    }
    for(int lyrics=0;lyrics<2;lyrics++) {
        sqlite3 *reader=NULL;assert(sqlite3_open(db,&reader)==SQLITE_OK);
        assert(sqlite3_exec(reader,"INSERT OR IGNORE INTO SEARCH_DIRTY SELECT path FROM MEDIA_TABLE; DELETE FROM SEARCH_LYRIC_STATE",NULL,NULL,NULL)==SQLITE_OK);
        assert(sqlite3_exec(reader,"BEGIN; SELECT count(*) FROM MEDIA_TABLE",NULL,NULL,NULL)==SQLITE_OK);
        if(lyrics)library_search_refresh_lyrics();else library_search_instant("imported",4,search_result,NULL);
        assert(sqlite3_exec(reader,"ROLLBACK",NULL,NULL,NULL)==SQLITE_OK);sqlite3_close(reader);
        assert(sqlite3_open(db,&reader)==SQLITE_OK);
        assert(sqlite3_exec(reader,"BEGIN IMMEDIATE; ROLLBACK",NULL,NULL,NULL)==SQLITE_OK);sqlite3_close(reader);
        assert(library_import_file(first));wait_scan();
    }
    assert(library_track_title(first, title, sizeof(title)));
    assert(strcmp(title, "An imported title") == 0);
    assert(!library_detect_changes());
    assert(library_import_file(first));
    wait_scan();
    assert(library_track_count() == 1);
    assert(library_import_file(second));
    wait_scan();
    assert(library_track_count() == 2);
    assert(library_track_title(first, title, sizeof(title)));
    assert(library_track_title(second, title, sizeof(title)));
    assert(!library_detect_changes());
    library_close();
    assert(library_open(db));
    assert(library_track_count() == 2);
    library_close();
    puts("PASS library import: empty index, no duplicates, existing tracks retained, queue/search/lyrics commit contention recovery, settings unchanged, persistent index, bounded newest-first history, new folder artwork invalidates cached absence");
    return 0;
}
