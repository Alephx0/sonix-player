#include "search_index.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Ordinary B-tree posting lists keep the embedded SQLite build small. Each
 * document has one posting per distinct word, never every possible prefix.
 * Prefix lookup is a range seek. Full text stays on the card, capped at 16KiB. */
#define TEXT_MAX 16384
#define WORD_MAX 96
static bool word_byte(unsigned char c) { return c >= 128 || isalnum(c); }
static const char *next_word(const char *p, char word[WORD_MAX]) {
	while (*p && !word_byte((unsigned char)*p))
		p++;
	size_t n = 0;
	while (*p && word_byte((unsigned char)*p)) {
		if (n + 1 < WORD_MAX)
			word[n++] = *p;
		p++;
	}
	word[n] = 0;
	return p;
}
bool search_index_schema(sqlite3 *db) {
	const char *sql = "CREATE TABLE IF NOT EXISTS SEARCH_DOC(id INTEGER PRIMARY KEY,kind INT,key TEXT,name TEXT,path TEXT,text TEXT,folded TEXT,UNIQUE(kind,key));"
					  "CREATE TABLE IF NOT EXISTS SEARCH_WORD(word TEXT,doc INT,PRIMARY KEY(word,doc)) WITHOUT ROWID;"
					  "CREATE INDEX IF NOT EXISTS search_word_doc ON SEARCH_WORD(doc);"
					  "CREATE TRIGGER IF NOT EXISTS search_doc_delete AFTER DELETE ON SEARCH_DOC BEGIN DELETE FROM SEARCH_WORD WHERE doc=old.id; END;"
					  "CREATE TABLE IF NOT EXISTS SEARCH_LYRIC_STATE(path TEXT PRIMARY KEY,stamp TEXT);"
					  "CREATE TRIGGER IF NOT EXISTS search_lyric_delete AFTER DELETE ON MEDIA_TABLE BEGIN DELETE FROM SEARCH_LYRIC_STATE WHERE path=old.path; END;"
					  "CREATE TABLE IF NOT EXISTS SEARCH_DIRTY(path TEXT PRIMARY KEY);"
					  "CREATE TRIGGER IF NOT EXISTS search_track_insert AFTER INSERT ON MEDIA_TABLE BEGIN INSERT OR IGNORE INTO SEARCH_DIRTY VALUES(new.path); END;"
					  "CREATE TRIGGER IF NOT EXISTS search_track_update AFTER UPDATE ON MEDIA_TABLE BEGIN INSERT OR IGNORE INTO SEARCH_DIRTY VALUES(new.path); END;"
					  "CREATE TRIGGER IF NOT EXISTS search_track_delete AFTER DELETE ON MEDIA_TABLE BEGIN DELETE FROM SEARCH_DOC WHERE path=old.path; DELETE FROM SEARCH_DIRTY WHERE path=old.path; END;"
					  "INSERT OR IGNORE INTO SEARCH_DIRTY SELECT path FROM MEDIA_TABLE WHERE NOT EXISTS(SELECT 1 FROM SEARCH_DOC WHERE kind=0 AND key=MEDIA_TABLE.path);";
	return sqlite3_exec(db, sql, NULL, NULL, NULL) == SQLITE_OK;
}
void search_index_drop(sqlite3 *db, int kind, const char *key) {
	sqlite3_stmt *s = NULL;
	if (sqlite3_prepare_v2(db, "DELETE FROM SEARCH_DOC WHERE kind=? AND key=?", -1, &s, NULL) != SQLITE_OK)
		return;
	sqlite3_bind_int(s, 1, kind);
	sqlite3_bind_text(s, 2, key, -1, SQLITE_TRANSIENT);
	sqlite3_step(s);
	sqlite3_finalize(s);
}
void search_index_put(sqlite3 *db, int kind, const char *key, const char *name, const char *path, const char *text) {
	search_index_drop(db, kind, key);
	if (!text || !*text)
		return;
	char *folded = malloc(TEXT_MAX + 1);
	if (!folded)
		return;
	library_search_fold(text, folded, TEXT_MAX + 1);
	sqlite3_stmt *s = NULL;
	if (sqlite3_prepare_v2(db, "INSERT INTO SEARCH_DOC(kind,key,name,path,text,folded) VALUES(?,?,?,?,?,?)", -1, &s, NULL) != SQLITE_OK) {
		free(folded);
		return;
	}
	sqlite3_bind_int(s, 1, kind);
	sqlite3_bind_text(s, 2, key, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(s, 3, name, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(s, 4, path ? path : "", -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(s, 5, text, (int)strnlen(text, TEXT_MAX), SQLITE_TRANSIENT);
	sqlite3_bind_text(s, 6, folded, -1, SQLITE_TRANSIENT);
	bool ok = sqlite3_step(s) == SQLITE_DONE;
	sqlite3_finalize(s);
	sqlite3_int64 id = sqlite3_last_insert_rowid(db);
	if (ok && sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO SEARCH_WORD VALUES(?,?)", -1, &s, NULL) == SQLITE_OK) {
		const char *p = folded;
		char word[WORD_MAX];
		unsigned count = 0;
		while (*p && count++ < 2048) {
			p = next_word(p, word);
			if (!*word)
				break;
			sqlite3_bind_text(s, 1, word, -1, SQLITE_TRANSIENT);
			sqlite3_bind_int64(s, 2, id);
			sqlite3_step(s);
			sqlite3_reset(s);
		}
		sqlite3_finalize(s);
	}
	free(folded);
}
int search_index_query(sqlite3 *db, const char *query, int limit, library_instant_cb_t cb, void *user) {
	if (!db || !query || !cb || limit < 1)
		return 0;
	if (limit > 12)
		limit = 12;
	char folded[256], word[WORD_MAX], upper[WORD_MAX + 1];
	library_search_fold(query, folded, sizeof(folded));
	next_word(folded, word);
	if (!*word)
		return 0;
	snprintf(upper, sizeof(upper), "%s\xff", word);
	/* The first word drives an indexed range, while the full literal phrase
     * filters candidates. At most 512 candidates/category keep a broad single
     * letter from monopolising the library lock. Prefixes of words match. */
	const char *sql = "SELECT name,path,text,folded FROM SEARCH_DOC WHERE kind=?3 AND id IN"
					  " (SELECT w.doc FROM SEARCH_WORD w JOIN SEARCH_DOC d ON d.id=w.doc WHERE w.word>=?1 AND w.word<?2 AND d.kind=?3 LIMIT 512)"
					  " AND instr(folded,?4)>0 GROUP BY CASE WHEN kind IN(1,2,3) THEN name ELSE path END"
					  " ORDER BY CASE WHEN folded=?4 THEN 0 WHEN substr(folded,1,length(?4))=?4 THEN 1 ELSE 2 END,name LIMIT ?5";
	sqlite3_stmt *s = NULL;
	if (sqlite3_prepare_v2(db, sql, -1, &s, NULL) != SQLITE_OK)
		return 0;
	int total = 0;
	for (int kind = 0; kind < 5; kind++) {
		sqlite3_bind_text(s, 1, word, -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(s, 2, upper, -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(s, 3, kind);
		sqlite3_bind_text(s, 4, folded, -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(s, 5, limit);
		while (sqlite3_step(s) == SQLITE_ROW) {
			library_instant_hit_t hit = {.type = (library_search_type_t)kind};
			const char *name = (const char *)sqlite3_column_text(s, 0), *path = (const char *)sqlite3_column_text(s, 1);
			snprintf(hit.name, sizeof(hit.name), "%s", name ? name : "");
			snprintf(hit.path, sizeof(hit.path), "%s", path ? path : "");
			if (kind == LIBRARY_SEARCH_LYRICS) {
				const char *original = (const char *)sqlite3_column_text(s, 2), *f = (const char *)sqlite3_column_text(s, 3);
				const char *at = f ? strstr(f, folded) : NULL;
				/* Folded offsets differ for accented bytes; walk each original
                 * line to find a matching line without misaligned UTF-8 slicing. */
				const char *line = original;
				char sample[512], normal[512];
				while (line && *line) {
					size_t n = strcspn(line, "\n");
					if (n >= sizeof(sample))
						n = sizeof(sample) - 1;
					memcpy(sample, line, n);
					sample[n] = 0;
					library_search_fold(sample, normal, sizeof(normal));
					if (strstr(normal, folded)) {
						snprintf(hit.excerpt, sizeof(hit.excerpt), "%.191s", sample);
						break;
					}
					line = strchr(line, '\n');
					if (line)
						line++;
				}
				if (!hit.excerpt[0] && at)
					snprintf(hit.excerpt, sizeof(hit.excerpt), "%s", original);
			}
			cb(&hit, user);
			total++;
		}
		sqlite3_reset(s);
	}
	sqlite3_finalize(s);
	return total;
}
