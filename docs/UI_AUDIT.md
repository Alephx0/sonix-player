# Whole-product UI and functionality audit

## 2026-11-06: Waveform and filtered Library repairs — verified

Latest evidence: `artifacts/wave-library-20261106/` (junction to D:).
Temporary device executable `/mnt/sd_0/sonix_wave_verified`, MD5
`bd35eadd4e14a39541e9c614a26fae0d`, is running on the connected R1. Playback left
paused, waveform enabled for inspection. `/usr/bin/sonix_player` remains MD5
`b1658cf81526fa916308cdd3460fa815`; no firmware flash occurred. Reboot restores
that previously flashed executable.

Subsequent owner request: this same verified executable is packaged as
`artifacts/package-wave-20261106/r1.upt` and staged at `/mnt/sd_0/r1.upt` for the
owner to flash. See its package report and hardware-and-packaging record for
checksums, installed-base readback, partition bounds, and the preserved old
update. Packaging does not change the running/flashed distinction above.

Verified changes:

- Waveform preparation starts during compact-player playback, before the full
  view is opened, then prepares one next queue entry. Persistent cache survives
  restart; cached work bypasses the audio-start quiet wait. UI reads the actual
  playback path independently of asynchronous artwork.
- Waveform uses a compact 32-pixel envelope with explicit controls height. The
  cover yields space; no control reordering or overflow on repeated toggles at
  Normal or Large. Sibling indices are only used before slider reparenting.
- Library rebuild resolves layout before grip geometry, clamps travel and clears
  stale handles before deleting the old list. Artists-only jumps remain inside
  the viewport and never pin rows or open Now Playing.
- Library sorting and the A-Z bubble share Unicode case/accent folding and the
  existing leading-article convention. Mixed lower/uppercase names interleave.
- Latest user instruction supersedes the earlier conservative fallback: untyped
  7+ local-track releases are estimated Albums; fewer are Singles & EPs. Whole
  release counts apply even to a performer credited on only one track. Explicit
  tags, suffixes and local overrides win; source audio is unchanged.

Evidence and checks:

- `test_waveform.log`: real FLAC current/next preparation before any UI read;
  cached advance and new-process restart each returned in 10 ms on the host.
- `test_discography.log`, `test_collection_library.log`: PASS native release
  1/6/7 boundaries, tagged precedence, featured credits, override/reset and
  persistence, mixed-case paginated sorting and existing collection operations.
- `evidence/ui-wave-library-c_fih6gn/report.json` (Large) and
  `evidence/ui-wave-library-hvxe33ks/report.json` (Normal): PASS repeated preference
  changes, seek input, geometry and filtered Artists grip. Screenshots inspected.
- `evidence/ui-responsiveness-zyozmlmt/report.json`: PASS playing tab navigation,
  estimated Singles category, manual Album override, reset and return journey.
- `device-wave-first.png`, `device-next-cached.png`: real R1 waveform present on
  opening and on advancing to a previously prefetched song, with intact controls.
- `device-artists-grip.png`, `device-artists-jump-ready.png`: reachable grip and
  actual jump to index 808 of 842 artists; a return jump to 0 also logged.
- `device-bad-artists.png`, `device-singles.png`: user's local Bad Bunny releases
  visibly split into estimated Albums and Singles & EPs.
- `device.log`: initial uncached real analyses took 4.3–5.0 seconds after quiet
  time; these completed in the background. Playback logged zero write recoveries
  during navigation, waveform decoding, and Library dragging. Available memory
  stayed around 21–22 MB in sampled playback logs.
- Host and MIPS builds completed; target ABI check passed against glibc 2.22.
  Existing unrelated format-truncation warnings remain in the target build log.

Limits: a never-analyzed track cannot have an authentic instantaneous waveform;
first analysis still yields to audio startup. Cache hits/prefetch remove the delay
for prepared tracks, not every arbitrary new selection. No whole-library eager
decode was added. The 842-artist distant jump still logged a 734 ms GUI pass while
rebuilding its row window; this batch fixes reachability, not smartphone frame
pacing. Startup scan and large cover decode pauses remain known separate work.
Unusual release-duration rules intentionally do not override the owner's explicit
7-track fallback; partial imports can therefore be estimated incorrectly.

