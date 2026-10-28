/* Seed the real library and playlist APIs for SDL interaction tests. */
#include "src/system/core/config.h"
#include "src/system/library/collections.h"
#include "src/system/library/library.h"
#include "src/system/library/playlists.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
int main(int argc, char **argv) {
	assert(argc == 2);
	char db[1024], config[1024], path[1024];
	snprintf(db, sizeof(db), "%s/.local/library.db", argv[1]);
	snprintf(config, sizeof(config), "%s/config.ini", argv[1]);
	config_init(config);
	assert(library_open(db));
	assert(library_scan_start(argv[1]));
	for (int i = 0; library_scan_running() && i < 12000; i++)
		usleep(10000);
	assert(!library_scan_running());
	playlists_init(argv[1]);
	collections_init(argv[1]);
	const char *names[] = {"Night Drive", "Empty Playlist", "A very long playlist title for evenings beside the ocean and quiet mornings", "Large Playlist"};
	for (int n = 0; n < 4; n++) {
		assert(playlists_create(names[n]));
		collections_put(COLLECTION_PLAYLIST, names[n], names[n], "You", true);
		int count = n == 1 ? 0 : n == 3 ? 257 : 6;
		for (int i = 0; i < count; i++) {
			snprintf(path, sizeof(path), "%s/Music/track%03d.flac", argv[1], i);
			assert(playlists_add_track(names[n], path));
		}
	}
	collections_catalog_sync();
	library_close();
	puts("UI fixture ready: 257 tracks, four playlists");
}
