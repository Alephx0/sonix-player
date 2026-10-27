#ifndef SONIX_DISCOVERY_H
#define SONIX_DISCOVERY_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define DISCOVERY_MAX_RESULTS 24
#define DISCOVERY_MAX_PAGES 1000
#define DISCOVERY_MAX_FILE_BYTES (2LL * 1024 * 1024 * 1024)
typedef enum { DISCOVERY_ALBUM, DISCOVERY_TRACK } discovery_kind_t;
enum { DISCOVERY_SEARCH_ALBUMS=1, DISCOVERY_ALBUM_TRACKS=2, DISCOVERY_DOWNLOAD=4,
       DISCOVERY_ARTIST_RELEASES=8, DISCOVERY_PAGED_RESULTS=16 };
typedef struct {
    discovery_kind_t kind;
    char id[128];                 /* Archive item identifier */
    char title[192], artist[128], album[192];
    char filename[256];           /* Exact source filename; never a local path */
    char url[1024], artwork[256], license[128];
    char format[16], md5[33];
    long long bytes;
    bool downloadable;
} discovery_item_t;
typedef struct {
    discovery_item_t items[DISCOVERY_MAX_RESULTS];
    int count;
    bool truncated;
} discovery_results_t;
unsigned discovery_capabilities(void);
const char *discovery_provider_name(void);
/* Blocking, worker-thread only. Result storage is caller owned. No UI calls. */
bool discovery_search(const char *query, discovery_results_t *out, char *error, size_t error_size);
bool discovery_album(const char *identifier, discovery_results_t *out, char *error, size_t error_size);
/* One-based bounded pages; artist_only filters the catalog's creator field.
 * Creator text is source metadata, not a globally unique artist identity. */
bool discovery_search_page(const char *query, bool artist_only, unsigned page,
                          discovery_results_t *out, char *error, size_t error_size);
bool discovery_album_page(const char *identifier, unsigned page,
                         discovery_results_t *out, char *error, size_t error_size);
/* Parsing helpers also used by deterministic offline regression tests. */
bool discovery_parse_search(const char *json, discovery_results_t *out, char *error, size_t error_size);
bool discovery_parse_album(const char *identifier, const char *json, discovery_results_t *out, char *error, size_t error_size);
bool discovery_parse_album_page(const char *identifier, const char *json, unsigned page,
                               discovery_results_t *out, char *error, size_t error_size);
bool discovery_license_allowed(const char *url);
bool discovery_item_valid(const discovery_item_t *item);
#endif