The earlier `ui-wave-library-m8uyryb_` failure is retained: its assertion selected
the occluded mini-player title rather than the foreground title. Corrected the
test's title selection, inspected pixels, and reran both display sizes.

## 2026-11-06: Home/Search latency and artist release filters

Historical record: the latest repair above supersedes the type fallback and
temporary executable details in this section.

Measured on the connected R1 with 2,384 indexed tracks, Large UI (384 x 640
layout, 480 x 800 panel). The earlier flashed build performed a 1,513 ms Home
build and a 1,766 ms GUI pass while preparing the hidden Library page. Expensive
Home metadata and Search browse queries now run in bounded workers. Library
catalog synchronization also runs in a worker, with synchronized mount-path and
revision access. Home retains its current feed during refresh; Home/Library
retain bounded covers across tab switches. Tab preference writes are coalesced.
Library skips text normalization for an empty search, only registers changed
playlists, and initially renders one page instead of two.

Final temporary device executable: `/mnt/sd_0/sonix_perf_verified`, MD5
`f0f45dd55e7aee74eaa6a9be320b6321`. Source build and native 480 x 800 SDL checks
passed. The prior flashed executable remains unchanged (MD5
`b1658cf81526fa916308cdd3460fa815`); reboot returns to that version.

Evidence in `artifacts/perf-discography-20261106/` (junction to D:):

- `timing-verified.txt`: 12 tab changes, first visible body update 113–251 ms.
- `timing-verified-warm.txt`: 12 tab changes, 113–319 ms, median 122.5 ms.
  Timings include a 70 ms injected touch hold; they measure framebuffer changes
  in rows 55–620, not physical touch accuracy or time to load every thumbnail.
- `perf-verified-r1.log`: Library initial refresh 111 ms (previous intermediate
  build 439 ms); first Home data request/shell 40 ms. An asynchronous feed apply
  still produced a 500 ms GUI pass during the startup scan. Wake and startup
  remain slower than ordinary navigation. Do not claim smartphone frame pacing.
- `perf-final-r1.log`, `timing-final-playing.txt`: predecessor with the same
  playback changes kept audio active during navigation; logged 0 ALSA write
  recoveries. Final SDL navigation tests also check playback continuity.
- `ram-evidence/ui-responsiveness-5un5rfxm/report.json`: PASS repeated tab taps
  while playing, untyped Albums visibility, release-type sheet bounds at Large,
  EP reclassification, navigation/reopen, and reset to file tags. Pixels inspected.
- `ram-evidence/library-retry-el_gc760/report.json`: PASS 120-playlist scroll
  grip, distant jump, return to top, no accidental pin or Now Playing opening.
- `discography-final.log`, `collections-final.log`: native tests PASS explicit
  types, featured/owner credits, full dates, local override persistence/reopen,
  folders, sorting, pinning, and deletion preserving audio.
- Initial `home-cards-test.log` and `local-search-test.log`: PASS native Home
  card/shelf interactions and local Search journeys; screenshots inspected.

All inspected card files lacked release-type tags. Owned untyped releases now
stay in Albums with "Type not tagged"; they are not claimed to be verified
albums. Explicit tags/suffixes or a local correction distinguish Singles & EPs.
Featured On follows owner/performer credits. A release long press opens the type
correction sheet; changes never edit the audio file.

Test-environment correction: C: filled and caused WSL I/O/read-only failures.
Generated artifact folders were preserved on D: using junctions. Ubuntu-24.04
was moved with `wsl --manage --move` to `D:\justi\Hiby R1 Build\Ubuntu-24.04`.
The final simulator fixtures ran on a private tmpfs and their evidence was copied
to D:. Filesystem-stalled runs are not accepted as UI regressions or passing checks.

Remaining scope: cold-start/feed-apply frame time, database contention during
scans, and physical-touch feel under prolonged use require further profiling.
Audiobook recommendation loading was not optimized in this batch. No whole-product
completion, universal latency guarantee, or new flash is claimed.

## 2026-11-05: Device repair follow-up

Scope is the user's 17-item repair batch, retaining the prior native feature work.
New corrections: mini-player-only expansion without invisible edge fallback;
vertical scroll ownership for row swipes; conservative unknown release types;
storage worker synchronization, category classification and bounded layout.
Regression fixtures now exercise the real fast-scroll grip with 120 indexed
playlists. See the top entries of UI_SPEC, UI_RESEARCH, UI_TEST_MATRIX and
UI_DECISIONS for the current contract and methods; older round-4 count-inference
notes are superseded.

