#ifndef SONIX_SEARCH_INDEX_H
#define SONIX_SEARCH_INDEX_H
#include "src/system/db/sqlite3.h"
#include "library.h"
/* Internal helpers, caller owns the library database lock. No file reads. */
bool search_index_schema(sqlite3 *db);
void search_index_put(sqlite3 *db, int kind, const char *key, const char *name,
                      const char *path, const char *text);
void search_index_drop(sqlite3 *db, int kind, const char *key);
int search_index_query(sqlite3 *db, const char *query, int per_category,
                       library_instant_cb_t cb, void *user);
#endif
