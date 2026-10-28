#include "collections.h"
#include "library.h"
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
	sqlite3_exec(db,
				 "CREATE TABLE IF NOT EXISTS custom_order(kind INT,key TEXT,rank INT,PRIMARY KEY(kind,key));"
				 "CREATE TRIGGER IF NOT EXISTS custom_order_delete AFTER DELETE ON collection BEGIN DELETE FROM custom_order WHERE kind=old.kind AND key=old.key;END;"
				 "CREATE TRIGGER IF NOT EXISTS custom_order_rename AFTER UPDATE OF key ON collection BEGIN UPDATE OR REPLACE custom_order SET key=new.key WHERE kind=old.kind AND key=old.key;END;"
				 "CREATE TABLE IF NOT EXISTS folder_link(playlist TEXT PRIMARY KEY,folder TEXT NOT NULL);"
				 "CREATE INDEX IF NOT EXISTS folder_children ON folder_link(folder);"
				 "CREATE TABLE IF NOT EXISTS catalog(kind INT,key TEXT,title TEXT,creator TEXT,added INT,year INT,PRIMARY KEY(kind,key));"
				 "CREATE TRIGGER IF NOT EXISTS collection_remove_folder AFTER DELETE ON collection BEGIN "
				 "DELETE FROM folder_link WHERE (old.kind=5 AND folder=old.key) OR (old.kind=1 AND playlist=old.key);END;"
				 "CREATE TRIGGER IF NOT EXISTS collection_move_name AFTER UPDATE OF key ON collection BEGIN "
				 "UPDATE folder_link SET folder=new.key WHERE old.kind=5 AND folder=old.key;"
				 "UPDATE folder_link SET playlist=new.key WHERE old.kind=1 AND playlist=old.key;END;",
				 NULL, NULL, NULL);
	return db;
}
static bool valid(collection_kind_t kind, const char *key) { return kind >= 1 && kind <= 5 && key && *key && strlen(key) < 512; }
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
	if (sqlite3_column_count(s) > 8)
		out->year = sqlite3_column_int(s, 8);
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
	if (!out || capacity < 1 || offset < 0 || filter < 0 || filter > 5)
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