Verified follow-up evidence:

- `ui-device-fixes-g5rfg7_e/report.json`: PASS settings/name/picture selector,
  SD-update no-image feedback, nested-page scroll restoration, storage categories
  and lower content. Native 480 x 800 captures inspected at Large.
- `ui-device-fixes-bdlp34_s/report.json`: PASS combined Library, player and
  settings journey while audio plays. Includes intentional swipe-to-pin/unpin,
  accidental gesture prevention, artwork X/outside dismissal, centered format,
  dots position, seek, queue spacing, Sound overflow and deeper return paths.
- `ui-device-fixes-m68vxref/report.json`: PASS waveform/line preference, native
  Rock preset chip, custom EQ naming/save and restart persistence.
- `ui-device-fixes-pkiu43g4/report.json`: PASS final player run, including both
  left- and right-end seek targets. Copied to `simulator-player` locally.
- `bugfix-discography.log`, `bugfix-import.log`: native model PASS, including
  explicit release tags/unknown records and small thumbnail warm-cache identity.
- `test_eq_presets.py`: required genre curves and fresh-process custom preset
  reload/delete PASS.
- `artifacts/device-fixes-20261105/`: connected R1 screenshots and configuration
  backup. Quick panel crop/brightness and Library scroll checked on device.

UI report paths above are under `/home/justi/sonix-r1-build/artifacts` (WSL).
Reports/captures copied to `artifacts/device-fixes-20261105/simulator-combined`,
`simulator-settings`, and `simulator-controls` for access from the workspace.
The temporary SD executable is running; flashed firmware has not been updated.
Do not claim the whole product audited, every hardware gesture accepted, or menu
frame-time targets met from these checks. User physical-touch feedback is pending.


The objective is a polished Spotify Mobile-like R1 product, not preservation of
whatever code currently exists. Prior passing checks establish specific historical
observations only. Begin with an honest inventory and reproduce present behavior.

## Study and inventory

Read the instruction set, current source and relevant history. Establish the
active branch, build/run commands, target configuration, data/queue ownership,
shared components, and test fixture limitations. Map all reachable entry points,
menus, nested sheets, screens, settings and back paths. Inspect actual rendering
and action callbacks; do not infer functionality from labels or filenames.

Include Home and recommendations, Search, Library, albums/playlists/Liked Songs,
artist credits/discography, Now Playing/artwork/lyrics, queue and transport,
audiobooks/chapters/bookmarks/speed/resume, keyboards, scan/import, settings,
connectivity/audio/device controls, confirmations, status and failure recovery.
Include any additional retained feature discovered in the source or running UI.
For each route, identify legacy Sonix presentation or navigation escaping the
Spotify-style shell. Backends may remain native; user-facing legacy handoffs may not.

## Audit record

Use one row per meaningful journey; split complex journeys into state-specific
records. Add findings during inspection rather than declaring broad areas done.

