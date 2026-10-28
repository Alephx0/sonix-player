#include "collections.h"
#include "src/system/db/sqlite3.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
static char root[512];
static unsigned revision;
void collections_init(const char *card_root) { snprintf(root, sizeof(root), "%s", card_root ? card_root : ""); }
unsigned collections_revision(void) { return revision; }
static sqlite3 *open_db(void) {
	if (root[0] != '/' || access(root, W_OK))
		return NULL;
	char path[600];
	snprintf(path, sizeof(path), "%s/.local", root);
	struct stat st;
	if (lstat(path, &st)) {
		if (mkdir(path, 0755))
			return NULL;
	} else if (!S_ISDIR(st.st_mode))
		return NULL;
	snprintf(path, sizeof(path), "%s/.local/collections.db", root);
	if (!lstat(path, &st) && !S_ISREG(st.st_mode))
		return NULL;
	sqlite3 *db = NULL;
	if (sqlite3_open_v2(path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOFOLLOW, NULL) != SQLITE_OK) {
		sqlite3_close(db);
		return NULL;
	}
	sqlite3_busy_timeout(db, 250);
	if (sqlite3_exec(db, "PRAGMA cache_size=-128; CREATE TABLE IF NOT EXISTS collection(kind INTEGER NOT NULL,key TEXT NOT NULL,title TEXT NOT NULL,creator TEXT NOT NULL,saved INTEGER NOT NULL,added INTEGER NOT NULL,recent INTEGER NOT NULL DEFAULT 0,pin INTEGER NOT NULL DEFAULT 0,PRIMARY KEY(kind,key));", NULL, NULL, NULL) != SQLITE_OK) {
		sqlite3_close(db);
		return NULL;
	}
	return db;
}
static bool valid(collection_kind_t kind, const char *key) { return kind >= 1 && kind <= 4 && key && *key && strlen(key) < 512; }
static void bind_key(sqlite3_stmt *s, collection_kind_t kind, const char *key) {
	sqlite3_bind_int(s, 1, kind);
	sqlite3_bind_text(s, 2, key, -1, SQLITE_TRANSIENT);
}
bool collections_put(collection_kind_t kind, const char *key, const char *title, const char *creator, bool saved) {
	if (!valid(kind, key) || !title || strlen(title) >= 256 || (creator && strlen(creator) >= 256))
		return false;
	sqlite3 *db = open_db();
	if (!db)
		return false;
	sqlite3_stmt *s = NULL;
	const char *sql = "INSERT INTO collection(kind,key,title,creator,saved,added) VALUES(?1,?2,?3,?4,?5,?6) ON CONFLICT(kind,key) DO UPDATE SET title=excluded.title,creator=excluded.creator,saved=excluded.saved,added=CASE WHEN collection.saved=0 AND excluded.saved=1 THEN excluded.added ELSE collection.added END,pin=CASE WHEN excluded.saved=0 THEN 0 ELSE collection.pin END";
	bool ok = sqlite3_prepare_v2(db, sql, -1, &s, NULL) == SQLITE_OK;
	if (ok) {
		bind_key(s, kind, key);
		sqlite3_bind_text(s, 3, title, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(s, 4, creator ? creator : "", -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(s, 5, saved);
		sqlite3_bind_int64(s, 6, time(NULL));
		ok = sqlite3_step(s) == SQLITE_DONE;
	}
	sqlite3_finalize(s);
	sqlite3_close(db);
	if (ok)
		revision++;
	return ok;
}
static void read_row(sqlite3_stmt *s, collection_item_t *out) {
	memset(out, 0, sizeof(*out));
	out->kind = sqlite3_column_int(s, 0);
	snprintf(out->key, sizeof(out->key), "%s", sqlite3_column_text(s, 1));
	snprintf(out->title, sizeof(out->title), "%s", sqlite3_column_text(s, 2));
	snprintf(out->creator, sizeof(out->creator), "%s", sqlite3_column_text(s, 3));
	out->saved = sqlite3_column_int(s, 4);
	out->added = sqlite3_column_int64(s, 5);
	out->recent = sqlite3_column_int64(s, 6);
	out->pin = sqlite3_column_int(s, 7);
}
bool collections_get(collection_kind_t kind, const char *key, collection_item_t *out) {
	if (!valid(kind, key) || !out)
		return false;
	sqlite3 *db = open_db();
	if (!db)
		return false;
	sqlite3_stmt *s = NULL;
	bool ok = sqlite3_prepare_v2(db, "SELECT kind,key,title,creator,saved,added,recent,pin FROM collection WHERE kind=?1 AND key=?2", -1, &s, NULL) == SQLITE_OK;
	if (ok) {
		bind_key(s, kind, key);
		ok = sqlite3_step(s) == SQLITE_ROW;
		if (ok)
			read_row(s, out);
	}
	sqlite3_finalize(s);
	sqlite3_close(db);
	return ok;
}
bool collections_touch(collection_kind_t kind, const char *key) {
	if (!valid(kind, key))
		return false;
	sqlite3 *db = open_db();
	if (!db)
		return false;
	sqlite3_stmt *s = NULL;
	bool ok = sqlite3_prepare_v2(db, "UPDATE collection SET recent=?3 WHERE kind=?1 AND key=?2", -1, &s, NULL) == SQLITE_OK;
	if (ok) {
		bind_key(s, kind, key);
		sqlite3_bind_int64(s, 3, time(NULL));
		ok = sqlite3_step(s) == SQLITE_DONE && sqlite3_changes(db) > 0;
	}
	sqlite3_finalize(s);
	sqlite3_close(db);
	if (ok)
		revision++;
	return ok;
}
bool collections_pin(collection_kind_t kind, const char *key, bool pin) {
	if (!valid(kind, key))
		return false;
	sqlite3 *db = open_db();
	if (!db)
		return false;
	sqlite3_stmt *s = NULL;
	const char *sql = pin ? "UPDATE collection SET pin=(SELECT coalesce(max(pin),0)+1 FROM collection) WHERE kind=?1 AND key=?2 AND saved=1 AND pin=0 AND (SELECT count(*) FROM collection WHERE pin>0)<4" : "UPDATE collection SET pin=0 WHERE kind=?1 AND key=?2";
	bool ok = sqlite3_prepare_v2(db, sql, -1, &s, NULL) == SQLITE_OK;
	if (ok) {
		bind_key(s, kind, key);
		ok = sqlite3_step(s) == SQLITE_DONE && sqlite3_changes(db) > 0;
	}
	sqlite3_finalize(s);
	sqlite3_close(db);
	if (ok)
		revision++;
	return ok;
}
bool collections_forget(collection_kind_t kind, const char *key) {
	if (!valid(kind, key))
		return false;
	sqlite3 *db = open_db();
	if (!db)
		return false;
	sqlite3_stmt *s = NULL;
	bool ok = sqlite3_prepare_v2(db, "DELETE FROM collection WHERE kind=?1 AND key=?2", -1, &s, NULL) == SQLITE_OK;
	if (ok) {
		bind_key(s, kind, key);
		ok = sqlite3_step(s) == SQLITE_DONE;
	}
	sqlite3_finalize(s);
	sqlite3_close(db);
	if (ok)
		revision++;
	return ok;
}
bool collections_rename(collection_kind_t kind, const char *key, const char *new_key, const char *title) {
	if (!valid(kind, key) || !valid(kind, new_key) || !title || strlen(title) >= 256)
		return false;
	if (!strcmp(key, new_key))
		return true;
	sqlite3 *db = open_db();
	if (!db)
		return false;
	sqlite3_stmt *s = NULL;
	/* Replacing stale metadata for a previously deleted name retains the source
	 * pin, saved date and recent interaction. Audio and playlist rows live elsewhere. */
	bool ok = sqlite3_prepare_v2(db, "UPDATE OR REPLACE collection SET key=?3,title=?4 WHERE kind=?1 AND key=?2", -1, &s, NULL) == SQLITE_OK;
	if (ok) {
		bind_key(s, kind, key);
		sqlite3_bind_text(s, 3, new_key, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(s, 4, title, -1, SQLITE_TRANSIENT);
		ok = sqlite3_step(s) == SQLITE_DONE;
	}
	sqlite3_finalize(s);
	sqlite3_close(db);
	if (ok)
		revision++;
	return ok;
}
int collections_list(int filter, collection_sort_t sort, int offset, collection_item_t *out, int capacity) {
	if (!out || capacity < 1 || offset < 0 || filter < 0 || filter > 4)
		return 0;
	if (capacity > 48)
		capacity = 48;
	sqlite3 *db = open_db();
	if (!db)
		return 0;
	sqlite3_stmt *s = NULL;
	char sql[500];
	const char *order = sort == COLLECTION_ADDED ? "added DESC" : sort == COLLECTION_NAME ? "title COLLATE NOCASE" : sort == COLLECTION_CREATOR ? "creator COLLATE NOCASE,title COLLATE NOCASE" : "recent DESC,added DESC";
	snprintf(sql, sizeof(sql), "SELECT kind,key,title,creator,saved,added,recent,pin FROM collection WHERE saved=1 AND (?1=0 OR kind=?1) ORDER BY CASE WHEN pin>0 THEN 0 ELSE 1 END,pin,%s,kind,key LIMIT ?2 OFFSET ?3", order);
	int count = 0;
	if (sqlite3_prepare_v2(db, sql, -1, &s, NULL) == SQLITE_OK) {
		sqlite3_bind_int(s, 1, filter);
		sqlite3_bind_int(s, 2, capacity);
		sqlite3_bind_int(s, 3, offset);
		while (count < capacity && sqlite3_step(s) == SQLITE_ROW)
			read_row(s, &out[count++]);
	}
	sqlite3_finalize(s);
	sqlite3_close(db);
	return count;
}
