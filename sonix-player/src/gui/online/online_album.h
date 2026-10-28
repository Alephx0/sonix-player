#ifndef SONIX_ONLINE_ALBUM_H
#define SONIX_ONLINE_ALBUM_H
#include "src/gui/shell/gui.h"
#include "src/system/discovery/discovery.h"
typedef void (*online_track_cb_t)(const discovery_item_t *item);
bool online_album_active(void);
void online_album_open(gui_config_t *cfg,const discovery_results_t *results,unsigned page,
    online_track_cb_t track,lv_event_cb_t page_cb,lv_event_cb_t artist_cb);
#endif