| ID / journey | Entry and all destinations / source owner | Spotify research | Current finding and severity | Required R1 behavior | Status | Render / interaction / model / device evidence | Next action |
| --- | --- | --- | --- | --- | --- | --- | --- |
| INVENTORY-2026-11-02 | Entire reachable product: 237 routes in 8 subsystems plus 12 critic additions ([route inventory](records/route-inventory-2026-11-02.md)) | [Research records](records/spotify-research-2026-11-02.md) | 164 legacy, 32 mixed, 35 Spotify-styled, 3 unreachable; roughly 230 major findings. Root cause: Home ⋮ opens the legacy `tools_screen` grid and the gear opens legacy `settings_screen`; every nested utility uses the Adwaita palette (#242424, blue accent), `settingsrow` cards and gridpage tiles, and lacks the mini-player/nav | Shared-component conversion: reference palette, Spotify settings rows/header, footer on pushed pages, one Settings entry, sheets/snackbar for choosers and notices | Implementing (palette, settings kit, hubs, footer, device menu, snackbar, confirm dialog, option-menu sheet, audiobook lists and note type simulator verified via `--journey device-settings`; remaining: Browse music index pages, multi-select toolbar chrome (its Add to playlist now uses the shared sheet), Control centre swipe-back, remaining page-specific cards, first run, time picker, quick panel, power menu, shell localization; layout chooser hidden 2026-11-03) | Source tracing; first-level legacy captures inspected at 480 x 800 (Home, tools grid, Settings, Appearance, Wireless, More, legacy Audiobooks, classic Music) | Work through the priority list below |
| COL-DELETE-AUDIO-FILES | Music settings → Library → Allow deleting tracks; long-press select in a collection → red bin | Spotify never deletes local files | Unlinks audio files from the collection UI, contradicting the data-integrity rule. Severity critical (data loss) | Collection UI never deletes audio | Simulator verified (functional: option and bin absent in reference; `collections` 72 checks) | `medialist.c` `delete_offered()`, `musicsettings.c` | Device check pending |
| HOME-PLAY-EXPANDS-NP | Home card Play / Resume | Pending dated evidence | Music Play from Home expands Now Playing, contradicting the preview rule (book Play does not). Major | Play keeps the current view; only the mini-player expands Now Playing | Simulator verified | `--journey home-play` (7 checks), capture `home-play-preview.png` inspected | Research dated Home Play evidence (still a research gap) |
| NP-CONTEXT-KIND | Now Playing header after any collection Play | Spotify Android community thread titles name "Playing from Playlist" (medium) | Header said only "PLAYING FROM" for non-album sources. Polish | "PLAYING FROM PLAYLIST/ARTIST/SEARCH/GENRE/YOUR LIBRARY" with the name; kind persisted with the restored queue | Simulator verified | `player-core` (context kind before/after restart), native `priority_queue` kind lifecycle | Device check |
| NP-SHUFFLE-RESTORE | Shuffle + Play, then restart | Spotify resumes the same session order (behavior) | Fresh deal from `playlist_shuffle_restart()` was never persisted, so a restart restored the previous deal with the current song last (empty queue). Major | Persist the re-deal immediately | Simulator verified | `player-core`: queue order and position identical after restart | Device check |
| LAYOUT-CHOOSER-ESCAPE | Music settings → Now Playing → Player layout pills | n/a (product rule: no legacy fallback) | Pills switched to the Standard/Alternative/Studio players and the Adwaita theme. Major | Chooser, local-only note and cover-accent hidden in the reference layout; cover accent never recolours the reference player | Simulator verified (source + render) | `device-settings` unchanged pass; capture review | None |
| SEARCH-SEE-ALL | Search results "See all results"; Browse music search button | 9to5mac/RouteNote 2021 search filter launch: chips narrow results to one kind | "See all results" and Browse music's search opened the legacy search page; result count text was capped/misleading; chips only filtered six rows. Major | Chips re-query one kind (30 bounded rows); a full group ends in "See all songs/albums/artists" opening the shared collection list for `LIBRARY_FILTER_SEARCH`; Browse music search opens the Search tab | Simulator verified | `local-search` 30 checks (chip shows 30 songs, See all opens the 252-match list, plays with kind SEARCH, preview kept) | Playlists/lyrics have no full list yet |
| SHARED-TITLE-OFFSET | Every `settingsrow_title` page in the reference layout | n/a | Title measured before layout (height 0), so every reference heading sat 16 px below the back chevron. Polish (product-wide) | Use the font line height | Simulator verified | probe rect: title centred on the 56 px back button | None |
| SONG-ROW-ARTIST | Song lists (search lists, playlists, Liked Songs, All songs) | Spotify song rows show the artist under the title | Artist line depended on a legacy setting and was missing on most lists. Polish | Always shown on reference song rows except an artist's own lists | Simulator verified | `local-search-all-songs`, `collections` | None |
| BULK-ADD-PICKER | Collection long-press selection → add; Files & folders → Add to playlist | Spotify "Add to playlist" sheet (prior NP-REF/collection research) | Opened the legacy playlist picker page. Major | Shared Add to playlist sheet listing destinations ("N songs", New playlist, Liked Songs, playlists); background add with snackbar | Simulator verified | `player-core`: sheet over collection, destination playlist count +1 | Device timing for large sets |
| PLAYLISTS-LEGACY-PAGE | Search "Playlists" tile; Browse music → Playlists | Spotify playlists live in Your Library | Both opened the legacy playlists page. Major | Your Library with the Playlists chip | Simulator verified | `local-search`: tile lands on Your Library, `library_filter` 1, chip toggles back | None |
| NP-AUDIOBOOK | Now Playing while a book plays: ⋮ Chapters/Bookmarks/Summary, speed, right control | Spotify support "Purchase audiobooks" (mobile: Playback speed "1x", Skip 15 s, Sleep timer, dots menu); Engadget 2024-11-21 (audiobook sleep timer) | Chapters/bookmarks/summary opened legacy card pages and closed the player; speed was a dim gauge; the music repeat button showed on books; header had no context. Major | Sheets over Now Playing (current chapter green, tap seeks; bookmarks with remove; summary); "1x" speed label with titled sheet; moon sleep timer (5–60 min, End of chapter, green when set); header names the current chapter | Simulator verified | `--journey audiobook-player` 36 checks; captures inspected | Skip size stays the R1 setting (10 s default, Spotify 15 s); device check |
| SETTINGS-MUSIC-UNREACHABLE / PROFILE-MENU | Home/Search/Library avatar → device sheet → Settings; Music settings | 9to5Google 2023-10-04 (side menu from the profile picture); SlashGear 2025-09-14 (Settings categories) | The avatar opened a bottom sheet, not Spotify's left drawer; Settings root was legacy groups with no Music/Playback/Audio entry, so crossfade, ReplayGain and EQ were reachable only from the classic library or Now Playing. Major | Left profile drawer (Listening stats, Recents, Settings, then R1 destinations); Settings as Spotify categories with summaries; Music settings split into Playback, Audio quality and Your Library | Simulator verified | `--journey profile-activity` (53 checks), `device-settings` (51); captures profile-drawer, settings-categories, settings-playback, settings-audio-quality inspected | Device touch check of the drawer drag |
| SPOTIFY-IOS-CAPTURES | Header, Settings, drawer, Recents, Listening stats | User-supplied Spotify iOS captures 2026-11-04 (records/spotify-ios-captures-2026-11-04.md) | Settings rows lacked icons and chevrons; no settings search, tiles or private session; drawer narrow; stats four-week totals; Recents ungrouped; titles left-aligned, bare back chevron | Rebuilt to the captures (see UI_DECISIONS 2026-11-04 afternoon) | Simulator verified | `profile-activity` (73 checks) | Device touch of the back disc and tiles; Wi-Fi/Bluetooth tiles untested on hardware (host has neither) |
| SPOTIFY-IOS-QUEUE | Queue sheet, Now Playing time, Now Playing menu | Same captures | No tiles, Edit Queue, countdown or radio | Built (UI_DECISIONS 2026-11-04 evening) | Simulator verified, also on the user's 2,387-track card | `player-core` | Section headings ("Now playing", "Next from") remain where Spotify has none; Edit Queue shows the sheet close beside its own |
| SPOTIFY-IOS-ROUND3 | Search bar, Your Library filters, playlist pills | Same captures | Dark search field; no filter sheet; no playlist pills | Built (UI_DECISIONS 2026-11-04 night) | Simulator verified; filters and health also on the user's card | `profile-activity`, `collections`, `library-create` | Genre counts depend on tags (the user's card has almost none) |
| R1-ADDITIONS | Now Playing badge and Sound sheet, Play next, Save queue as playlist, Library health | research section B | Not present | Built | Simulator verified | `player-core`, `profile-activity` | Sound sheet effects only audible on hardware; toast after Save queue not visible over Now Playing |
| R1-ADDITIONS-2 | Now Playing menu, Settings > Playback, Home | research section B; Spotify newsroom 2025-09-05 (Snooze) | No resume of long tracks, no lyric timing, no album shuffle, no artist shelf, no Snooze | Built (UI_DECISIONS 2026-11-05 later) | Simulator verified | `player-core`, `profile-activity` | Album grouping cost on a several-thousand-track queue on the R1 CPU |
| DEVICE-R1-1 | Scan, library lists, search, Home, every page (size) | user report after the first flash; copy of the device index | Freezes, Scan at 0, duplicate albums, slow search, everything small | Fixed in code (UI_DECISIONS 2026-11-05 device) | Simulator verified; R1 not yet re-flashed | `display-size`; full regression | Frame times, scaling cost and touch latency on the R1; first background rebuild of the card's search index |
| FINAL-BATCH | Now Playing menu, queue, Liked Songs, Search Browse, playlist options, Create | captures; research sections A and B | Not built | Built (UI_DECISIONS 2026-11-05 night) | Simulator verified | `player-core`, `profile-activity` | Bluetooth output change and its sound swap on the R1; composer tags appear after a rescan |
| SEARCH-CLEAR | Search: clear the field, tap away | User report 2026-11-05 | Rows and chips stayed after clearing | Empty search runs on clear and on keyboard close | Simulator verified | `local-search` | -- |
| NOW-PLAYING-CLEAN | Now Playing | User report 2026-11-05; iOS captures | Banded gradient, two-line header, cramped lyrics card | Flat tone, name-only header, lyric line above title | Simulator verified, also on the user's card | `player-core`, `audiobook-player` | Panel colour accuracy on the R1 |
| SPOTIFY-IOS-OPEN | Now Playing cards, playlist recommendations, Home details | Same captures | Up next/Credits cards under Now Playing (Now Playing does not scroll); "More like" headers with an avatar on Home. Built since: Recommended songs, Home type labels | Open | Not started | -- | -- |
| SCAN-LYRIC-WAIT | Settings > Your Library > Scan on a real card | User's card 2026-11-04 | Page sat on Cancel through the lyric pass (17+ min after 7.5 min of tracks) | Reports finished after the tracks; lyrics continue behind | Simulator on the real card | real-card run (scratch script) | R1 time for both passes on its own SD reader |
| SETTINGS-CONTROLS-ALIGN | Settings > Controls row; Controls > Buttons slider | User report 2026-11-04 | Controls icon wider than the icon column ran into its name; slider knob cut in half at the row edge | Icons boxed at 28 px, own icon; track inset by half a knob | Simulator verified | `profile-activity` (icons), capture | -- |
| RECENTS-STATS-AUTOPLAY | Drawer → Recents, Listening stats; end of a queue | Spotify support Listening stats; Engadget 2024-11 Recents (snippet); newsroom 2025-09-05 Autoplay | No listening history, stats or Autoplay in the product. Gap | Recents by day, four-week stats, Autoplay default on with a Playback switch | Simulator verified | `profile-activity` (Recents grouping/art/playback; stats totals and ranks; Autoplay on/off/restart) | Device timing of the autoplay query on a large card |

