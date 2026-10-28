#include "src/system/core/config.h"
#include "src/system/library/library.h"
#include "src/system/library/metadata.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char **argv) {
	assert(argc == 2);
	char path[768], db[768], cfg[768];
	song_metadata_t m;
	snprintf(cfg, sizeof(cfg), "%s/config.ini", argv[1]);
	config_init(cfg);
	snprintf(path, sizeof(path), "%s/identity.mp3", argv[1]);
	metadata_read(path, &m);
	assert(!strcmp(m.release_type, "ep"));
	assert(!strcmp(m.release_date, "2025-08-19"));
	assert(!strcmp(m.artists, "Nova; Guest"));
	assert(!strcmp(m.artist_ids, "id-nova; id-guest"));
	assert(!strcmp(m.album_artist_ids, "id-nova"));
	snprintf(db, sizeof(db), "%s/.local/library.db", argv[1]);
	assert(library_open(db));
	for (int i = 0; i < 6; i++) {
		snprintf(path, sizeof(path), "%s/Music/release%d.flac", argv[1], i);
		metadata_read(path, &m);
		if (i == 1)
			assert(!strcmp(m.release_date, "2024-11-02"));
		assert(library_import_download(path));
		for (int wait = 0; library_scan_running() && wait < 3000; wait++)
			usleep(10000);
		assert(!library_scan_running());
	}
	char credits[16][256];
	snprintf(path, sizeof(path), "%s/Music/release4.flac", argv[1]);
	assert(library_track_artists(path, credits, 16) == 2);
	library_release_t r[12];
	int n = library_discography("Nova", 0, 0, 12, r);
	assert(n == 5);
	assert(!strcmp(r[0].title, "Later"));
	assert(!strcmp(r[1].title, "Earlier"));
	assert(!strcmp(r[2].title, "Untyped"));
	assert(r[2].category == 1 && !r[2].type[0]);
	assert(r[3].category == 2 && !strcmp(r[3].title, "Short"));
	assert(r[4].category == 3 && !strcmp(r[4].title, "Guest") && !strcmp(r[4].artist, "Other"));
	assert(library_discography("Nova", 1, 0, 12, r) == 3);
	assert(library_discography("Nova", 2, 0, 12, r) == 1);
	assert(library_discography("Nova", 3, 0, 12, r) == 1);
	assert(library_discography("Nova", 1, 1, 1, r) == 1 && !strcmp(r[0].title, "Earlier"));
	snprintf(path, sizeof(path), "%s/identity.mp3", argv[1]);
	assert(library_import_download(path));
	for (int wait = 0; library_scan_running() && wait < 3000; wait++)
		usleep(10000);
	assert(library_discography("Nova", 2, 0, 12, r) == 2 && !strcmp(r[0].title, "Identity"));
	assert(library_discography("Guest", 3, 0, 12, r) == 1 && !strcmp(r[0].title, "Identity"));
	library_close();
	assert(library_open(db));
	assert(library_discography("Nova", 0, 0, 12, r) == 6);
	library_close();
	puts("PASS discography: embedded FLAC/ID3 credits, full-date ordering, explicit single/EP, conservative unknown and featured classification, paging and persistence");
}
