#include "download_import.h"
#include "download_queue.h"
#include "src/system/library/library.h"
#include <string.h>

/* A removed or undecodable first job must not starve later completions.
 * Retry unindexed files at most once per minute of idle-scanner polls. A new
 * completion starts a fresh sweep. Busy/closed libraries retain pending work. */
void download_import_poll(void) {
    static unsigned indexed_revision, observed_revision, next;
    static unsigned cooldown[DOWNLOAD_MAX_JOBS];
    static download_job_t jobs[DOWNLOAD_MAX_JOBS];
    unsigned revision = download_revision();
    if (revision == indexed_revision || !library_is_open() || library_scan_running()) return;
    if (revision != observed_revision) {
        observed_revision = revision;
        memset(cooldown, 0, sizeof(cooldown));
    }
    int count = download_snapshot(jobs, DOWNLOAD_MAX_JOBS);
    bool pending = false;
    for (int i = 0; i < count; ++i) if (cooldown[i]) cooldown[i]--;
    for (int offset = 0; offset < count; ++offset) {
        unsigned i = (next + (unsigned)offset) % (unsigned)count;
        char title[256];
        if (jobs[i].state != DOWNLOAD_DONE || library_track_title(jobs[i].path, title, sizeof(title))) continue;
        pending = true;
        if (cooldown[i]) continue;
        cooldown[i] = 60;
        next = (i + 1) % (unsigned)count;
        /* Even a successful launch can fail to decode/index the file. Leave
         * the generation pending until the library confirms its presence. */
        library_import_download(jobs[i].path);
        return;
    }
    if (!pending) indexed_revision = revision;
}
