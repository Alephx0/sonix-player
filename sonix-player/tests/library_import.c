#include "src/system/library/library.h"
#include "src/system/core/config.h"
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
    assert(library_open(db));
    assert(library_track_count() == 0);
    assert(!library_detect_changes());
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
    puts("PASS library import: empty index, no duplicates, existing tracks retained, settings unchanged, persistent index");
    return 0;
}
