#include "src/system/library/library.h"
#include "src/system/core/config.h"
#include "src/gui/nowplaying/cover.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void wait_scan(void) {
    for (int i = 0; i < 1000 && library_scan_running(); i++) usleep(10000);
    assert(!library_scan_running());
}

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
    assert(!library_import_download("relative.wav"));
    assert(!library_import_download("/not-present.wav"));
    assert(library_import_download(first));
    wait_scan();
    assert(library_track_count() == 1);
    assert(library_track_title(first, title, sizeof(title)));
    assert(strcmp(title, "An imported title") == 0);
    assert(!library_detect_changes());
    assert(library_import_download(first));
    wait_scan();
    assert(library_track_count() == 1);
    assert(library_import_download(second));
    wait_scan();
    assert(library_track_count() == 2);
    assert(library_track_title(first, title, sizeof(title)));
    assert(library_track_title(second, title, sizeof(title)));
    assert(!library_detect_changes());
    library_close();
    assert(library_open(db));
    assert(library_track_count() == 2);
    library_close();
    puts("PASS library import: empty index, no duplicates, existing tracks retained, settings unchanged, persistent index, bounded newest-first history, new folder artwork invalidates cached absence");
    return 0;
}