static void fold_sql(sqlite3_context *ctx, int argc, sqlite3_value **v) {
	(void)argc;
	char text[2048];
	const char *s = (const char *)sqlite3_value_text(v[0]);
	library_search_fold(s ? s : "", text, sizeof(text));
	sqlite3_result_text(ctx, text, -1, SQLITE_TRANSIENT);
}
bool collections_catalog_sync(void) {
	sqlite3 *db = open_db();
	if (!db)
		return false;
	char path[600];
	snprintf(path, sizeof(path), "%s/.local/library.db", root);
	sqlite3_stmt *s = NULL;
	bool ok = false;
	if (sqlite3_prepare_v2(db, "ATTACH DATABASE ?1 AS music", -1, &s, NULL) == SQLITE_OK) {
		sqlite3_bind_text(s, 1, path, -1, SQLITE_TRANSIENT);
		ok = sqlite3_step(s) == SQLITE_DONE;
	}
	sqlite3_finalize(s);
	if (ok)
		ok = sqlite3_exec(db,
						  "BEGIN;DELETE FROM catalog;"
						  "INSERT INTO catalog SELECT 2,m.album||char(31)||m.album_key,m.album,MIN(COALESCE(NULLIF(m.album_artist,''),m.artist)),MAX(COALESCE(a.added,m.ctime)),MAX(m.year) FROM music.MEDIA_TABLE m LEFT JOIN music.HOME_ADDED a ON a.path=m.path WHERE m.album<>'' GROUP BY m.album,m.album_key;"
						  "INSERT INTO catalog SELECT 3,c.name,c.name,'',MAX(COALESCE(a.added,m.ctime)),0 FROM music.RELEASE_CREDIT c JOIN music.MEDIA_TABLE m ON m.path=c.path LEFT JOIN music.HOME_ADDED a ON a.path=m.path WHERE c.name<>'' GROUP BY c.name COLLATE NOCASE;"
						  "INSERT INTO collection(kind,key,title,creator,saved,added) SELECT kind,key,title,creator,0,added FROM catalog WHERE 1 ON CONFLICT(kind,key) DO UPDATE SET title=excluded.title,creator=excluded.creator;COMMIT;",
						  NULL, NULL, NULL) == SQLITE_OK;
	if (!ok)
		sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
	sqlite3_close(db);
	if (ok)
		revision++;
	return ok;
}
int collections_browse(int filter, collection_sort_t sort, const char *query, const char *folder, int offset, collection_item_t *out, int capacity) {
	if (!out || capacity < 1 || offset < 0)
		return 0;
	if (capacity > 48)
		capacity = 48;
	sqlite3 *db = open_db();
	if (!db)
		return 0;
	sqlite3_create_function(db, "fold", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, fold_sql, NULL, NULL);
	char sql[1600], folded[512];
	library_search_fold(query ? query : "", folded, sizeof(folded));
	const char *order = sort == COLLECTION_CUSTOM ? "COALESCE((SELECT rank FROM custom_order o WHERE o.kind=c.kind AND o.key=c.key),0),c.title COLLATE NOCASE" : sort == COLLECTION_ADDED ? "COALESCE(a.added,c.added) DESC" : sort == COLLECTION_NAME ? "c.title COLLATE NOCASE" : sort == COLLECTION_CREATOR ? "c.creator COLLATE NOCASE,c.title COLLATE NOCASE" : "c.recent DESC,c.added DESC";
	snprintf(sql, sizeof(sql), "SELECT c.kind,c.key,c.title,c.creator,c.saved,COALESCE(a.added,c.added),c.recent,c.pin,COALESCE(a.year,0) FROM collection c LEFT JOIN catalog a ON a.kind=c.kind AND a.key=c.key WHERE c.kind<>4 AND (c.saved=1 OR a.key IS NOT NULL) AND (?1=0 OR c.kind=?1 OR (?1=1 AND c.kind=5)) AND instr(fold(c.title||' '||c.creator),?2)>0 AND ((?3<>'' AND c.kind=1 AND EXISTS(SELECT 1 FROM folder_link f WHERE f.playlist=c.key AND f.folder=?3)) OR (?3='' AND (?2<>'' OR c.kind<>1 OR c.pin>0 OR NOT EXISTS(SELECT 1 FROM folder_link f WHERE f.playlist=c.key)))) ORDER BY CASE WHEN c.pin>0 THEN 0 ELSE 1 END,c.pin,%s,c.kind,c.key LIMIT ?4 OFFSET ?5", order);
	sqlite3_stmt *s = NULL;
	int n = 0;
	if (sqlite3_prepare_v2(db, sql, -1, &s, NULL) == SQLITE_OK) {
		sqlite3_bind_int(s, 1, filter);
		sqlite3_bind_text(s, 2, folded, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(s, 3, folder ? folder : "", -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(s, 4, capacity);
		sqlite3_bind_int(s, 5, offset);
		while (n < capacity && sqlite3_step(s) == SQLITE_ROW)
			read_row(s, &out[n++]);
	}
	sqlite3_finalize(s);
	sqlite3_close(db);
	return n;
}
bool collections_folder_create(const char *name) {
	collection_item_t old;
	if (!name || !*name || strlen(name) > 200 || strchr(name, '\n') || collections_get(COLLECTION_FOLDER, name, &old))
		return false;
	return collections_put(COLLECTION_FOLDER, name, name, "", true);
}
bool collections_move_playlist(const char *playlist, const char *folder) {
	collection_item_t item;
	if (!playlist || !collections_get(COLLECTION_PLAYLIST, playlist, &item))
		return false;
	if (folder && *folder && !collections_get(COLLECTION_FOLDER, folder, &item))
		return false;
	sqlite3 *db = open_db();
	if (!db)
		return false;
	sqlite3_stmt *s = NULL;
	const char *sql = folder && *folder ? "INSERT INTO folder_link VALUES(?1,?2) ON CONFLICT(playlist) DO UPDATE SET folder=excluded.folder" : "DELETE FROM folder_link WHERE playlist=?1";
	bool ok = sqlite3_prepare_v2(db, sql, -1, &s, NULL) == SQLITE_OK;
	if (ok) {
		sqlite3_bind_text(s, 1, playlist, -1, SQLITE_TRANSIENT);
		if (folder && *folder)
			sqlite3_bind_text(s, 2, folder, -1, SQLITE_TRANSIENT);
		ok = sqlite3_step(s) == SQLITE_DONE;
	}
	sqlite3_finalize(s);
	sqlite3_close(db);
	if (ok)
		revision++;
	return ok;
}
int collections_folder_count(const char *folder) {
	sqlite3 *db = open_db();
	if (!db)
		return 0;
	sqlite3_stmt *s = NULL;
	int n = 0;
	if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM folder_link WHERE folder=?1", -1, &s, NULL) == SQLITE_OK) {
		sqlite3_bind_text(s, 1, folder, -1, SQLITE_TRANSIENT);
		if (sqlite3_step(s) == SQLITE_ROW)
			n = sqlite3_column_int(s, 0);
	}
	sqlite3_finalize(s);
	sqlite3_close(db);
	return n;
}

bool collections_custom_first(collection_kind_t kind, const char *key) {
	if (!valid(kind, key))
		return false;
	sqlite3 *db = open_db();
	if (!db)
		return false;
	sqlite3_stmt *s = NULL;
	bool ok = sqlite3_prepare_v2(db, "INSERT INTO custom_order VALUES(?1,?2,(SELECT COALESCE(MIN(rank),0)-1 FROM custom_order)) ON CONFLICT(kind,key) DO UPDATE SET rank=excluded.rank", -1, &s, NULL) == SQLITE_OK;
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