### Priority order (2026-11-02)

1. Critical/behavioral: COL-DELETE-AUDIO-FILES, HOME-PLAY-EXPANDS-NP.
2. Shared visual system: reference palette in `theme.c`, Spotify header/rows/toggles/sliders in `settingsrow.c`, footer on pushed pages.
3. Entry and hubs: one Settings entry replacing ⋮ + gear; re-home tools (Devices, Library and storage, Audio, More) as Spotify list pages.
4. Notices and choosers: confirm, toast and popover choosers done (2026-11-03); the collection multi-select toolbar remains.
5. Remaining per-route findings in the inventory (first run, keyboard chrome, quick panel, language coverage of the shell, audiobook deeper controls).

Statuses: Not audited, Researching, Confirmed gap, Implementing, Simulator verified,
Device verified, or Blocked (state the exact missing input/resource). Separate
visual, functional, persistence and hardware status when they differ.
Severity: critical (data loss/crash/playback corruption), major (broken journey,
legacy handoff, unreadable/overlapping content), or polish (inconsistent hierarchy,
spacing, icons, feedback or motion). No feature is accepted solely from screenshots.

## How to challenge an implementation

- Compare the live journey with researched mobile behavior and explicit user intent.
- Check every action actually changes the correct state; verify through database,
  queue or playback observations as appropriate, not only a success toast.
