# Local Spotify-inspired UI

Branch: `feature/spotify-ui-local` (created from `feature/r1-discovery`).
Status: source removal complete; host and R1 cross-builds pass. No hardware flash.
Changes remain in the working tree; the preexisting dirty work was not reset.

## Removed

101 C/header files were removed from the native source tree. The inventory is
`artifacts/local-ui-removed-files.txt`. This includes SpotBye/SpotiFLAC discovery,
provider matching, transfer queues, importing transfer manifests, provider settings,
authentication and browser/phone verification; legacy Tidal/Qobuz, podcast fetching,
radio/HLS, DLNA fetching, progressive download readers, online lyric lookup,
AutoEQ fetching and OTA downloading were also removed with their callers.
The file-transfer page no longer offers an outbound download action.

The old working source is preserved in
`artifacts/branch-backups/before-ui-local-20261102-151210.zip`. Historical online
reports are reference history, not current capabilities. The build mirror removes
stale source/objects and the old helper so deleted modules cannot re-enter a build.
Packaging excludes stale streaming credentials/helper files from old overlays.

Local playback, local imports, embedded/sidecar metadata and lyrics, local artwork,
EQ presets, queues, playlists, likes, folders and library organization remain.
Bluetooth, AirPlay reception, SonixLink, local uploads, clock synchronization and
optional Last.fm scrobbling are separate retained device functions. Last.fm reads
user-configured `lastfm.api_key`/`api_secret`; bundled provider credentials are gone.
TLS certificate checks and build-time dependency fetching remain intact.
The Bluetooth firmware-loader argument containing "download" is hardware driver
initialization, not a music acquisition path.

## UI changes

- Local Search: All, Songs, Albums, Artists, Playlists and Lyrics filters, embedded
  thumbnails, bounded asynchronous queries and stale-response rejection.
- Artist search indexes individual credits instead of combined collaboration
  strings. Existing indexes migrate through the dirty queue without rescanning
  audio. Artist result subtitles use "Artist" and retain release artwork.
- Track selection stays in mini-player preview. Album/playlist pages share the
  existing collection renderer; cover taps open Liked Songs and All songs.
- Now Playing and Queue keep the existing upward sheet flow, icon utilities and
  uninterrupted playback. Empty online routes and settings have been removed.

## Verification ? 2026-11-02

Evidence: `artifacts/local-ui-20261102/summary.json` and sibling logs/captures.
All renders are actual LVGL 9.6.0 SDL frames at 480 x 800; these are candidates,
not approved visual baselines.

| Check | Result |
| --- | --- |
| Host build | PASS |
| MIPS R1 build + glibc ABI ceiling 2.22 | PASS |
| Source and native-symbol boundary check | PASS |
| Queue, offline library, collections, discography, Home, incremental import | PASS, six native suites |
| Local FLAC / CBR / VBR / no-Xing MP3 duration, seeking and EOF | PASS |
| Packaging guard | PASS, two tests; packer shell syntax checked |
| Collection journey | PASS, 71 assertions |
| Local Search journey | PASS, 14 assertions |
| Cover taps, preview, Now Playing and Queue sheet | PASS, 14 assertions |

Screenshots inspected: grouped and filtered Search, individual artist results,
discography, playlist preview, Now Playing, and Queue. Model checks include old
search-index migration, playlist persistence, non-destructive collection deletion,
priority queues, accent-insensitive search and lyric indexing.

The stripped target is 9,327,660 bytes; executable hashes are in summary.json.
Host RSS was about 37?40 MiB in the recorded journeys; this does not establish
R1 RAM usage. Connected-device RAM, touch, standby, physical controls and DAC
behavior remain unverified. No firmware image was generated or flashed.

## Continue UI work

Use `docs/UI_SPEC.md`, `docs/DESIGN_SYSTEM.md`, `docs/UI_TEST_MATRIX.md` and
`docs/UI_DECISIONS.md`. Expand native interaction coverage for playlist membership,
artist selection, full lyrics and library customization without reintroducing
provider dependencies or redesigning unrelated completed screens.

## Audiobooks Home feed ? 2026-11-02

Audiobooks beside Music now renders native Spotify-style shelves and cover cards
inside Home. Cover/title opens details; Play/Resume restores the local saved
position while keeping the mini-player visible. Scan books refreshes the existing
audiobook index in place. The selected category persists across restarts.

Host/R1 builds and ABI check pass. The populated and empty native UI journeys
pass 18 and five assertions respectively; inspected 480 x 800 captures are in
`artifacts/audiobooks-feed-20261102/`. Device behavior is not yet verified.

## Collection usability and visual audit ? 2026-11-02

Long playlists now have a draggable fast scrollbar. Pull down at the top of an
album, playlist or Liked Songs to reveal page-local filtering and view sorting.
Title, artist and album matches stay within that collection; result playback
uses the full original context. Shuffle is a passive green toggle applied by Play.
The reference UI no longer offers random-track queue insertion.

Fixed inherited dark header text, enlarged missing-art placeholders, legacy album
layout overrides and filter/heading overlap. Reference keyboard accents match the
player green. Native UI journeys passed 173 checks across the core music screens,
plus collection and queue model suites; host/R1 builds and ABI check passed.
Evidence: `artifacts/collection-find-20261102/`. Legacy utility screens and hardware
behavior remain outside this rendered audit; screenshots are candidate evidence.
