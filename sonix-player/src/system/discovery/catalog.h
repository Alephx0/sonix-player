#ifndef SONIX_CATALOG_H
#define SONIX_CATALOG_H
#include "discovery.h"
/* Selection: 0 Archive, 1 Deezer tracks, 2 Deezer albums. Immutable descriptors. */
const discovery_provider_t *catalog_provider(unsigned selection);
/* Exact HTTPS Deezer track/album URLs only; emits a namespaced internal key. */
bool catalog_link(const char *url,char *key,size_t size,bool *album);
bool catalog_parse_deezer(const char *body,discovery_kind_t kind,
                         discovery_results_t *out,char *error,size_t size);
#endif
