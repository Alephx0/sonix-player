#ifndef COLLECTIONVIEW_H
#define COLLECTIONVIEW_H
#include "src/gui/shell/gui.h"
#include "src/system/library/collections.h"
void collectionview_build(lv_obj_t *parent, gui_config_t *cfg);
void collectionview_refresh(void);
// The same, only when something it lists has changed since it was drawn.
void collectionview_refresh_if_stale(void);
// Whether something it lists has changed since it was drawn.
bool collectionview_stale(void);
/* Your Library's chip: 0 All, 1 Playlists, 2 Albums, 3 Artists. */
void collectionview_set_filter(int filter);
void collectionview_manage_track(const char *path);
/* Several tracks at once: the same sheet, listing destinations to add them to. */
void collectionview_add_tracks(const char *const *paths, int count);
/* Opens save/follow and pin actions without changing playback. */
void collectionview_collection_options(collection_kind_t kind,const char *key,const char *title,const char *creator);
void collectionview_search(lv_event_t *e);
void collectionview_create_menu(lv_event_t *e);
void collectionview_create_playlist(lv_event_t *e);
bool collectionview_track_saved(const char *path);
// A playlist page pill: 1 Add songs, 2 Edit tracks, 5 Name & details (rename).
void collectionview_playlist_command(const char *name, int action);
/* Your Library from its top (its tab tapped again). */
void collectionview_scroll_top(void);
#endif