- Inspect duplicates, missing/corrupt metadata/artwork, long titles/credits, empty
  and large collections, no card, interruption, rapid input, and restart behavior.
- Check all screens for clipping, contrast, alignment, hit areas, scroll bounds,
  keyboard occlusion, stale artwork, inconsistent controls and conflicting gestures.
- Distinguish available data from saved membership and playback/download history;
  the active branch has no download workflow to restore.
- Review shared causes before patching individual screens. Keep behavior and visuals
  synchronized across every consumer of a changed component.
- Reassess tests that assert old behavior; update them only against a documented
  requirement and add checks that expose the real regression.

## End-product acceptance

Every reachable retained feature has an inventoried, researched, intentionally
adapted UI, no unexplained native-screen escape, and explicit acceptance evidence.
No known critical/major issue is left hidden behind a completion claim. Core and
secondary journeys maintain consistent styling, playback continuity, data integrity,
and navigation. Performance is bounded by design and tested on device when available.
Unverified device behavior must remain identified; simulator success is not device
acceptance. A count of tests or approximate parity percentage is not a substitute.

For each handoff, list changes, reference decisions, tests and captures actually
inspected, unresolved findings, and the next concrete verification step. Preserve
historical evidence without treating it as an approved visual baseline.

## 2026-11-05: Hardware touch coordinate calibration repair

