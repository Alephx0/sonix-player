# Offline player UI specification

Authority: requirements visible in this conversation, consolidated 2026-10-28.
Hardware target: HiBy R1, 480 × 800, 64 MB RAM. Exact connected hardware revision
and installed firmware have not been verified. The user asked to assume latest.

## Navigation and playback

Home, Search, Your Library are the three primary tabs. A persistent mini-player
sits above navigation. Opening a collection never starts or interrupts audio;
its Play/Shuffle actions establish the automatic context. Now Playing expands
upward from the mini-player and collapses downward. Physical transport buttons
and touch controls use the same playback actions.

## Collections

Albums and playlists use the SAME collection renderer: centered artwork,
bounded title, creator/artist, metadata, shuffle and play controls, scrolling
track rows, and persistent mini-player/navigation. Playlist-specific actions
belong in the contextual menu, not a separate browsing layout. Empty playlists
retain their header and a usable Add songs action. Playlist edits update the
visible collection immediately. Long names must not overlap controls.

Your Library is metadata organization, never the Sonix filesystem browser.
Header: Your Library, Search, Create. Filters: All, Playlists, Albums, Artists;
active chip toggles off. All exposes the complete indexed local catalog.
Sort: Recents, Recently Added, Alphabetical, Creator, Custom. List/Grid and sort
persist. Up to four pins stay above ordinary sorted results. Liked Songs is a
permanent independently liked collection. Playlists support create, rename,
add/remove songs, ordering, deletion, and one-level organizational folders.
Collection deletion never deletes audio. Saved albums, followed artists, likes,
playlist membership, pins, and physical file availability remain independent.

Artist text opens individually selectable credited artists. The artist page is
discography ONLY: All, Albums, Singles & EPs, Featured On; newest release date
first with year. Use ALBUMARTIST and structured credits/identifiers for ownership;
do not infer singles merely from track count. No photos, biography, popularity,
or arbitrary name splitting. Uncertain credits must remain visibly uncertain.

## Now Playing and temporary panels

Artwork is primary; tapping enlarges it separately from lyrics. Title opens the
release; artist text opens the artist selector. Plus/check beside the title
reflects membership in ANY playlist, including Liked Songs. No duplicate bottom
save button or heart favorite control. Lyrics is lower-left and Queue lower-right
beneath transport controls. Center controls and glyphs within their hit areas.
Inline lyrics appear only when synchronized lyrics exist; use a subdued surface.
Full lyrics may show an unavailable message or unsynchronized plain text.

Use the shared bottom sheet for queue, playlist selection, track actions, and
artists. Slide vertically up/down, dim the background, round top corners, keep
audio uninterrupted. Nested actions replace sheet content without exposing the
background. Dismiss via handle swipe, close, or scrim. Content scrolling and row
reordering must not dismiss the sheet. Playlist selection includes search, saved
membership, new playlist/name entry, and Done.

## Queue and local discovery

History, manual priority FIFO, and automatic context are independent. Shuffle
and reshuffle affect only the automatic context; disabling restores the remaining
original relative order. Clear removes manual entries only. Support remove,
reorder, immediate play, bulk edit, resume persistence; repeat-all repeats the
context without permanently adding priority entries. Previous restarts after
roughly three seconds, then walks actual history. Repeat-off stops at exhaustion.

Search is incremental, local, accent/case insensitive with grouped songs,
artists, albums, playlists, and lyric excerpts. Results use existing release art
or light placeholders; artist recognition can use release art, not downloaded
artist portraits. Use a persistent index, bounded queries, short debounce.

Home: six pinned/recent/frequent shortcuts; Jump Back In with context resume;
Recently Added based on import time. Conditionally show On Repeat (rolling local
history), metadata mixes, Rediscover, and unfinished spoken content if supported.
Do not show empty recommendations or reorder the feed while scrolling. Card tap
opens, explicit Play starts. All recommendations reference existing local files.

## Verification boundaries

The simulator renders production LVGL widgets, but host RSS/timing do not prove
64 MB device performance, touch fidelity, DAC behavior, or firmware compatibility.
Hardware tests and approved pixel baselines remain separate acceptance gates.
