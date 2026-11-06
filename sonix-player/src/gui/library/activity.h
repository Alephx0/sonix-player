#ifndef ACTIVITY_H
#define ACTIVITY_H

#include "src/gui/shell/gui.h"

/* Spotify's personal pages, from the profile menu: Recents (what was played,
 * by day) and Listening stats (the last four weeks). */
void activity_init(gui_config_t *cfg);
void activity_open_recents(void);
void activity_open_stats(void);
// Composers from the COMPOSER / TCOM tags; one opens their tracks.
void activity_open_composers(void);

#endif /* ACTIVITY_H */
