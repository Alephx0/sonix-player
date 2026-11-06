# Offline player UI specification

## 2026-11-05: Device repair clarification

Updated 2026-11-06 (latest explicit request): estimate untagged releases using
the complete local release count: 7+ tracks -> Albums; 1-6 -> Singles & EPs
(1-3 Single, 4-6 EP). Display "(estimated)". This intentionally accepts the risk
of partially imported albums; it supersedes the earlier untyped-in-Albums rule.
Manual corrections, explicit tags, and explicit title suffixes take precedence.
Featured On still follows performer/owner credits, regardless of track count.
Unknown ownership stays separate. Long-press a release to choose Album / Single /
EP or Automatic (tags, then local estimate). Never modify or duplicate audio.

Home recommendation gathering, Search browse metadata, and Library catalog
rebuilding must not block the GUI thread. Show navigation immediately, keep the
previous feed until a bounded fresh snapshot is ready, and apply it at the top
while idle. Do not continuously reorder content under an active scroll.

- Now Playing expansion starts inside the visible mini-player, never from an
  arbitrary list position or an invisible bottom-edge fallback. Vertical list
  scrolling cannot later become a swipe-to-pin gesture during that same press.
- Release type prefers explicit embedded tags or `- Single` / `- EP` suffixes;
  missing types use the owner's count estimate above, labeled as such.
  Owned releases without type tags remain in **Albums**, labeled **Type not
  tagged**. Unknown ownership remains in **Other releases** in All. Local
  corrections can classify a release without changing its audio file. The
  earlier Other-only placement and experimental count inference are superseded.
- Your Library retains filtering, search, sort and list/grid, with a draggable
  fast-scroll grip for long collections and tab-reselection to return to top.
  No additional Load more screen is introduced.
- Storage categories describe usage, not disk partitions. The SD card and
  internal system storage are separate. `.m4b` is an audiobook even outside the
  Audiobooks directory; arbitrary hidden directories are not player data.


Authority: requirements visible in this conversation, updated 2026-11-02.
Hardware target: HiBy R1, 480 × 800, 64 MB RAM. Exact connected hardware revision
and installed firmware have not been verified. The user asked to assume latest.

## Product-wide fidelity requirement

This document defines intended behavior, not proof that the implementation meets
it. Study and scrutinize every reachable feature using `UI_RESEARCH.md` and
`UI_AUDIT.md`. Proactively research current Spotify Mobile appearance and function
on the internet for each feature. Record the reference version/platform and
resolve conflicts explicitly. User-specific requirements below remain deliberate
R1 adaptations even when current Spotify differs.

The ENTIRE retained user-facing experience must use the same Spotify-inspired
presentation and navigation. This includes audiobooks and their deeper controls,
settings, EQ/audio configuration, connectivity, library scanning, keyboards,
confirmations and recovery states. A shortcut to an unchanged native Sonix page
does not meet this requirement. Reuse native services and models beneath a
consistent interface; do not mistake backend reuse for UI integration.

For functions Spotify does not provide, design a fitting extension of its visual
and interaction language, with honest local-device behavior. Do not fabricate
cloud recommendations, authentication, streaming, download availability or other
unsupported functions. Keep offline content fully discoverable.

Re-evaluate old geometry and implementation choices using research, renders and
R1 constraints. Existing test passes and historical screenshots are not evidence
of full-product fidelity. Audit secondary routes and settings previously excluded
from scoped checks. No retained feature is exempt merely because it already works.

## Navigation and playback

Home, Search, Your Library are the three primary tabs. A persistent mini-player
sits above navigation. Opening a collection never starts or interrupts audio;
its Play action establishes the automatic context. Collection Shuffle is a passive
toggle (green when enabled); apply it when Play is pressed, without changing the
current song, queue or playback position on toggle. Now Playing expands
upward from the mini-player and collapses downward. Physical transport buttons
and touch controls use the same playback actions.
Starting a track or pressing Play from the reference collection view keeps that
view visible and updates the mini-player (preview mode). Only an explicit tap on
the mini-player expands Now Playing. Collection artwork and names share the same
open action, including Liked Songs and All songs in both List and Grid layouts.

