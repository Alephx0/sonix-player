# UI acceptance and evidence

## Waveform and filtered Library regression (2026-11-06)

- `python3 tools/test_waveform.py <repo>`: real FLAC background preparation of
  current and next track before rendering, cached advance during audio startup,
  and persistent cache in a fresh process. Host timing is not an R1 measurement.
- `python3 tools/test_ui_wave_library.py <repo> <fixture-card> [125|100]`:
  repeated Line/Waveform changes through Settings, 480 x 800 renders, title/seek
  ordering, no control overflow, functional waveform seeking, filtered Artists
  grip bounds and repeated jumps without pinning or opening Now Playing. Use a
  disposable card containing at least 40 indexed artists.
- Native discography suite: explicit types beat counts; untagged 1/6/7-track
  classification, featured performer on only one track of a seven-track album,
  override/reset persistence, date ordering and pagination.
- Native collections suite: mixed ASCII and accented case ordering through
  paginated Library browse, shared upper-case index initial and article handling.

## Responsiveness and release classification (2026-11-06)

`python3 tools/test_ui_responsiveness.py <repo> [--card <isolated-fixture>]`
uses real pointer input at Large, validates repeated tab navigation while audio
plays, opens an artist through Search, long-presses a release, checks the type
sheet fits the screen, moves an estimated Single to Albums with an override,
reopens the artist, and resets to Automatic. The optional card must be a disposable
fixture: this test deliberately clears the fixture release tags and overrides.
The native discography test separately verifies close/reopen persistence.

Also run Library gesture regressions after changing row prefetch/cache lifetime,
and native collection tests after changing browse SQL. Record device latency
separately from host behavior. A timed framebuffer change is not proof every
cover is ready, physical touches are accurate, or animations meet a frame target.
See the November 6 audit for exact accepted reports and remaining startup pauses.

## Device repair regressions (2026-11-05)

`python3 tools/test_ui_device_fixes.py <repo> [all|library|player|settings|controls]`
uses the same native Session as the collection harness. Default is Large (125%);
screenshots remain 480 x 800. It seeds 120 actual indexed playlists through the
native fixture (`SONIX_FIXTURE_PLAYLISTS`), not M3Us added after import.

- Library: actual grip present, jump to distant records, no pin mutation, no
  unexpected Now Playing/action-sheet opening, reselect tab to return to top.
- Player: collection preview playback, scroll ownership, badge/corner bounds,
  focused-art dismissal, seek from zero, queue section spacing, Sound overflow,
  deeper audio/settings and details return paths while playing.
- Settings: name editing, picture picker, SD updater preflight (never install),
  nested-page scroll retention, storage categories and lower content access.
- Controls: waveform/line preference, EQ chip, custom naming/save and restart.
- `tools/test_discography.py`: FLAC/ID3 tags, featured credits, full dates,
  paging/reopen, unknown types retained without count guessing.
- `tools/test_eq_presets.py`: required genre presets apply expected bands;
  custom curve survives a separate process; invalid path names rejected.
- `tools/test_library_import.py`: includes bounded small thumbnails and byte-
  identical warm cache results, plus cache invalidation when artwork appears.

Driver detail: `offline_ui_session.py drag`'s final argument is the initial
hold duration, NOT total gesture duration. A 400 ms hold can deliberately open
a row's long-press menu. Use a short hold for scrolling. Probe node order is
not visual vertical order, and displayed labels can be ellipsized at Large.


Run native host build with `tools/build_wsl.py`; run the actual SDL executable
on a private 480 × 800 Xvfb display using `tools/test_ui_collections.py`.
Inspect its PNGs before accepting a change. JSON probes observe live LVGL bounds
and playback state; xdotool supplies real mouse/touch-compatible input. Do not
substitute browser renders. LVGL version is read from the checked-out headers.

