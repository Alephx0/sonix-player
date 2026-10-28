# Organized library and individual artist credits

Your Library now operates on indexed recordings and collection relationships.
Its controls no longer open the legacy playlist manager, folder browser, download
manager or generic album/artist index pages. Album track lists and discography
continue to use shared metadata/playback services; audio files remain in place.

## Library controls

- Header: Your Library, library-scoped search, and Create. Search updates while
  typing, folds accents/case, and shows collection types and available artwork.
  The query can remain applied to the main list for paging through more results.
- Filters: All, Playlists, Albums and Artists. Tapping the selected filter clears
  it. Albums/artists include indexed local releases/credits even when unsaved.
  All Songs keeps recordings without album tags discoverable.
- Sorting: Recents, Recently Added, Alphabetical, Creator and Custom. Collection
  menus can place an item first in custom order. Pins precede other collections;
  Liked Songs remains a permanent leading shortcut. Grid/List, sort and filter
  preferences persist in device configuration.
- Long press or three dots opens a contextual bottom sheet. A horizontal right
  swipe pins/unpins, with the existing four-pin limit. Collection selection does
  not change playback, and the mini-player remains above bottom navigation.
- Grid uses large existing cover images and small centered placeholder icons.
  Titles/subtitles have explicit heights and ellipsize inside their own bounds.

## Playlists and folders

Custom playlists have a dedicated page with Play, Shuffle, Add Songs and Edit.
The song picker searches indexed titles/artists, supports paging, and marks
existing membership. Edit exposes Up, Down and Remove for each entry. The
playlist menu offers Rename, Edit, Add Songs, Add to Queue, Move to Folder,
Delete, pinning and custom collection ordering. Home and Search also open this
playlist page.

Create offers Playlist and Playlist Folder. Folders are one level of metadata
relationships, not directories on the microSD card. Folder menus support rename,
pinning, custom ordering and deletion. Deleting a folder returns its playlists
to the Library root; deleting a playlist never removes its recordings. Rename
preserves folder membership, pins and dates. Nested folders are not supported.

Liked Songs remains the automatic collection of individually liked tracks.
Saved albums, followed artists, playlist membership and physical audio files
remain independent. Collection artwork loads through bounded worker pools;
playlist views page through twelve songs and retain four decoded thumbnails.
The library projects indexed albums/artists into its collection catalog when the
music index changes, and refreshes on playlist and collection mutations.

## Artist sheet and metadata

The Now Playing artist sheet shows separate selectable performers. Structured
credits and semicolon-separated values are preferred. Classical role-delimited
lists such as `Singer - Soprano, Singer - Bass` split into individual names;
ordinary commas in artist names remain intact. This is deliberately conservative
for ambiguous unstructured credits. The source files are never rewritten.

Rows have fixed text bounds and zero button padding, preventing multiline labels
from covering neighboring artists. Long labels ellipsize; tapping a performer
opens their discography. A one-time metadata refresh updates older indexed
credits through the existing background scanner.

ALBUMARTIST/structured identifiers still determine album ownership and featured
appearances. When ownership is absent, a credited recording remains reachable
under Featured On with `Credited artist / owner unknown`. An ambiguous credit
with a different known album owner is not promoted to a verified featured role.

## Validation

`tools/test_collection_library.py` exercises the native APIs with original silent
fixtures: individual credits/discography, unsaved local albums/artists, paged song
search, library filtering, custom ordering, folders, rename propagation, pins,
persistence and deletion without audio removal. Existing queue, offline-library,
discography and Home tests run alongside host/MIPS builds and the ABI check.

Native simulator evidence includes:

- [Separate Bach performers](screenshots/collection-artists.png) and [selected discography](screenshots/collection-discography.png)
- [Library list](screenshots/collection-library.png), [artwork grid](screenshots/collection-grid.png), and [filtered search](screenshots/collection-search.png)
- [Add songs](screenshots/collection-add-songs.png), [playlist editing](screenshots/collection-playlist-edit.png), and [reordered songs](screenshots/collection-playlist-reordered.png)
- [Move to folder](screenshots/collection-move.png), [folder contents](screenshots/collection-folder-contents.png), [swipe pin](screenshots/collection-pin.png), and [sorting](screenshots/collection-sort.png)

Regression logs are stored under `evidence/collections-*.log`, with executable
hashes in `evidence/collections-builds.json`. Device flashing and hardware acceptance remain
outside this simulator/native-build update.