Owner reports that tapping near the middle selects controls farther down/right
on the flashed R1. Windows currently exposes the card as USB storage only;
no live evdev range or installed configuration has yet been read.

The bundled LVGL evdev implementation accepts ABS_MT_POSITION_X/Y events but
probes calibration using only ABS_X/Y. On MT-only panels this leaves zero
ranges and passes physical coordinates through, clamped to logical bounds.
This is incorrect for the Large display layout (384 x 640 drawn at 480 x 800).
The application now reads a valid legacy axis pair or falls back to a valid MT
pair and explicitly supplies those ranges to LVGL. Rotation reverses that same
measured range instead of replacing it with framebuffer pixel dimensions.
No guessed per-device offset or axis swap is applied.

Targeted native range-selection tests and the R1 build/glibc <= 2.22 ABI gate
pass. This is a source/build fix, not yet a verified repair on the connected R1.
ADB access and physical center/corner taps, scrolling, both display sizes and
rotation remain pending. No firmware or files on the device/card changed.
Developer access in the reference UI: Settings -> Device -> Developer options
-> ADB (unlock via five build-number taps in System).

### 2026-11-05: Connected-device confirmation and temporary test

ADB now connected as serial `dev` (the stock gadget reports Nexus_4/mako;
kernel and device files identify the R1). Kernel 4.4.94-sonix. Input `hyn_ts`
is /dev/input/event1, direct-touch. Live EVIOCGABS probe: legacy ABS_X/Y both
succeed but return min=max=0; MT X is 0..720 and MT Y is 0..1280. This confirms
the calibration defect on the connected hardware, rather than merely a theory.
The startup log confirms Large layout: 384x640 drawn to a 480x800 framebuffer.

Transferred the built player to /tmp/sonix_player_touchfix, verified its MD5,
stopped the old reboot supervisor before stopping the player, and launched a
new supervisor with this temporary executable. PID 2070 at test start. The new
log explicitly reports `(0,0)-(720,1280) -> 384x640 logical pixels`.
Firmware is unchanged. Reboot discards /tmp and restores the flashed player.
Owner asked to check center/edge taps and collection scrolling; response pending.

Prepared, but did not flash or copy onto the card, a package at
artifacts/touch-calibration-20261105/r1.upt. SHA-256:
27875db179c43a4982cdf110bd0f2f139b82d09bc81c09d1842761f8846bd65d
Uses the previous device candidate's kernel/assets/packing recipe plus the
corrected binary. Independently verified OTA chains, extracted player hash,
unchanged kernel, LF scripts, and rootfs 44,683,264 bytes < measured rootfs
partition 47,185,920 bytes (/proc/mtd: mtd2 02d00000). This does not prove recovery
or alternate-slot capacity (rootfs2 is smaller); keep prior recovery limitations.
Evidence: artifacts/touch-calibration-20261105/device-live.log and
package-report.json. Explicit user authorization remains required for flashing.

### 2026-11-05: Accepted repair build installed permanently

The later repair pass and actual touch-coordinate corrections supersede the
temporary calibration candidate above. The owner accepted the temporary build
and explicitly requested packaging and flashing. Installed that exact executable
through the SD-card updater and verified normal boot, the installed executable
hash, complete kernel/rootfs readback hashes, retained preferences, profile image,
cover navigation, and Library scrolling. No provider/download paths were restored.
See `docs/records/r1-repairs-2026-11-05.md` for package identity, rollback backups,
evidence, and remaining performance uncertainties. The previous unflashed touch
candidate is obsolete; use `artifacts/flash-ui-20261105/r1.upt` for this device.
