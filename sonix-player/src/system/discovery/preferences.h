#ifndef SONIX_DISCOVERY_PREFERENCES_H
#define SONIX_DISCOVERY_PREFERENCES_H
#include "download_queue.h"
typedef struct {
    bool enabled;
    unsigned format;
    download_options_t transfers;
} discovery_preferences_t;
discovery_preferences_t discovery_preferences_defaults(void);
discovery_preferences_t discovery_preferences_load(void);
bool discovery_preferences_save(const discovery_preferences_t *value);
#endif
