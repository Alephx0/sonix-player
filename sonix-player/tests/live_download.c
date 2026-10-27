/* Opt-in network integration: openly licensed Archive fixture, real transport,
 * queue, checksum, index and decoder. Never performs hardware output or flash. */
#include "src/system/discovery/discovery.h"
#include "src/system/discovery/download_queue.h"
#include "src/system/library/library.h"
#include "src/system/library/metadata.h"
#include "src/system/decode/decode.h"
#include "src/system/core/config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv) {
    assert(argc == 2);
    char path[1024], error[192] = "";
    snprintf(path, sizeof(path), "%s/settings.ini", argv[1]);
    config_init(path);
    discovery_results_t *results = calloc(1, sizeof(*results));
    assert(results);
    if (!discovery_album("loc-jukebox-733632-toccata", results, error, sizeof(error))) {
        fprintf(stderr, "FAIL live metadata: %s\n", error); free(results); return 2;
    }
    assert(results->count > 0);
    printf("Live licensed source: %s | %s | %lld bytes\n", results->items[0].title,
           results->items[0].license, results->items[0].bytes);
    assert(download_queue_init(argv[1]));
    unsigned id = download_enqueue(&results->items[0], error, sizeof(error));
    if (!id) { fprintf(stderr, "FAIL enqueue: %s\n", error); return 3; }
    download_resume(id); /* Also makes a rerun verify a restored paused job. */
    free(results);
    download_job_t jobs[DOWNLOAD_MAX_JOBS], completed = {0};
    for (int tick = 0; tick < 1800; ++tick) {
        int count = download_snapshot(jobs, DOWNLOAD_MAX_JOBS);
        for (int i = 0; i < count; i++) if (jobs[i].id == id) completed = jobs[i];
        if (completed.state == DOWNLOAD_DONE || completed.state == DOWNLOAD_FAILED) break;
        usleep(100000);
    }
    download_queue_shutdown();
    if (completed.state != DOWNLOAD_DONE) {
        fprintf(stderr, "FAIL transfer: %s (%lld/%lld)\n", completed.error, completed.done, completed.total);
        return 4;
    }
    printf("PASS completed file and source MD5: %lld bytes, %u attempts (use --fresh to force network transfer)\n", completed.done, completed.attempts);
    snprintf(path, sizeof(path), "%s/.local", argv[1]); mkdir(path, 0755);
    snprintf(path, sizeof(path), "%s/.local/library.db", argv[1]);
    assert(library_open(path));
    assert(library_import_download(completed.path));
    for (int i = 0; i < 1000 && library_scan_running(); ++i) usleep(10000);
    assert(!library_scan_running());
    char title[256];
    assert(library_track_title(completed.path, title, sizeof(title)));
    printf("PASS indexed metadata: %s\n", title);
    library_close();
    decoder_t *decoder = decoder_open(completed.path, decode_detect_format(completed.path));
    assert(decoder);
    int32_t pcm[4096];
    int frames = (int)decoder_read_pcm_frames_s32(decoder, 1024, pcm);
    assert(frames > 0);
    printf("PASS offline decode: %d frames, %d Hz, %d channels (%s)\n", frames,
           decoder_sample_rate(decoder), decoder_channels(decoder), decoder_codec_name(decoder));
    decoder_close(decoder);
    return 0;
}
