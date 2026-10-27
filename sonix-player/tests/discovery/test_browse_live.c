/* Opt-in live checks: no audio downloaded. */
#include "src/system/discovery/discovery.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *tr(const char *s) { return s; }
const char *config_get(const char *section, const char *key, const char *fallback) {
    (void)section; (void)key; return fallback;
}
int main(void) {
    discovery_results_t *page = calloc(1, sizeof(*page));
    assert(page);
    char error[192], first[128], artist[128];
    if (!discovery_search_page("Bach", false, 1, page, error, sizeof(error)) || !page->count) {
        fprintf(stderr, "FAIL first live page: %s\n", error); return 1;
    }
    snprintf(first, sizeof(first), "%s", page->items[0].id);
    snprintf(artist, sizeof(artist), "%s", page->items[0].artist);
    printf("PASS catalog page 1: %d eligible releases, more=%d, first=%s\n", page->count, page->truncated, first);
    if (!discovery_search_page("Bach", false, 2, page, error, sizeof(error)) || !page->count) {
        fprintf(stderr, "FAIL second live page: %s\n", error); return 2;
    }
    assert(strcmp(first, page->items[0].id));
    printf("PASS catalog page 2: %d eligible releases, more=%d, first=%s\n", page->count, page->truncated, page->items[0].id);
    if (!*artist || !discovery_search_page(artist, true, 1, page, error, sizeof(error)) || !page->count) {
        fprintf(stderr, "FAIL artist release list: %s\n", error); return 3;
    }
    printf("PASS creator release list: %s, %d eligible releases\n", artist, page->count);
    free(page);
    return 0;
}
