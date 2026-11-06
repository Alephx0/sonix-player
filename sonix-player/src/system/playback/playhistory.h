#ifndef PLAYHISTORY_H
#define PLAYHISTORY_H

#include <stdbool.h>


// A track has been loaded. `title` and `artist` are what it is to be shown by;
// `kept_out` says it is not to go into the history. With `waiting`, it only
// counts once it starts playing (playhistory_resumed): the track a reboot put
// back, paused, was not played again just by coming back.
void playhistory_started(const char *path, const char *title, const char *artist, bool kept_out, bool waiting);

// The track loaded has started playing after a pause.
void playhistory_resumed(void);

// The track loaded is left. Writes it down, once: a second call before the
// next track starts does nothing.
void playhistory_left(void);

#endif // PLAYHISTORY_H
