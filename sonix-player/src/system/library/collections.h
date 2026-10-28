#ifndef SONIX_COLLECTIONS_H
#define SONIX_COLLECTIONS_H
#include <stdbool.h>
#include <stddef.h>
typedef enum { COLLECTION_PLAYLIST=1, COLLECTION_ALBUM, COLLECTION_ARTIST, COLLECTION_PODCAST } collection_kind_t;
typedef enum { COLLECTION_RECENT, COLLECTION_ADDED, COLLECTION_NAME, COLLECTION_CREATOR } collection_sort_t;
typedef struct {
    collection_kind_t kind;
    char key[512], title[256], creator[256];
    bool saved;
    int pin;
    long long added, recent;
} collection_item_t;
/* UI-thread metadata only. Each operation closes its database before returning,
 * so card export cannot leave an open handle behind. Never deletes audio. */
void collections_init(const char *card_root);
bool collections_put(collection_kind_t kind, const char *key, const char *title, const char *creator, bool saved);
bool collections_get(collection_kind_t kind, const char *key, collection_item_t *out);
bool collections_touch(collection_kind_t kind, const char *key);
bool collections_forget(collection_kind_t kind, const char *key);
bool collections_rename(collection_kind_t kind, const char *key, const char *new_key, const char *title);
bool collections_pin(collection_kind_t kind, const char *key, bool pin);
/* filter 0 means all; pages are bounded to 48 records. Pins precede sort. */
int collections_list(int filter, collection_sort_t sort, int offset, collection_item_t *out, int capacity);
unsigned collections_revision(void);
#endif
