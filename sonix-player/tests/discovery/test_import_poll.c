#include "src/system/discovery/download_import.h"
#include "src/system/discovery/download_queue.h"
#include "src/system/library/library.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned revision = 1, calls[3];
static bool opened = true, scanning, indexed[3];
unsigned download_revision(void) { return revision; }
bool library_is_open(void) { return opened; }
bool library_scan_running(void) { return scanning; }
int download_snapshot(download_job_t *out, int capacity) {
    assert(capacity >= 3);
    memset(out, 0, 3 * sizeof(*out));
    for (int i = 0; i < 3; i++) {
        out[i].state = DOWNLOAD_DONE;
        snprintf(out[i].path, sizeof(out[i].path), "/%d.mp3", i);
    }
    return 3;
}
bool library_track_title(const char *path, char *out, size_t size) {
    (void)out; (void)size;
    return indexed[path[1] - '0'];
}
bool library_import_download(const char *path) {
    int i = path[1] - '0';
    calls[i]++;
    if (i == 0) return false; /* Removed after completion. */
    scanning = true;
    if (i == 2) indexed[i] = true;
    return true; /* Job 1 launches but fails to decode. */
}
int main(void) {
    opened = false;
    download_import_poll();
    assert(!calls[0]);
    opened = true; scanning = true;
    download_import_poll();
    assert(!calls[0]);
    scanning = false;
    download_import_poll();
    assert(calls[0] == 1);
    download_import_poll();
    assert(calls[1] == 1 && scanning);
    download_import_poll();
    assert(!calls[2]);
    scanning = false;
    download_import_poll();
    assert(calls[2] == 1 && indexed[2]);
    scanning = false;
    for (int i = 0; i < 57; i++) download_import_poll();
    assert(calls[0] == 1 && calls[1] == 1 && calls[2] == 1);
    download_import_poll();
    assert(calls[0] == 2); /* Failed job gets a bounded retry. */
    indexed[0] = indexed[1] = true;
    download_import_poll();
    for (int i = 0; i < 100; i++) download_import_poll();
    assert(calls[0] == 2 && calls[1] == 1 && calls[2] == 1);
    indexed[2] = false; revision++;
    download_import_poll();
    assert(calls[2] == 2); /* New completion restarts sweep. */
    puts("PASS import handoff: busy/closed deferral, removed and undecodable jobs cannot starve later files, bounded retry, confirmed-index generation");
    return 0;
}
