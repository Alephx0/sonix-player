#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H
#include <stdbool.h>
void playlistview_open(const char *name,bool edit);
void playlistview_add_songs(void);
void playlistview_renamed(const char *old_name,const char *name);
#endif