| Journey | Acceptance | Automation |
| --- | --- | --- |
| Album / playlist | Same cover/title/action geometry, scrolling rows and footer | UI collection harness |
| Portrait / playback preview | Liked Songs and All songs open via cover; track/Play/Shuffle stay collapsed; explicit mini-player expands | `test_ui_collections.py --journey preview-local`, full collection journey |
| Empty / long / large playlist | No overlap, useful empty action, bounded widget count | UI collection harness |
| Playlist menu / editing | Add, remove, reorder, rename persist without file deletion | UI harness + `test_collection_library.py` |
| Artist selection | Individual selectable credits, bounded text | `test_collection_library.py`, rendered artist evidence |
| Discography | Correct owner, featured, type and chronology | `test_discography.py` |
| Queue | Priority before context; shuffle/repeat preserve invariants; restart | `test_priority_queue.py` |
| Offline library / lyrics | Membership, saved state, individual artist search, migration, lyric search and timestamping | `test_offline_library.py` |
| Home Audiobooks | In-place shelves/grid, details sheet, saved-position resume in preview, rescan isolation, category persistence, uninterrupted browsing | `--journey home-audiobooks` |
| Empty Audiobooks | Useful empty state, in-place scan completion, return to Music | `--journey home-audiobooks-empty` |
| Home Play preview | Shelf Play starts the collection, Home and mini-player stay visible, only the mini-player expands Now Playing | `--journey home-play` |
| Device menu / settings kit | One profile-position button opens the device sheet; Settings, Screen and Devices use reference rows, 52 × 32 switches, no Adwaita accent pixels, footer mounted with the list above it, real toggle state, tab navigation back, uninterrupted playback | `--journey device-settings` |
| Core player | Repeat toggle and green dot, "PLAYING FROM PLAYLIST" header and persisted context kind, title/artist/bar spacing, queue sheet header and "Next from <context>" across restart, exact shuffle deal restored after restart, details above the player with Back, Shuffle+Play keeps repeat, Add to queue → manual FIFO, Add to playlist → membership sheet, selection Add to playlist → shared destination sheet (+1 song in the playlist), playback unchanged | `--journey player-core` + `test_priority_queue.py` |
| Audiobook Now Playing | "PLAYING AUDIOBOOK" + current chapter, "1x" speed label and titled speed sheet, moon sleep timer (sheet, End of chapter, green when set, off), Chapters/Bookmarks/Summary as sheets over Now Playing (current chapter green, chapter and bookmark seek, add/remove bookmark, empty state), playback never interrupted | `--journey audiobook-player` (fixture book0 carries Vorbis CHAPTERnnn tags) |
| Search chips and See all | Songs chip lists 30 bounded rows of one kind; full group ends in "See all songs"; the shared list holds every match, plays with context kind SEARCH in preview; no legacy search route | `--journey local-search` |
| Library create and swipes | Create sheet (compact, described kinds, no Done), naming (question, default hint, Cancel/Create pills), new playlist opens with the empty-state invitation and picker, options icons, rename, delete confirmation; shared row swipe: content travel, grey→green/red strip at 96 px, commit vs spring-back, no play/select/scroll side effects, queue removal | `--journey library-create` |
| Home and Search cards | Shortcut tiles (56 px, two columns, no badges, green playing title), shelf carousels (edge to edge, gutter-aligned first card, peeking next card, flick and snap, vertical drag still scrolls, no side effects), long-press card menu, Search top-genre carousel and Browse all grid with tilted sleeves | `--journey home-cards`; `home-play` (menu Play keeps preview) |
| Long-track resume, lyrics timing, album shuffle, Snooze, Hide in this playlist | Resume within 85-100 s after leaving at 90 s (threshold lowered to 60 s); finished track starts over; lyrics delay steps, persists over a restart, resets; shuffled playlist keeps each album in one run; Snooze and Unsnooze rows | `--journey player-core` |
| Storage, Daylist | Storage found by Search settings with Card, FLAC and Library index rows; a Daylist card named for the moment in Made for you | `--journey profile-activity` |
| Home artist shelf, Shuffle by album switch | "Discover more from" over the most played artist with album cards; the Playback switch | `--journey profile-activity` |
| Device round 4 (2026-11-05) | Dots in the corner, FLAC text centred, queue heading on one line, Sound sheet scrolls to All audio settings, focused cover with X, quick panel two lines of four with no cropping, Settings card and scroll kept across a subpage, About name and picture, Storage breakdown, EQ chips; untagged singles/EPs by size; swipe-up only from the mini-player (device) | `--journey device-settings`, `--journey profile-activity` (Storage breakdown, device card), `tools/test_discography.py` (untagged by size), scratchpad round4shots.py / round4b.py at Large, dev.py + taptime on the R1 |
| Device round 3 (2026-11-05) | No grey block after the lyrics (pixel beside the sleeve before and after, both sizes); a long lyric line shown whole in pieces; the catalogue scrolls whole with Featured On filled and the chips scrolling apart from the page (copy of the device index); tab switches timed on the R1 with injected taps | `--journey player-core` (catalogue has no More), scratchpad lyricscheck / longlyric / disccheck / taptime + round3.sh (device) |
| Device round 2 (2026-11-05) | Artists split at commas with exceptions, credits re-split on re-filing (copy of the device index); repeated tracks shown once; Spotify Now Playing at Large with lyric line and controls clear of the action row; Your Library scrolls down and back to the same top (copy of the device index); library rows span the list | `--journey collections` (row width), scratchpad devcheck2 / libscroll_test / npcheck (not repo tests) |
| Device round 1 (2026-11-05) | Display size: Standard lays out 480x800, Large 384x640 drawn to the 480x800 panel, choice saved, restart offered; Home not rebuilt on a return; a full scan of a copy of the device index starts counting at once (scratchpad fullscan_test.sh, not a repo test); foreign-root rows forgotten (copy of the device index) | `--journey display-size`; all other journeys; `offline_library` |
| Remaining captures and R1 list | Next skips hidden songs; Liked Songs genre chips; Up next / Credits sheet with composer; Composers browse; Choose cover; sound kept per output (probe override); Smart Shuffle picks in and out of the queue; smart playlists Most played and Genre | `--journey player-core`, `profile-activity` |
| Search clear | Clear button and tapping away from an empty field bring Browse back with no rows or chips | `--journey local-search` |
| Now Playing cleanup | Header names only the context; controls and shortcut row at their new places; book header without capitals | `--journey player-core`, `audiobook-player` |
| R1 additions on Now Playing | Format badge text, Sound sheet switches and normalization pills, Save queue as playlist stores the playing song and what follows | `--journey player-core` |
| Smart Filters, Library health | Filter button, genre chips with counts, green genre chip filters and clears; health rows with counts open examples | `--journey profile-activity` |
| Playlist pills and Recommended songs | Add, Edit, Sort in one row under the header; Recommended songs under the last row, (+) adds to the playlist (`profile-activity`); Edit opens the editor; songs still swipe to queue below the pills | `--journey collections`, `library-create` |
| Queue sheet, countdown, Go to radio | "Playing <context>", Edit pill, Shuffle/Repeat/Timer tiles along the bottom, Repeat tile cycles, Edit Queue with round selectors and Move up/Remove, time left as "-m:ss", Go to radio plays "<song> Radio" with more songs | `--journey player-core` |
| Profile drawer, Recents, Listening stats, Settings, Autoplay | Avatar and 432 px left drawer with a raised profile card (scrim and drag close), Recents by day grouped by context (folded, indented songs, covers, preview playback), weekly stats (minutes, top artist and song tiles, past weeks, a week's ranked artists with split credits and songs that play as a list), centred titles, Settings quick tiles, category icons and chevrons, Private session, Search settings, Playback and Audio quality pages, Autoplay on/off/persistence | `--journey profile-activity`; `device-settings` |
| Collection find / fast scrolling | Passive green Shuffle then Play, pull-down search, title/artist/album scope, clear/sort, full-context result playback, 257-track end and return | `--journey collection-find` + `test_collection_library.py` |
| Home | Local ranking, pins, import dates, resume and conditional shelves | `test_home_feed.py` |
| Local decoding | FLAC, CBR/VBR MP3, files without Xing, seeking and EOF | `test_local_decode.py` |
| Import | Incremental additions and changed metadata | `test_library_import.py` |
| Sheets | Upward entry, close/scrim/handle dismissal, nested continuity | UI harness and targeted captures |
| Now Playing | Preview expansion, lyrics preview, queue icon/sheet, playback preservation and downward dismissal | `--journey preview-local`; further journeys remain incremental |
| Library | Filters, sorting, list/grid, pins, folders, persistence | Model suite + targeted simulator interaction |
| Search | Local category filters, thumbnails, empty results, artist destination, preview playback | `--journey local-search` + model suite |
| Audiobook deeper flows | Chapters, bookmarks, speed, resume and their return paths remain in the shared UI | Inventory + researched native interaction journeys; coverage must be established |
| Settings / audio / connectivity / device utilities | Every reachable destination uses the shared design language; controls execute real supported actions | Route audit + native render/interact checks; historical coverage incomplete |
| Scan / import / missing card / recovery | Cohesive progress, cancellation, errors and recovery without losing audio/data/navigation | Native journeys + model suites; extend to observed gaps |
| Route completeness | No unchanged legacy Sonix screen reachable from a styled entry; every nested route inventoried | `UI_AUDIT.md` inventory plus runtime navigation and source inspection |
| Hardware | RAM, frame timing, standby, physical buttons, DAC | Pending connected-device validation |

Use synthetic/local licensed fixtures only. Include collaboration, release types,
embedded/missing art, long metadata, synchronized lyrics, empty/large playlists.
Snapshots taken after transitions settle; paused deterministic fixtures avoid
progress drift. Candidate screenshots are NOT approved baselines. An optional
baseline directory can compare normalized captures after explicit baseline
selection; never silently overwrite it. Functional assertions remain required.

Performance reports record host RSS and idle CPU over a stated interval and
visible/total widget counts. These are regression signals, not device budgets.
Each completed run writes its actions, assertions, screenshots and metrics to an
artifact report. See `UI_DECISIONS.md` for current evidence and remaining gaps.

Architecture check: `python3 tools/test_local_ui_scope.py .` rejects removed service
modules, provider/helper symbols and runtime transfer entry points. Host renders
are candidate evidence only; connected R1 measurements remain pending.

## Required acceptance procedure

The table is a starting inventory, not an exhaustive list of all reachable routes.
Add discovered features and states to `UI_AUDIT.md`; do not leave them outside the
product audit. For every audited/changed journey:

1. Read current requirements, trace its implementation, and research Spotify Mobile
   appearance and behavior using `UI_RESEARCH.md` before setting expected results.
2. Define concrete visual, functional, persistence, playback and performance checks.
   Include nested destinations and a return path. Record justified R1 differences.
3. Build the real app, render at 480 x 800, interact through actual input, inspect
   screenshots and state, fix defects, and rerun relevant checks. Use probes for
   observation/fixture setup, not to substitute for validating a user-facing action.
4. Include adverse states appropriate to the feature: long/missing metadata, empty
   or large collections, corrupt/missing art, rapid gestures, interrupted operations,
   restart, and browsing during playback. Confirm action identity and data integrity.
5. Recheck shared-component consumers. If a test contradicts current requirements,
   justify its correction and retain regression coverage; do not bless broken output.
6. Attach source references, inspected captures, assertion results, and clear
   simulator/device status to the audit record. Log known gaps and blocked checks.

Completion requires both correct behavior and cohesive presentation, including
secondary screens. A build, static screenshot, passing historical suite, or count
of assertions cannot independently establish acceptance. A whole-product claim
requires the complete route inventory and evidence for all applicable gates.
No new approved screenshot baseline is implied by generating captures.

### Touch coordinate regression (2026-11-05)

Linux: from sonix-player run `gcc -Wall -Wextra -Werror -I. tests/touch_calibration.c -o /tmp/sonix-touch-calibration-test` then run that executable.
Covers MT-only, missing/degenerate legacy axes, no mixed protocol pairs,
nonzero legacy minima, unchanged output on failure and reversible rotation.
R1 target/ABI build passed. Physical acceptance remains pending: tap center and
four corners, scroll a collection, test Standard and Large layouts and rotation.
The SDL mouse does not exercise the device evdev calibration path.

### Accepted R1 repair package: installation evidence (2026-11-05)

The owner accepted the later temporary repair build and authorized permanent
installation. Settings > Device > System > Update from SD card > Update completed
on the connected R1. The normal boot executable matches the accepted binary;
kernel and rootfs readback hashes match the verified package. User preferences
survived (only the clock timestamp changed), and post-boot injected touch events
opened Liked Songs by its cover and scrolled Library without expanding Now Playing.
Evidence: `artifacts/flash-ui-20261105/` and
`docs/records/r1-repairs-2026-11-05.md`. This does not independently complete the
full physical corner/rotation matrix or quantitative performance measurements.
