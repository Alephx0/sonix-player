#ifndef SONIX_DOWNLOAD_QUEUE_H
#define SONIX_DOWNLOAD_QUEUE_H
#include "discovery.h"
#define DOWNLOAD_MAX_JOBS 32
typedef enum {
    DOWNLOAD_QUEUED, DOWNLOAD_RUNNING, DOWNLOAD_PAUSED, DOWNLOAD_DONE,
    DOWNLOAD_FAILED, DOWNLOAD_CANCELLED
} download_state_t;
typedef struct {
    unsigned id;
    download_state_t state;
    discovery_item_t item;
    long long done, total;
    unsigned bytes_per_second, attempts;
    char path[768], error[192];
    char destination[256]; /* Captured at enqueue, stable across preference changes. */
} download_job_t;
typedef struct {
    bool start_immediately;
    unsigned attempts; /* 1..3 */
    unsigned rate_kib; /* 0 unlimited, otherwise 64..1024 KiB/s */
    unsigned naming; /* 0 source identity, 1 title - artist, 2 artist - title */
} download_options_t;
void download_configure(const download_options_t *options);
download_options_t download_options(void);
/* Call once per card/root before enqueue. Repeated call with same root is safe.
 * Changing roots is rejected while worker runs. NULL/empty is never a card. */
bool download_queue_init(const char *sd_root);
/* 0 on error; existing job ID on duplicate. Rejects unlicensed or malformed item. */
unsigned download_enqueue(const discovery_item_t *item, char *error, size_t size);
/* Atomic bounded release submission: no partial enqueue on failure. */
bool download_enqueue_release(const discovery_results_t *release, char *error, size_t size);
/* Exact source identity; copies the matching job without a large UI snapshot. */
bool download_lookup(const discovery_item_t *item, download_job_t *out);
int download_snapshot(download_job_t *out, int capacity);
bool download_cancel(unsigned id);
bool download_pause(unsigned id);
bool download_resume(unsigned id);
/* Monotonic completion generation; UI may refresh the local index on change. */
unsigned download_revision(void);
bool download_busy(void);
/* Joins worker. Use during orderly shutdown, never in a UI event handler. */
void download_queue_shutdown(void);
void download_safe_name(const char *text, char *out, size_t size);
const char *download_state_name(download_state_t state);
#endif
