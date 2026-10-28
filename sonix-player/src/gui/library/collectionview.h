#ifndef COLLECTIONVIEW_H
#define COLLECTIONVIEW_H
#include "src/gui/shell/gui.h"
#include "src/system/library/collections.h"
void collectionview_build(lv_obj_t *parent, gui_config_t *cfg);
void collectionview_refresh(void);
void collectionview_manage_track(const char *path);
/* Opens save/follow and pin actions without changing playback. */
void collectionview_collection_options(collection_kind_t kind,const char *key,const char *title,const char *creator);
void collectionview_create_playlist(lv_event_t *e);
bool collectionview_track_saved(const char *path);
#endif
