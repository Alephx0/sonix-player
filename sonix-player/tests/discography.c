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
	for (int i = 0; i < 19; i++) {
		snprintf(path, sizeof(path), "%s/Music/release%d.flac", argv[1], i);
		metadata_read(path, &m);
		if (i == 1)
			assert(!strcmp(m.release_date, "2024-11-02"));
		assert(library_import_file(path));
		for (int wait = 0; library_scan_running() && wait < 3000; wait++)
			usleep(10000);
		assert(!library_scan_running());
	}
	char credits[16][256];
	snprintf(path, sizeof(path), "%s/Music/release4.flac", argv[1]);
	assert(library_track_artists(path, credits, 16) == 2);
	library_release_t r[12];
	int n = library_discography("Nova", 0, 0, 12, r);
	assert(n == 7);
	assert(!strcmp(r[0].title, "Later"));
	assert(!strcmp(r[1].title, "Earlier"));
    assert(r[2].category == 1 && !strcmp(r[2].title,"Long Untyped") && !strcmp(r[2].type,"Album (estimated)"));
    assert(r[3].category == 2 && !strcmp(r[3].title,"Short"));
    assert(r[4].category == 2 && !strcmp(r[4].title,"Untyped") && !strcmp(r[4].type,"Single (estimated)"));
    assert(r[5].category == 2 && !strcmp(r[5].title,"Six Untyped") && !strcmp(r[5].type,"EP (estimated)"));
    assert(r[6].category == 3 && !strcmp(r[6].title,"Guest"));
    assert(library_discography("GuestOne",3,0,12,r)==1 && !strcmp(r[0].type,"Album (estimated)"));
    assert(library_discography("Nova",1,0,12,r)==3);
    assert(library_discography("Nova",2,0,12,r)==3);
    assert(library_discography("Nova",3,0,12,r)==1);
    assert(library_discography("Nova",1,1,1,r)==1 && !strcmp(r[0].title,"Earlier"));
    assert(library_discography("Nova",1,2,1,r)==1 && !strcmp(r[0].title,"Long Untyped"));
    char untyped[300];snprintf(untyped,sizeof(untyped),"%s",r[0].value);
    assert(!library_release_type_set(untyped,"guessed"));
    assert(library_release_type_set(untyped,"EP"));
    assert(library_discography("Nova",2,0,12,r)==4);
    library_close();assert(library_open(db));
    assert(library_discography("Nova",2,0,12,r)==4);
    assert(library_release_type_set(untyped,""));
    assert(library_discography("Nova",2,0,12,r)==3);

	snprintf(path, sizeof(path), "%s/identity.mp3", argv[1]);
	assert(library_import_file(path));
	for (int wait = 0; library_scan_running() && wait < 3000; wait++)
		usleep(10000);
	assert(library_discography("Nova", 2, 0, 12, r) == 4 && !strcmp(r[0].title, "Identity"));
	assert(library_discography("Guest", 3, 0, 12, r) == 1 && !strcmp(r[0].title, "Identity"));
	library_close();
	assert(library_open(db));
	assert(library_discography("Nova", 0, 0, 12, r) == 8);
	library_close();
	puts("PASS discography: explicit tags win; untagged 1/6/7-track estimates; whole-release count for featured artists; override/reset, dates, paging and persistence");
}