## Collections

Albums and playlists use the SAME collection renderer: centered artwork,
bounded title, creator/artist, metadata, shuffle and play controls, scrolling
track rows, and persistent mini-player/navigation. Playlist-specific actions
belong in the contextual menu, not a separate browsing layout. Empty playlists
retain their header and a usable Add songs action. Playlist edits update the
visible collection immediately. Long names must not overlap controls.

Long collections have a visible draggable scrollbar that jumps across the full
list while retaining pooled rows. Pull down at the top to reveal Find in playlist
(or Find on this page for albums/Liked Songs). Filter only this collection by
track title, artist or album with a short debounce. Offer Clear and Original /
Title / Artist / Album / Added view sorting; Added uses the date the file entered the local library; never rewrite playlist order while filtering
or sorting. A result starts the correct entry in the full collection context.
Remove random-track-to-queue actions from the reference collection UI.

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
use the explicit count fallback above only when release type is absent. No artist-page photo hero, biography, popularity,
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
or light placeholders. Artist recognition uses embedded release art with a neutral fallback.
Use a persistent index, bounded queries, short debounce.

Home category row: All, Music, Audiobooks. Audiobooks is an in-place Spotify-style
Home feed, not a shortcut to the legacy audiobook screen. Show Continue listening,
Recently added, and a paginated All audiobooks cover grid. Cards show title, author,
and saved position/finished status. Cover/title opens a details bottom sheet;
explicit Play/Resume uses the existing local audiobook playback and saved position
without expanding Now Playing. Keep the mini-player and main navigation visible.
Persist the selected category; offer an in-place Scan books action and a useful
empty state for the card's Audiobooks folder.

Home: six pinned/recent/frequent shortcuts; Jump Back In with context resume;
Recently Added based on import time. Conditionally show On Repeat (rolling local
history), metadata mixes, Rediscover, and unfinished spoken content if supported.
Do not show empty recommendations or reorder the feed while scrolling. Card tap
opens, explicit Play starts. All recommendations reference existing local files.

## Verification boundaries

The simulator renders production LVGL widgets, but host RSS/timing do not prove
64 MB device performance, touch fidelity, DAC behavior, or firmware compatibility.
Hardware tests and approved pixel baselines remain separate acceptance gates.
Target dimensions and the 64 MB budget are project constraints, not a measurement
of a connected device. Verify exact hardware assumptions against official HiBy
sources and build configuration; document CPU/ABI, display/input, storage and
memory implications before hardware-dependent changes. Preserve a responsive UI
and uninterrupted playback under indexing, artwork loads and large libraries.
See the audit's end-product gates before claiming the system is complete.

## Local-only branch

`feature/spotify-ui-local` replaces the SpotBye parity objective. No online catalog,
provider matching, music transfers, download manager, provider authentication or
browser/phone verification belongs in the Sonix/LVGL application. Removed services
include legacy Tidal/Qobuz, podcast fetching, radio/HLS, DLNA progressive files,
online lyric lookup, AutoEQ fetching and OTA downloading. Local EQ presets,
embedded/sidecar lyrics, indexed artwork, queues and playlist data remain usable.

Search is entirely local: All, Songs, Albums, Artists, Playlists and Lyrics.
Local search runs off the GUI thread with a bounded result/artwork pool and drops
stale responses. Selecting a track keeps preview mode; artist and release results
open the established local destinations without replacing playback.

Device connectivity (Bluetooth, AirPlay reception, SonixLink, local file upload),
clock synchronization and optional Last.fm scrobbling are separate from music
acquisition. TLS certificate checks remain intact. Last.fm uses user-configured
`lastfm.api_key`/`api_secret`; bundled streaming credential infrastructure is removed.
Build-time dependency fetching is tooling, not a runtime firmware downloader.
