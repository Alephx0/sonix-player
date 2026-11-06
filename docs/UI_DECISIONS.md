# UI decisions and recurring corrections

## 2026-11-06: Latest release fallback and waveform/Library repairs

The owner now explicitly requests 7+ local tracks as Albums and fewer as Singles
& EPs when tags are absent. This supersedes this document's earlier conservative
fallback. Label estimates, count the entire release (including tracks not
crediting the viewed artist), and retain explicit types/manual overrides.
Automatic replaces the misleading "Use file tags" reset label.

Prepare the current waveform during compact-player playback and then one next
track, with persistent cache reuse. Keep audio-start quiet time and idle-priority
decoding; do not trade USB DAC stability for an instant uncached first analysis.
Use the actual audio path, not asynchronously loaded artwork, to request a shape.
Repeated layout passes may compare slider and waveform indices only while they
are siblings. The compact envelope gets explicit height in the controls budget.

Library filter rebuilds must resolve layout before deriving grip travel. Clear
old handle pointers before deleting rows, clamp travel, and share Unicode folding
between sort order and index labels. Bounded row windows and background catalog
refresh remain required.

## 2026-11-06: Responsive navigation and usable untagged discographies

The R1 measurements confirmed GUI-thread work, not a new coordinate calibration
error, behind the reported Home/Search delays. Move recommendation/catalog
gathering into bounded native workers, publish results to the UI thread, keep
the visible feed while refreshing, and reuse bounded cover slots across tabs.
Avoid normalizing thousands of names when Library search is empty and avoid
building a second invisible page on first entry. Keep the existing touch mapping.

The owner's library has no embedded release types. The earlier Other-only rule
made Albums appear broken. Owned untyped releases therefore remain browsable in
Albums with "Type not tagged". This is an explicit navigation fallback, not an
inference that one local file is a single or that a partly imported release is
an EP. Explicit metadata/title suffixes classify Singles & EPs; ownership and
performer credits classify Featured On. Long-press a release to store Album,
Single, or EP locally, or reset with Use file tags. The database override survives
restart and does not modify the recording.

Device and SDL evidence, measured latency limits, build-environment relocation,
and the temporary executable hash are recorded in the November 6 UI_AUDIT entry.
The tested build is running from SD; it has not replaced the flashed image.

## 2026-11-05: Repair follow-up, conservative release types and gesture ownership

This entry supersedes two experimental choices in the device round-4 record:
untagged releases are **not** inferred from the number of files, and pages with
no mini-player have **no** invisible bottom-edge opening gesture. The user asked
which classification to use; the recommendation was explicit tags/title suffixes,
otherwise Release. Discography All has an Other releases section so those files
remain discoverable; the four existing filter chips remain intact.

The mini-player gesture now checks both axes of its actual visible rectangle and
clears stale tracking on each press. Row swipes latch vertical scroll ownership;
drifting sideways after starting a vertical scroll cannot pin a collection.

Storage's worker publishes completion with release/acquire atomics. Its usage bar
cannot exceed its available width, notes are bounded away from sizes, refresh
retains scroll, `.m4b` is recognized outside the Audiobooks folder, and only known
player directories count as player data. Existing round-4 feature work is retained.

The handoff's grip test did not create indexed playlists: it added M3Us after the
fixture imported them. The fixture now optionally creates native indexed playlists
before opening the UI. Tests also account for Large's logical coordinates,
ancestor clipping, ellipsized labels, creation order versus visual order, and the
driver's initial-hold duration (400 ms is a long press, not a fast scroll).

Native host and R1 target builds pass; target ABI check reports no symbols newer
than glibc 2.22. Discography, small-thumbnail/cache/import, and EQ preset persistence
model tests pass. Device testing runs `/mnt/sd_0/sonix_ui_fixes`, a temporary binary;
no firmware partition was written. Reboot uses the previously flashed firmware.
The original configuration is backed up in `artifacts/device-fixes-20261105`.

On the connected R1 at Large: the Library scroll stayed in Library, the quick
panel's two rows of controls fit, and a brightness drag beginning 20 physical
pixels below the line changed the sysfs level 15 -> 65. It was restored to 15.
Framebuffer evidence is in that artifact directory. The requested profile picture
is visible in the header and remains at `.local/profile.jpg`. Idle process RSS
was 13,268 KiB (one observation, not a peak-playback or performance guarantee).

Remaining limits: simulated pointer events do not establish every physical-finger
edge case; user confirmation was requested. Cold full-art decode and menu frame
timing need broader on-device measurement before claiming performance targets.
The SD updater was exercised only through no-image feedback, never installation.
Existing unrelated trailing whitespace in player.c remains outside this fix.


## 2026-11-02: Whole-product scrutiny and mandatory mobile research

The user requests an instruction set for another LLM to study the entire system
and improve its style and function toward Spotify Mobile on the R1. This expands
the audit beyond the previously tested core music journeys. No old implementation,
screenshot, test result, or completion claim is presumed correct. Historical
records below remain scoped observations; they do not exempt a screen from review.

`AGENTS.md` now requires proactive internet research per feature, source/variant
records in `UI_RESEARCH.md`, and whole-route findings in `UI_AUDIT.md`. Every retained
user-facing feature, including nested audiobook flows, settings and device utilities,
must use the coherent Spotify-inspired interface. Routing to an unchanged native
Sonix screen is incomplete. Native backend reuse is encouraged; legacy presentation
is not an acceptable substitute. Correct evidenced defects in previously completed
screens rather than treating earlier "do not redesign" guidance as a prohibition.

This instruction update does not restore the retired SpotBye/download objective or
override explicit user adaptations such as offline-only discovery, discography-only
artists and passive collection Shuffle. New research informs implementation within
those requirements. Hardware-specific behavior remains subject to R1 validation.

Documentation-only change: no new Spotify research, application behavior, rendered
verification or hardware acceptance is claimed here. The new audit starts unverified;
its records must be populated from actual inspection and source research.

## 2026-11-02: Collection scrolling, local find, and passive Shuffle

Latest request replaces immediate shuffle-and-play on collection headers. Shuffle
is now a green/off pending toggle; it does not touch the active queue or song.
Play applies the selected state. Remove random-track-to-queue from reference
collection headers and play menus; keep deliberate Add to queue actions.

Use one shared collection renderer for albums, playlists and Liked Songs. Long
lists expose a draggable scrollbar over the existing bounded row pool. Disable
the overlapping A-Z rail on reference track lists. Pull down at the top to reveal
a page-local field and Original/Title/Artist/Album/Added sorting. SQLite queries the
collection's indexed metadata; the view stores only row IDs. Preserve source
entry identity when playing or bulk-removing filtered results, including duplicate
entries. Editing restores original order before drag handles become active.

Recurring visual corrections: a repaint must not remove explicit white header
ink and reveal inherited dark text; placeholder art must not upscale a small glyph
to fill the cover; a fixed heading must not cover the active filter field. Give
the field clearance above the keyboard. Short/empty pages need an explicit pull
gesture because overscroll events alone are insufficient. Reference album layout
must take precedence over an old legacy album-head preference. Reference keyboard
accept keys/carets use the same green accent and dark accept-key ink.

Verification: native collection-find journey passes 55 checks; collection editing
and persistence 72; local Search/discography 14; preview/Now Playing/Queue 14;
Home/Audiobooks 18 (173 checks total). Collection database and playback queue
model suites pass. Host and R1 cross-builds and glibc <= 2.22 ABI check pass.
Inspected 480 x 800 SDL captures cover Home, Audiobooks, grouped Search, Library,
discography, album art/missing art, playlist editing, filtered results with keyboard,
empty Liked Songs, fast-scroll end/return, Now Playing and Queue. Evidence lives in
`artifacts/collection-find-20261102/`; candidates, not approved visual baselines.
One test initially mistook the mini-player title for a filter result; assertions
now constrain matches to the result viewport. Added sorting uses local file import
dates, not an invented per-playlist membership timestamp. Legacy settings/game/
wireless screens and physical R1 RAM/touch/DAC were not part of this visual audit.
The interactive simulator was restarted with the updated native binary.

## 2026-11-02: Audiobooks as a Spotify-style Home feed

The latest user correction supersedes the native-screen shortcut below. Audiobooks
now selects an in-place Home feed: Continue listening, Recently added, and All
audiobooks. Reuse existing cover cards, the eight-slot artwork cache, details
bottom sheet, persistent mini-player, and existing audiobook playback/resume APIs.
The all-books grid is limited to 12 books per page; shelves to six each.

Book index callbacks hold a database mutex. Collect identifiers into a bounded
window first, then query metadata and create widgets outside the callback. Scan
only the card's Audiobooks directory, never the entire music card. Scanning keeps
saved positions; selecting a book opens details, explicit Play/Resume starts audio
without automatically expanding Now Playing. The category survives restart.

Verification: host and R1 builds and glibc ABI check pass. Populated Home journey
passes 18 assertions, and the empty journey passes five. Inspected native 480 x 800
renders include the feed, details sheet, scrolled shelves/grid, restored category,
and empty state. Evidence: `artifacts/audiobooks-feed-20261102/`. These captures are
candidates, not approved baselines. The fixture uses three original synthetic
recordings with metadata and embedded/missing artwork. Hardware RAM, touch and DAC
behavior remain unverified. Pagination beyond 12 books is implemented but has not
yet received a rendered interaction test.

## 2026-11-02: Home Audiobooks entry

Add Audiobooks immediately beside Music using the existing Home category chip.
It opens `audiobooks_screen` through normal navigation, rather than storing a third
music-feed filter. Back therefore preserves All/Music and the current playback.
The native audiobook section retains its Library, Authors, Series and Continue
Listening flows, with Scan when no books are indexed.

Host and R1 builds plus ABI check pass. `--journey home-audiobooks` passes eight
checks, including tab geometry, native destination, Back and playback continuity.
Inspected 480 x 800 SDL screenshots and live-state reports are in
`artifacts/home-audiobooks-20261102/`. Hardware behavior remains unverified.

## 2026-11-02: local Spotify-inspired UI branch

Latest user direction supersedes every older online/SpotBye entry below.
Branch: `feature/spotify-ui-local`. The previous dirty source, tools, tests and
UI documents were preserved before deletion in
`artifacts/branch-backups/before-ui-local-20261102-151210.zip`.

Delete provider modules and their lifecycle callers, not just their menu entries.
The Linux source mirror must remove deleted source and stale objects too, because
native test linkers enumerate build objects. It must never delete music or tools
outside the explicitly managed source/build paths.

Search uses local category chips and embedded thumbnails. Selecting a track keeps
mini-player preview. Search tests must distinguish result rows from the current
artist label in the mini-player. Model fixtures use real embedded metadata rather
than deleted downloader-specific `.source.json` attribution.

Evidence for this branch is recorded in `custom-firmware/local-ui.md`.
Older entries below describe the archived online branch, not current behavior.

Rendered search exposed combined collaboration credits as artist destinations.
Index individual RELEASE_CREDIT entries (bounded to 16), migrate existing search
rows via the dirty queue, and keep artist-result subtitles simply "Artist".
The native regression checks both collaborator lookup and persisted-index migration.

After replacing the fixture metadata, the UI harness still had double-encoded
accent/bullet expectations. Use explicit Unicode escapes in assertions; inspect
live labels before changing a correctly rendered component. Target a search row
by its content bounds so a duplicate mini-player title cannot absorb the tap.

## Historical decisions (superseded where they concern online features)


## 2026-11-02: nested download destinations

Folder templates affect transfers, restored jobs, cancellation cleanup, collection
receipts and M3U8 exports together. A filename-only settings change would leave
resume/export broken. Validate each path component, open parent directories with
O_NOFOLLOW, and perform final moves using the pinned parent descriptor and leaf
name. Keep no-replace commits. Reject oversized relative destinations rather than
truncating a source identity. Legacy single-level destinations remain valid.

The model compiler caught an omitted default initializer after the plan gained a
folder field; explicitly initialize it in restored collection plans. Full native
model checks remain required even when the host UI build succeeds.


## 2026-11-02: release metadata in filename templates

The adapter was discarding dates and numbering available in catalog responses.
Join raw collection entries to filtered recordings by ID, never array position,
and retain explicit album ownership/counts. Playlist length and position must not
become album totals or recording track numbers. Unknown values remain empty.
Persist these fields in both collection-page SQLite rows and individual manifests,
including ISRC previously omitted from individual manifests. Old caches migrate
with empty/zero metadata; admitted destinations remain unchanged.

The longer token row exposed an automated gesture overshoot: large inertial swipes
passed the desired token. Inspecting the failure render showed the controls were
present. Use smaller bidirectional swipes based on actual label bounds rather than
changing the layout to satisfy an incorrect test assumption.

See [release metadata evidence](custom-firmware/spotbye-template-metadata.md).

## 2026-11-02: captured filename templates

Expose only tokens supported by existing native metadata. Keep the source identity
suffix and source extension, sanitize the expanded prefix, and reject unsupported
tokens/path syntax before saving. Richer release/context tokens, folder templates
and separate album templates remain explicit parity gaps.

Individual jobs already persist their final destination. Collection jobs must also
persist the actual template so later settings changes cannot rename not-yet-admitted
members. Retain ISRC through the native parser and cached page metadata to avoid
different expansions after restart. The added persistence assertion caught the
parser discarding this optional field; accepting only validated codes fixed it. Do not mutate source metadata while formatting filenames.

The first native render exposed keyboard overlap with Clear/Use template actions.
Increase available sheet height; compilation did not reveal the collision. Verify
preview, token insertion, Save/Discard and keyboard cleanup through actual SDL taps.
See [filename-template evidence](custom-firmware/spotbye-filename-templates.md).


## 2026-11-02: online artist release categories

Use provider release types and dates, not track counts or artist-name guesses.
The pinned source exposes discography pages but not verified featured credits.
Keep compilations/unknowns under Other and state the Featured On limitation. Do
not advertise a loaded-page filter as a complete remote-catalog query.

Use a stable index permutation so grouping cannot detach artwork or callbacks
from release IDs. Preserve filter state across pages. Returning from an online
album must restore the underlying artist request identity; otherwise the visible
artist list's Next button can issue an album-track request. Direct-link artist
headings use the response title once available.

The native harness must use the actual Previous label; Previous page was a test
assumption, not a missing control. See [discography evidence](custom-firmware/spotbye-online-discography.md).


## 2026-11-02: collection failure continuation and recovery

The pinned related source continues ordinary per-recording failures but stops for
cancellation or recognized cooldown messages. Capture Continue/Pause in each job;
new preferences default to Continue while existing saved flags retain their policy.
Do not apply a later settings edit to an already submitted batch.

Persist failure identity, source position, error and attempt before advancing or
recycling a transfer slot. Transfer manifests must retain batch-attempt identity:
a failed slot from an earlier attempt can be recycled safely after a retry/restart,
but a newly failed transfer needs its own durable failure receipt first.

Retry traverses persisted pages, retains successful source-position receipts and
adds recovered recordings at their original positions. Never rewrite a successful
audio file. Keep earlier reports/playlists and use attempt-suffixed export names
from attempt two onward. A partial batch is Completed with failures, not a wholly
Downloaded batch. Persisted error details remain available beyond recent slots. Size the details
sheet to short content rather than leaving a full-screen empty panel for one error.
See [failure-recovery evidence](custom-firmware/spotbye-failure-recovery.md).


## 2026-11-02: selective collection downloads

Persist draft IDs in SQLite and copy them transactionally into submitted jobs.
Do not use row indices or a mutable UI selection as the transfer plan. Different
active plans for one collection must return an explicit conflict. A completed
subset must not silently stand in for a later full-collection request.

Selection shares the native album/playlist page and does not navigate playback.
Select page is deliberately labeled by its bounded scope. Repeated recording IDs
share one selection. Source-order exports and missing-recording failures are
verified; arbitrary desktop selection order and range selection remain gaps.

Render inspection caught uncentered button text and oversized checks; shared
collection buttons now center their labels and checks scale to 28 px. The UI
harness's visibility boundary must match the scroller, including its final button.
See [selective download evidence](custom-firmware/spotbye-selected-downloads.md).


## 2026-11-02: search saved download-collection members

Download search now reads persisted page metadata after recent transfer slots are
recycled. Keep this work off the LVGL thread, publish only the latest query, and
retain the existing eight-collection bound. A pending/error read must not be
represented as a definitive no-match result.

SQLite progress callbacks are deliberately omitted in this firmware. Use bounded
row batches with C budget checks rather than changing that shared build setting.
Release the read statement and collection gate between batches: a long read
transaction on the rollback journal can interfere with transfer receipt commits.
The query never changes queue state or fetches remote pages.

The managed UI harness must wait for the export confirmation popup to dismiss
before opening another panel; a click swallowed by that popup is not a search bug.
See [saved-member search evidence](custom-firmware/spotbye-saved-download-search.md).

## 2026-11-02: restore artist images

The missing online portraits were caused by an explicit adapter assignment that
cleared artist artwork to satisfy an older design rule. The latest user request
supersedes that rule: preserve provider thumbnails for artist identification.
Keep the discography page minimal. A live metadata comparison confirmed that the
upstream backend already supplied supported CDN URLs; neither URL validation nor
the image decoder needed loosening.

Your Library separately excluded artist rows from its artwork loader. Those rows
now use indexed local release art, with at most eight candidate recordings and
one asynchronous decode at a time. Missing art retains an artist placeholder;
scroll eviction restores that same placeholder. Existing eight-slot bounds remain.

The native tests now inspect decoded source dimensions rather than merely seeing
an image widget. A widget can exist while still showing a placeholder. Live
captures wait for multiple portraits, not only the first completed download.
Detailed evidence: [artist artwork](custom-firmware/spotbye-artist-artwork.md).

## 2026-11-01: result and download-manager filters

Added a shared native text-filter sheet with immediate matching counts, Clear,
Done and outside dismissal. Search retains source identity while combining its
filter with stable sorting. Downloads combines status, supported media type and
text filters without modifying scheduling. Exact manager frontend inspection
established richer member searching and added-time ordering; these remain explicit
gaps rather than being described as complete parity.

Repeated temporary keyboards require lifecycle ownership: unregister the keyboard
from global refresh lists, remove its screen callback and timer, and unregister
deleted caret fields. Defer keyboard storage release until child deletion finishes.
The native probe checks stable live keyboard count and exercises layout/theme
refresh after repeated sheet dismissal. Programmatic Clear must explicitly update
matching rows: the initial empty-text assignment did not refresh the list.

Automation must choose the topmost matching label when a modal is open. Downloads
has a background Done tab and a foreground Done button; clicking the first label
was a harness error, not a failure to close the native sheet.

Reports: [result filtering](custom-firmware/spotbye-search-filtering.md),
[download filtering](custom-firmware/spotbye-download-filtering.md), and
[current parity inventory](custom-firmware/spotbye-status.md).

## 2026-11-01: native online search sorting

Reimplemented the related SpotiFLAC SearchBar's category-specific sort choices
in the existing native search list. Sort uses a stable permutation of at most
24 indices, preserving provider metadata, artwork ownership and action identity.
Default restores the provider order. Each category persists its own mode. Local
relevance, album track order and discography chronology are unaffected.

The bottom sheet explicitly says sorting applies to the loaded page. Its height
follows the number of choices; inspection of the first artist render showed an
unnecessarily tall three-choice sheet, corrected without changing other screens.
Artwork cache slots must follow displayed positions. Source indices 0 and 12
can become adjacent after sorting; using source-index modulo 12 would let their
requests cancel or consume each other's artwork. Keep the item identity as the
row/callback key but assign cache slots by displayed position. The generated
color-artwork journey covers this collision. Its first fixture used a disallowed
URL host, so the real parser correctly stripped artwork; use accepted source URL
shapes and preseed the local cache, without loosening production URL validation.

The live harness now selects result-card labels by their 90 px parent geometry;
selecting the first label after the heading would accidentally open Sort.

Evidence and exact parity boundaries are recorded in
[the result-sorting report](custom-firmware/spotbye-search-sorting.md).
Candidates are not approved baselines; host/MIPS builds do not validate hardware.

## 2026-11-01: embedded download lyrics

New SpotiFLAC track/collection plans capture LRCLIB lyrics mode. The helper uses
the pinned original metadata lookup and LRC conversion, then merges only LYRICS
with upstream's TagLib dependency. Do not use upstream TagFile with a lyrics-only
Metadata value: that operation clears unrelated tags. Do not route this through
the desktop full-file FLAC rewrite or introduce an FFmpeg runtime dependency.
Native FLAC decode/checksum/finalization and incremental index remain downstream.

Off skips lookup; Synced only rejects plain results; Synced or plain preserves
plain presentation. Invalid, empty, oversized or incorrectly timed words are
skipped without failing audio. No broad title-search fallback. The source API
response and lyric text have explicit size bounds. Native default is Off;
reference per-provider defaults and extra lyric sources are not replicated yet.

Fixture correction: the native helper receives its output directory through fd 3.
The test-only FFmpeg child must inherit that descriptor to open /proc/self/fd/3;
otherwise fixture generation fails before production FLAC validation. The helper
itself uses TagLib and does not launch FFmpeg for lyrics.

Synchronized and plain download/import/full/search-after-restart verification passed;
plain lyrics correctly keep the inline card hidden,
as did settings persistence, model tests, original-backend generated-FLAC tag/audio
preservation, host/MIPS compilation and ABI checks. Final evidence and explicit
remaining gaps are in [the lyrics report](custom-firmware/spotbye-download-lyrics.md).
Candidate renders are not approved visual baselines or device measurements.

## 2026-10-31: automatic exports, metadata selection and search transaction recovery

Files now exposes automatic M3U8 creation, download reports and failures-only
reporting. New collection jobs capture these choices; migrated old jobs do not
suddenly generate files. Exports run on the worker with bounded buffers, no-replace
commit and byte-identical recovery after the file/DB crash window. Failed exports
retain the audio and offer explicit retry. Report attempts use separate names.

Metadata tags is a scrollable Files subview with 14 choices, the existing saved
settings draft and fixed actions above the mini-player. New track and collection
plans persist the exclusion mask. The pinned original backend applies it before
Sonix's FLAC validation and import; it never modifies existing library audio.

The longer 41-track journey exposed a second unchecked savepoint: search and
lyric-index refresh could leave a transaction open after failed RELEASE under an
external reader. A deterministic test seeded dirty search/lyric work, reproduced
the lock leak, and now passes. Shared ownership-aware savepoint cleanup covers
queue, search and lyrics. Dirty work rolls back and can be retried.

Native model/import/offline regressions and 480 x 800 managed/settings journeys
passed. Inspected Files, metadata and completed collection renders: controls fit,
scrollable content stays above fixed actions and the mini-player remains present.
Evidence and remaining parity differences are tracked in
[the finalization report](custom-firmware/spotbye-finalization.md).
These are candidate renders, not approved baselines or hardware measurements.

## 2026-10-31: persistent album and playlist download jobs

The exact SpotBye download-manager asset uses collection jobs with ordered track
plans and finalization options. Implemented a native SQLite collection controller
above the existing single-transfer worker. Album/playlist Download now advances
through bounded catalog pages; 24 results and 32 transfer slots no longer limit a
collection to its first page. Source/quality/naming choices are captured once.
Fetched pages, cursor and ordered completion receipts survive restart.

Keep transfer history distinct from collection totals. The initial render said
"32 total" above a completed 41-track collection; the summary now explicitly
labels collections and recent tracks. Completed slots require BOTH incremental
import acknowledgement and a committed collection receipt before reuse. A child
pause/cancel controls its collection so the controller cannot immediately undo
that action. Lock order is queue -> collection; no queue call holds the collection
mutex. Dismissal hides a finished/cancelled job without deleting audio or exports.

The managed journey uses generated audio, crosses the page/slot limits, restarts
paused, imports 41 files and exports their exact ordered relative M3U8 references.
The model also injects failure at track 21, retries, pauses through a child, and
restarts after track 25 and repeats the first recording beyond the reusable
window without duplicating its file. Evidence, final build checks and inspected 480 x 800
candidate captures are linked from [the collection report](custom-firmware/spotbye-managed-collections.md).
These checks do not establish live provider authentication or device performance.

Harness correction: after restart, clear the retained Search query and scroll
until the Downloads tile is inside the viewport. A present label below a clipped
scroll area is not a valid click target. The failed navigation attempts required
no production layout change. Database polling also explicitly closes each SQLite
connection and retries temporary locks within the original deadline: the Python
connection context manager commits/rolls back but does not close the connection.
Captures remain candidates, not approved baselines.

Repeated runs then exposed a production transaction leak, not just harness
contention: playback queue persistence ignored failed COMMIT/RELEASE operations.
A failed outer commit left a write transaction open, and subsequent incremental
imports reported "cannot start a transaction within a transaction". A deterministic
second-reader test reproduced the failure before the fix. Both queue persistence
forms now check statements and finish a savepoint with ownership-aware rollback;
the caller only marks a queue revision saved after success, allowing retry.
`tools/test_library_import.py` and the priority model cover this correction.

## 2026-10-31: native provider transfer path

Connected the reviewed SpotiFLAC Tidal/Qobuz/Amazon backend to the existing native
download worker. Quality settings are captured per job; unknown-length transfers
do not show fabricated percentages. Source failures use isolated attempt folders;
completion requires full FLAC decoding, duration/size checks, persisted checksum
and no-replace commit before incremental import. Source identity, not the newly
computed checksum, identifies repeat catalog actions.

Native generated-audio queue tests and the 480×800 download/import/play journey
passed. Settings persistence and uninterrupted local playback passed. Inspected
the actual Quality, detail sheet and indexed download renders. Evidence and
remaining live-provider/hardware gaps are in
[the transfer report](custom-firmware/spotbye-native-transfers.md).

Two harness corrections: the provider fixture command includes an 18-byte prefix
including its space; a shorter offset silently fails to open the fixture. The
temporary helper must also be moved after queue tests so the subsequent
missing-helper test still tests a genuinely missing helper. Neither failure
required changing production layout. Captures remain candidates, not approved
baselines. The end-to-end fixture runs a private copy of the native binary and
does not replace the installed helper or contact a commercial audio source.

## 2026-10-31: exact SpotBye frontend and native online restructuring

Recovered 45 embedded frontend assets from the user-opened SpotBye 2.0.0 PE/Go
embed.FS table. REA statically analyzed the settings and Go bindings; the recovered
119 methods and 159 literal defaults are evidence of this exact build's frontend,
not proof that the related SpotiFLAC Go fork is identical. No user credentials or
saved provider accounts were read. Full contract and limitations:
[exact-build audit](custom-firmware/spotbye-exact-audit.md).

Moved online settings into their own component and introduced visible native
Tracks / Albums / Artists / Playlists controls. Source and result category are
independent; fresh configurations use SpotiFLAC, existing choices persist.
Artist discography and playlists use bounded original-backend requests. Playlist
headers retain collection identity while individual songs retain source album IDs.
Direct track links now resolve the exact recording and provide Go to album.
Result pagination retains its own browse context when returning from a release.

Visual inspection found playlist covers were downloaded successfully but remained
placeholders: the CDN returned WebP, unsupported by Sonix's decoder. Added the
pinned static decoder to both host and target, with source/output dimension and
byte limits and animation rejection. The shared artwork cache now renders those
covers without another helper process. Do not disguise decoding failures as
missing online artwork. Synthetic native tests check colors, scaled dimensions
and invalid/oversized/animated data with ASan/UBSan.

Test correction: the artist result and search query can have the same label.
A generic text click hit the textarea, so the journey now selects the result
below the provider heading. Pagination taps also require the full target inside
the scroll viewport; offscreen labels below the mini-player can otherwise send
a test tap to bottom navigation. Artist identity requires the upstream overview query;
the paged discography response omits uri/profile. Validate via the overview
instead of dropping identity checks. Preserve playback throughout both requests.

Verification: host and MIPS builds plus glibc <=2.22 ABI checks passed; native
model/TLS tests and live artist/playlist/track requests passed. Settings regression
passed 16 checks, local-first regression 14. Expanded browsing passed 25
checks, including direct track links and artist pagination after release browsing.
Renders and report
are in `custom-firmware/evidence/spotbye-categories-ui.json`; current captures are
candidates, not approved visual baselines. Playback priority and collection model
regressions also passed after updating their static decoder linkage. Host RSS is
about 39 MiB after restart, not proof of simultaneous R1 playback/network memory.

Provider download plans, full settings behavior, conversion/finalization, online
discography filters, and actual device validation are still unfinished. The exact
frontend exposes a richer managed download contract than the existing native
Archive queue. This is a verified browsing overhaul, not completed 1:1 parity.

## 2026-10-28: original SpotiFLAC backend integration

The user explicitly rejected substitute providers as satisfying 1:1 parity.
Compile the supplied pinned Go backend as a headless worker and call it from
the real C/LVGL UI. Do not claim its successful catalog connection establishes
full download/settings parity. Source choice, native rows, album page and
modal/playback ownership reuse existing components.

The original backend compiles/links for MIPS soft-float and starts under QEMU;
live search and synthetic tagging also work under emulation. This disproves a
blanket assumption that Go code cannot run on the R1, but does not establish
the complete workload's memory or hardware suitability. Catalog requests have
response/page/deadline bounds; tagging remains an isolated feasibility probe.

The native UI journey passed 14 checks. Captures were inspected at 480 x 800.
An initial test used an unavailable album ID; the backend correctly rejected
the empty response. Use a live-verified recording/release identity, and report
unavailable metadata separately from a different returned identity. Do not
relax identity validation merely to make a test pass. Source digests must use
explicit case-sensitive POSIX path sorting across Windows and Linux.

Build instructions, candidate renders, evidence, dependency constraints and
the remaining parity matrix are in
[spotiflac-native-core.md](custom-firmware/spotiflac-native-core.md).

## 2026-10-28: integrated online release milestone

Submitted All searches previously waited for HTTPS before publishing local hits.
Separate bounded local/provider workers now publish independently. Query generations
invalidate stale results, including filter changes. A host-only delay exercises this
without mocking provider responses. Local search remains usable while online waits.

Extracted reference collection header widgets from medialist, reused on online
releases, and retained the shared footer and details sheet. Complete <=24-file
responses can be admitted atomically into 32 persistent slots; downloads remain
single-worker and separate from playback. The manager groups source files by release.
Paged releases explicitly retain per-file downloads, not an incomplete Download All.

Code-review correction: never compact queue slots while a transfer worker owns a
slot index. Batch admission reuses inactive slots in place. Regression coverage
checks capacity rejection, successful replacement and persisted 32-job restoration.
Test corrections: search-hit assertions exclude mini-player labels; use escaped
Unicode in shell-generated Python fixtures to preserve accented expected titles.

Rendered/interactive evidence: full collection regression 71 checks, new online
release/search journey 14 checks. Real HTTPS UI flow verifies automatic import and
local playback with discovery disabled. A separate live test denies connect() before
indexing and decoder reads; 6,419,897 verified bytes decode at 44.1 kHz stereo.
Provider/queue/TLS tests, host build, target build and glibc <=2.22 ABI gate pass.
Source review and remaining structured-discography/matching/artwork/hardware work
are documented in `custom-firmware/native-online.md`. Host RSS is evidence only,
not a claim that 64 MB R1 performance is established.

## 2026-10-28: artwork taps, playback preview and initial online flow

The decorative artwork panel on Library shortcuts intercepted touches. Make the
artwork and glyph non-clickable so hit testing reaches the owning collection row;
the same component serves list and grid. Names and portraits open identically.

Reference-layout collection track/Play/Shuffle actions now stay on the collection
page. All songs also mounts the shared mini-player/footer. Explicit mini-player
taps still expand Now Playing; legacy layouts keep their previous behavior.

The user selected native adaptation of SpotBye's workflow, rather than connecting
an existing SpotBye backend. This increment adds direct Archive release links and
a source-details bottom sheet before explicit downloading, reusing the existing
bounded provider worker, resumable queue and local importer. No SpotBye executable
or unverified private endpoint is used. Unsupported service links produce feedback.
The live provider contract is documented by the
[Archive metadata API](https://archive.org/developers/md-read.html).

Verified with actual simulator taps: Liked Songs artwork in list/grid, All songs
artwork, track playback staying collapsed, explicit mini-player expansion,
details-sheet playback continuity and no implicit download. The focused journey
passed 15 checks. Provider/queue/TLS regression tests passed. A fresh live
6,419,897-byte download passed checksum, index and offline-decode checks. Details
and inspected screenshots are in `custom-firmware/native-online.md`.
The broader collection journey passed 71 checks, including collapsed Play/Shuffle
and restart persistence. Host/target builds and the glibc <= 2.22 ABI gate passed.

## 2026-10-28: shared collection page and repeatable verification

Audit: playlists used a second screen in `playlistview.c` with text Play/Shuffle,
paged tracks, and no album-style cover. Albums already had a windowed track list,
artwork header, transport actions and mounted shared footer in `medialist.c`.
Decision: reuse medialist for playlist browsing; retain playlist-specific song
selection and metadata management. Disc numbering remains album-specific.

Recurring corrections are requirements, not optional restyling:

- Now Playing and panels enter upward, never right-to-left.
- Save is one plus/check by the song title; Lyrics and Queue sit opposite below
  transport. No duplicate bottom save bar. Hide unavailable inline lyrics.
- Credit rows represent individual selectable artists with bounded labels.
- Artist pages are discography only. Library is local metadata organization,
  independent of Sonix filesystem folders.
- Playlist and album browsing share their visual component.
- Inspect real rendered output and behavior before calling a UI task complete.

Existing evidence and model checks live in `docs/custom-firmware/`. Older pictures
may show superseded controls. This specification takes precedence. No approved
pixel baseline or connected-device verification was supplied for this task.

The host probe is opt-in via an environment variable and is compiled out of the
target. Its setup routes are fixture conveniences; interaction assertions use
actual simulator input and independently observed playback/model state.

Verification: the full native collection journey passed 67 checks, covering
album/playlist geometry, empty/long/257-track lists, missing artwork, scrolling,
handle and scrim dismissal, repeated/rapid taps, real playback continuity,
database-confirmed reordering/membership, priority preservation under shuffle,
Library filtering, Liked Songs and restart persistence. Captures were visually
inspected. A subsequent targeted empty-Liked-Songs run verified its disabled
empty action and first-open background correction. A pixel assertion now guards
the empty-page background: painting only the header had left the legacy theme
visible on a freshly constructed page.
Color checks allow RGB565 quantization instead of requiring exact 24-bit RGB.

Native priority queue, collection library, offline library/lyrics, discography,
and Home model suites passed. Host and target builds and glibc <= 2.22 ABI check
passed. The host observation was 39,680 KiB RSS and approximately 6% of one CPU
core over one second (SDL software renderer); this is not an R1 measurement.

Test lesson: Sonix success notices intercept input until dismissed. The journey
explicitly dismisses the queue-added notice before testing Shuffle, rather than
mistaking the intercepted tap for a playback-engine failure.

Remaining verification: hardware timing/RAM/audio/standby, approved pixel
baselines, and exhaustive UI automation of every unrelated screen. Existing
model suites cover queue/repeat/discography/search invariants; this change's UI
journey does not claim to exercise every control in those screens.


## 2026-10-28: inspect the running SpotBye, then implement native settings

REA inspected the exact user-opened 2.0.0 executable; Windows pointer/screenshot
inspection established settings and download-manager controls. CDP runtime tracing
was unavailable; repository URLs returned 404. Preserve this distinction: observed
UI is not a verified provider protocol.

Added saved native source/format, start-paused, retry, rate and filename options,
with draft Save/Discard/Defaults, plus queue filters and batch start/pause. Filename
preferences are captured per job so later edits and restarts preserve destinations.
The settings journey passed 16 checks with real 480 x 800 SDL interaction and
inspection; live regression, provider tests and offline decode also passed.
Detailed parity gaps, screenshots and reports are in
[spotbye-parity.md](custom-firmware/spotbye-parity.md). This remains partial:
SpotBye catalog/providers, tagging/template parity and hardware validation are open.


## 2026-10-28: supplied source unlocks a native catalog adapter

The accessible 1111Windows/SpotiFLAC fork is pinned at
90f51a4da5bff0ec51183dd5e789a78e6f6a009d. Its metadata/link/download separation
informed the new independently implemented native Deezer metadata adapter. The
SpotBye gist is a binary link, not source; it does not tie that fork to the
inspected executable. See `custom-firmware/spotiflac-source-audit.md`.

Preserve stable service IDs/ISRC rather than treating a title match as recording
identity. Catalog availability and full-audio download capability remain separate.
Use the existing image worker and shared collection header. A rendered long URL
revealed that textarea padding did not clip text beneath the search icon; the
input now has an actual narrower viewport inside the same capsule. Missing artist
metadata no longer produces a stray separator.

Verification: 19 live native catalog checks and 14 local-first/Archive regression
checks passed. Sanitized model/TLS/live-catalog tests and host/MIPS/ABI gates also
passed. Captures and source evidence are linked from the source audit. Test lesson:
a matching title can be stale while a new URL request is pending; wait for the
one-result response before asserting direct-link behavior. The opt-in host probe
now identifies textareas so geometry checks refer to the native input viewport.


Filename parity follow-up: execute only the recovered pure formatter to establish
numeric padding/disc-track examples. Record its asset digest and inputs/outputs;
this does not validate the executable's native download backend. Keep the native
bounded prefix/identity suffix difference explicit. Separate filename policies
must be resolved when a collection is submitted, not while its later pages arrive.

The expanded template journey outlasted the playing fixture song. Its exact-file
assertion failed on natural progression to the next track with the same queue.
For long browsing journeys verify active playback, original context membership,
unchanged priority/context queues and no download-induced playback instead of
requiring the first song to play forever.

Nested M3U8 paths beginning with # must use a ./ prefix; otherwise the playlist
parser interprets a legitimate folder or recording as a comment. A persisted
receipt/export regression covers that case without altering audio filenames.

## 2026-11-02: naming context remains separate from recording credits

Playlist names and creators are download context, not replacements for performers
or album artists. Persist them alongside each cached recording so paging/restart
does not lose naming inputs. Match raw catalog tracks by stable ID. Category uses
explicit release metadata only; a live playlist response omitted album type even
though the album page supplied it. Preserve that uncertainty rather than inventing
a category. Recovered pure frontend examples establish context expansion and
explicit markers, not provider delivery or complete executable parity.

As Files settings grew, Metadata tags moved below the viewport. Native regression
tests must scroll to its actual bounds before tapping; a matching offscreen label
is not an interactable target. Config values are truncated by the core store
before template loading; malformed-input tests must remain invalid after that
truncation rather than assuming the loader sees the original oversized string.

Verification: folder/naming journey 34 checks and general settings regression 24
checks passed. Generated audio resumed and indexed under its original captured
nested path after settings changes/restart. Folder/context previews and restored
settings were inspected at 480 x 800. ASan/UBSan, host/MIPS and glibc ABI checks
passed. Evidence and exact-behavior differences: [folder/naming report](custom-firmware/spotbye-folder-templates.md).

## 2026-11-02: interrupted provider staging recovery

Directory names alone cannot establish ownership. Version and flush a marker
before starting a provider, and inherit the exact locked directory descriptor
into the helper. Cleanup runs incrementally on the existing download worker;
locked stages are retried, and unknown/legacy folders stay untouched. Recovery
must never convert temporary audio into an assumed completed recording.

Crash-test lesson: restore normalizes unfinished jobs to Paused in memory. The
manifest can still show its pre-crash Running state until the next persistence
event. Assert live download state and absence of automatic transfer activity,
not that startup immediately rewrites the manifest.

Recovery verification: 16 native crash/restart/resume checks, full discovery
ASan/UBSan suite, ownership/locking cases and host/MIPS/ABI gates passed.
See [recovery evidence](custom-firmware/spotbye-stage-recovery.md).

## 2026-11-02: exact naming normalization is not artist navigation

Apply recovered primary-artist splitting only inside custom filename expansion.
Preserve structured/full credits for library navigation and metadata. Compare pure
frontend expansion separately from filename sanitization; AC/DC remains one artist
but its slash cannot appear inside a filename. Native catalog parsing already
validates/canonicalizes ISRC, so UI transfer fixtures must supply valid identifiers;
raw/legacy whitespace normalization is covered by formatter models.

Reference-fixture bounds: native ISRC storage is 13 bytes including its terminator.
Do not silently truncate an oversized desktop example before comparing outputs.
The fixture now checks every input length before copying and uses representable
identifiers. Whitespace-only suffix fallback remains a separate formatter case.

Naming verification: 29 reference examples and the full discovery sanitizer suite
passed; nine native UI checks verified normalized naming through restart, transfer,
indexing and offline preview. Host/MIPS and ABI gates passed. Evidence:
[naming normalization](custom-firmware/spotbye-template-normalization.md).

## 2026-11-02: all-pages selection uses a persistent rule

Store an all-pages flag and excluded IDs, not every remote recording in memory.
Selection never fetches metadata or starts audio/downloads. New pages inherit the
rule; the UI reports excluded counts because the full total is not known. Snapshot
the rule and exclusions with the job settings, and retain source-order receipts.
Document that unfetched remote content remains live until fetched; this is not
proof of exact desktop snapshot behavior.

Migration fixture lesson: INSERT statements for a subset of job fields must name
the columns. Positional VALUES broke when selection_all was added; explicit column
lists exercise migration defaults and tolerate later independent schema fields.

All-pages verification: 21 native checks, 20 explicit-selection regression
checks, full discovery sanitizer suite and host/MIPS/ABI gates passed. Selection
and paused-job screens were inspected at 480 x 800. Evidence:
[all-page selection](custom-firmware/spotbye-all-pages-selection.md).

## 2026-11-02: download admission order

The reference orders by addedAt, not recording ID or collection type. Capture
admission once and persist it in both stores. Sort small display references only;
keep queue storage, scheduling and callback identity unchanged. Legacy records
must not acquire an invented current date. Equal/unknown values stay stable.

Native tests must scroll the Search dashboard to its Downloads shortcut after
restart; presence in the object tree is not a usable tap target. A screenshot can
also capture the previous SDL frame after the probe sees the new route, so allow
that frame to settle and inspect pixels before retaining evidence.

Ordering journey: 16 checks; existing download filtering: 20 checks. Discovery
ASan/UBSan, host/MIPS and glibc ABI gates passed. See
[ordering evidence](custom-firmware/spotbye-download-ordering.md). Full desktop
job grouping and physical-device behavior remain unverified.

## 2026-11-02: persistence is a download precondition

Do not report a download as successful merely because its transfer completed.
Queue/admission, provider-integrity and audio/directory flush barriers now propagate
failure. Keep committed audio for local verification on retry. Storage failures
stop Continue batches at the current member instead of recording every remaining
track as unavailable. Reuse the existing download error card and explicit retry.

Native storage journey: 15 checks passed, with the save-failure and recovered-file
captures inspected at 480 x 800. Scope and model evidence are recorded in
[download durability](custom-firmware/spotbye-download-durability.md).

Final durability verification: full discovery ASan/UBSan, host/MIPS and glibc ABI
checks passed; the storage, crash-recovery and normal-transfer UI journeys passed
38 checks combined. Concurrent destination creation still preserves the other
writer's bytes and reports a finalization conflict. Hardware/live-provider testing
is not implied by these generated-audio and fault-injection results.

## 2026-11-02: whole-product route audit and shared reference system

A read-only inventory traced 237 reachable routes (plus 12 critic additions) and
97 Spotify Mobile research records: [inventory](records/route-inventory-2026-11-02.md),
[research](records/spotify-research-2026-11-02.md). The root cause of most legacy
escapes was shared: the Home ⋮ opened the Sonix tools grid, the gear opened the
Adwaita settings tree, and every utility page inherited the Adwaita palette
(#242424, user accent blue), `settingsrow` cards and gridpage picture tiles,
without the mini-player or navigation.

Decisions:

- Collection UI never deletes audio in the reference layout. The selection bin
  and "Allow deleting tracks" are not offered and the delete handler refuses.
  Explicit file deletion remains in the File manager, which presents files.
- Home shelf Play keeps Home visible (preview mode), matching collection Play;
  only the mini-player expands Now Playing.
- Reference layout uses a fixed palette (#121212 / #282828 / #b3b3b3 / #1ed760,
  black ink on green). Spotify Mobile has no light theme or accent choice, so
  Appearance hides Theme, Accent colour and Dynamic tint there; the preferences
  stay stored for legacy layouts. Clock position, text size and other device
  options remain.
- `settingsrow.c` and `gridpage.c` render flat Spotify-style rows, header,
  switches, sliders and chips in the reference layout and mount the shared
  footer. Scan screens are excluded because their own buttons are the exit.
- One device-menu button in the profile position (left of each tab title) opens
  the shared sheet: Settings, Devices, Audiobooks, Browse music, More. The tools
  grid and separate gear are no longer reachable from the shell.

Test lessons: `lv_obj_get_y()` of an aligned container is 0 until its first
layout, which made the first-shown pages extend under the footer; update the
layout before measuring. Probe nodes now report switch/slider type, checked and
scrollable so journeys assert real toggle state; exclude the screen root and the
list container when selecting rows.

Verification (host SDL 480 × 800, candidate evidence only): new journeys
`home-play` (7 checks) and `device-settings` (27 checks, during playback, no
legacy accent pixels); regression `collections` 72, `preview-local` 14,
`local-search` 14, `home-audiobooks` 18, `home-audiobooks-empty` 5,
`collection-find` 55; native suites scope, collection library, Home feed and
offline library pass. Captures inspected: Home after Play, device menu, Settings
root, Screen, Devices hub, Appearance, Buttons, keyboard, audiobook details.

Remaining gaps (see inventory): page-specific cards (Clock position, Control
centre list, legacy audiobook Library/Authors/Continue lists) still use raised
cards; 22 px notes are oversized; confirm/toast/popover are not yet the shared
dialog/snackbar/sheet; first-run panels, time picker, quick panel and power menu
keep legacy layouts (now on the reference palette); the Music settings layout
chooser can leave the reference experience; shell strings are not localized;
Control centre swipe-back needs verification; no device measurement.

## 2026-11-02: notices, dialogs and the queue swipe

Research (community reports, medium confidence) shows Spotify Mobile feedback as
a brief, non-blocking message at the bottom above the navigation that refreshes
on repeat; dialog visuals have no dated evidence. In the reference layout,
`toast_success/error/plain/glyph` now render as a snackbar: 448 px wide above the
mini-player and navigation, #3e3e3e (provisional colour), 26 px status glyph,
18 px text, transparent non-clickable veil so taps reach the page, replaced by
the next message. `toast_busy*` keeps the centred blocking card because the user
is waiting on that work. `confirm_show/notice` use a 16 px radius #282828 card,
22 px bold title, 18 px secondary body, 52 px pills: text-only Cancel and green
action with black ink. Taps outside still do not answer the question.

Defect found while testing: a deliberate but slow rightward swipe on a track row
outlasted LVGL's long-press delay and entered selection mode instead of adding to
the queue. A long press is now ignored once the row's horizontal swipe has been
claimed. The legacy multi-select toolbar over the collection cover remains a
recorded presentation gap.

Verification: `device-settings` now 38 checks, adding swipe-to-queue mutation,
snackbar position above the footer, non-blocking navigation while it shows,
reference dialog without legacy accent pixels and Cancel without scanning.
Captures `snackbar-queue.png` and `dialog-scan-confirm.png` inspected.

## 2026-11-03: option menus, audiobook lists and notes

- `popover_show()` (about 22 consumers: sorts, keyboard layout, Bluetooth and
  Wi-Fi row actions, EQ/MSEB/PEQ presets, remap, metronome and others) renders in
  the shared bottom sheet in the reference layout: 60 px rows, 20 px labels, the
  current option in green with a check, close before the action runs. If a sheet
  is already open the anchored card is kept (on the reference palette), because
  opening a sheet would replace the content the menu came from. `popover_close()`
  also closes the sheet form. Initial sheet height clipped the last option;
  height now includes the space above the body.
- Legacy audiobook Library/Authors/Continue/Series lists keep their pooled,
  windowed rows but use flat reference rows (76 px with 56 px art; 64 px names)
  and mount the shared footer via the new `settingsrow_footer()` helper.
- Explanatory notes use `settingsrow_note_font()`: 16 px in the reference
  layout instead of 22 px (43 call sites).
- Hub titles follow the device menu names: Devices, Browse music. The Screen
  brightness and Appearance clock-position cards are flat rows.

Confirmed gaps found during this pass: swipe-back does not leave Settings →
Control centre (its reorder list keeps the drag); Browse music index pages
(Albums, Artists, Album artists, Genres, Years from `medialist`) still use raised
card rows and have no footer.

Verification: `device-settings` 51 checks (adds option-menu sheet, green current
check, bottom clearance, close on choice, audiobook list rows/footer) and
`collection-find` 55 checks after the change; captures inspected:
`option-menu-sheet.png`, `audiobook-library-list.png`, Appearance, Buttons,
Browse music hub.

## 2026-11-03: core player behavior and Now Playing fidelity

User direction: focus on the main music player UI and features.

Research (read): allthings.how queue guide (updated 2021-12-08) and SoundGuys
(2024-12-17): mobile "Add to queue" appends to a separate "Next in queue" list in
the order added, clearable separately; the context continues afterward. Spotify
community reports treat repeat resetting on a new context as a defect. Prior
record NP-REF (newsroom/support, documented): down chevron, header naming the
playing context, plus/check beside title and artist, green active modes.

Behavior fixes:
- Collection Shuffle + Play uses `playlist_set_shuffle()` and keeps the chosen
  repeat; it previously reset repeat (and "continuous shuffle" forced repeat-all).
- In the reference layout `playlist_enqueue()` always appends to the manual
  priority FIFO; the legacy "add at end of context" preference is hidden there.
  Native test extended (`tests/priority_queue.c`).
- Collection track menu "Add to playlist" opens the shared membership sheet
  (search, Saved in, New playlist, Done) instead of the legacy playlist page.
- Now Playing "Track information" closed nothing, so the details page loaded
  invisibly beneath the player. It now closes the player, shows Details with
  the footer, and Back reopens Now Playing.
- Queue sheet: title at the sheet text edge, corner buttons clear of the close
  button, Clear/Reshuffle as pills. Context section reads "Next from <name>":
  a new persisted `playlist_set_context_name()` (cleared on every new queue,
  restored with the boot queue) carries playlist/page names; albums keep the
  existing album label used by album chaining.

Now Playing presentation (reference layout): down chevron while the player is
open; header "PLAYING FROM ALBUM/PLAYING FROM" plus the context name; title
block no longer shifts the title alone (it collided with the artist), with
12 px before the progress bar; inactive shuffle/repeat #b3b3b3, active green
with a 5 px green dot.

Verification: `--journey player-core` 30 checks (repeat toggle, header, single
mode dot, no title/artist overlap, artist clear of the bar, queue header/close
geometry, details above the player and Back, Shuffle+Play keeps repeat, preview
mode, "Next from Night Drive" before and after restart, track-menu queue goes to
the manual FIFO, membership sheet over the collection, playback unchanged).
Captures inspected: `player-now-playing.png`, `player-queue-sheet.png`,
`player-details.png`, `player-add-to-playlist.png`.

Remaining core-player gaps: audiobook mode in Now Playing still uses the legacy
popover/chapters pages; "See all results" opens the legacy search page; multi-select bulk add uses the legacy
picker; the Player layout chooser can leave the reference experience; context
kind (playlist/genre/Liked Songs) is not stored, so the header says "PLAYING
FROM" without the kind; no device check.

Follow-up (same day):
- Defect: Shuffle + Play on the list already playing reused the existing deal
  (the same-context fast path in `playlist_load_index`), so the random start sat
  late in the order and most songs counted as already played (one upcoming of
  five in a capture). New `playlist_shuffle_restart()` re-deals with the playing
  track first after an explicit shuffled Play; row selection inside the playing
  list still keeps its deal (existing "stable deal selection" requirement).
  Native assertion added; `player-core` checks the fresh deal.
- Search song taps play in the song's album context starting at that song
  (offline stand-in for Spotify autoplay); a song without an indexed album plays
  alone. Previously a filesystem-folder queue. `local-search` asserts the album
  context and that the folder is not queued.
- Album header artist text opens the credited artist's discography (one credit)
  or the shared chooser sheet (several). `player-core` reaches "Featured On".
- Probe now reports `context_album` and `context_name`.

## 2026-11-03: context kind, search lists, book player and remaining escapes

User direction: continue until the player matches Spotify Mobile as an offline
player.

Research (read 2026-11-03): Spotify support "Purchase audiobooks" (mobile: a
"Playback speed 1x" control, skip 15 s, a "Sleep timer" control, dots menu;
iOS/Android differ only in the dots glyph); Engadget 2024-11-21 (audiobook sleep
timer, follow-along); 9to5mac/RouteNote 2021-05 (search filter chips under the
field narrow results to one kind); Spotify Community Android thread titles
("Playing from Playlist" top bar, medium confidence, no dated screenshot). The
"PLAYING FROM ARTIST/SEARCH/YOUR LIBRARY" wordings follow that pattern and are
marked medium/low confidence; GENRE, YEAR and FOLDER are R1-only kinds.

Decisions:
- Context kind is stored beside the context name (`playlist_set_context()`,
  config `player.context_kind`), cleared with every new queue, never kept
  without a name, restored at boot. Liked Songs, Most played and Home mixes are
  "PLAYLIST" (Spotify presents them as playlists).
- Shuffle + Play persists its fresh deal at once (`device_state_queue_changed()`);
  before, a restart restored the old deal with the current song last.
- The player-layout chooser, the Waveform note and cover accent are not offered
  in the reference layout; cover accent never recolours the reference player.
- Search: chips re-query one kind with the whole 30-row budget (index cap raised
  12 → 30, still behind the 512-candidate bound); a group that fills its slots
  ends in "See all songs/albums/artists", which opens the shared collection list
  over `LIBRARY_FILTER_SEARCH` (rows, menus, swipe-to-queue, preview playback).
  The result count line is gone (Spotify shows none, and a capped count was
  wrong). Browse music's search button opens the Search tab. The chips appear
  only with a query; the empty browse page starts its tiles under the field.
- `settingsrow_title` centred reference headings with a height read before
  layout (0), so every reference page title sat 16 px low; it now uses the
  font's line height.
- Reference song rows always show the artist, except on an artist's own lists.
- Selection-mode and Files "Add to playlist" use the shared sheet in a
  destination mode ("N songs", New playlist, Liked Songs, playlists); the set is
  added on the existing worker with the snackbar. Search's and Browse music's
  "Playlists" open Your Library on the Playlists chip.
- Book player (reference layout): Chapters, Bookmarks and Summary open as sheets
  over Now Playing instead of pages that closed the player; the playing chapter
  is green and a pick seeks and closes the sheet; bookmark rows carry a remove
  button (a sheet cannot stack the long-press menu). The speed control is a
  "1x" label with a titled "Playback speed" sheet; the right control is a moon
  sleep timer (5–60 min, End of chapter via the existing chapter-end stop, Turn
  off timer when set; moon green while set) using the book's own timer. The
  header names the chapter being heard. Option sheets gained an optional title
  and an eighth row; untitled sheets start under the handle.
  On a book the bottom shortcuts drop lyrics and the queue shortcut opens the
  chapter sheet.
  Adaptation: skip size stays the R1 audiobook setting (10 s default) rather
  than Spotify's fixed 15 s.

Verification: full regression 10 journeys / 336 checks and 7 native suites
pass; then `audiobook-player` 38 checks with the shortcut. New
`--journey audiobook-player` (fixture book0 now carries Vorbis chapter tags),
extended `player-core`, `local-search`, native `priority_queue`.

Remaining: Playlists/lyrics have no full search list; the selection toolbar's
own chrome; "All songs" lacks the shared collection header; no device check.

## 2026-11-03: finished-looking library flows and one row swipe

User direction: keep improving the core, with a clean, finished UI and flawless
swipe controls (example: the plain Create pop-up).

Research (read 2026-11-03): Online Tech Tips 2022-12-30 (mobile: + in Your
Library, bottom menu with Playlist/Blend, name prompt, Create); 9to5Google
2022-03-03 (Android swipe right on a song row queues, as on iOS); TechCrunch
2015-01-22 (iOS swipe right queue, swipe left save). Spotify's naming screen
wording ("Give your playlist a name", default "My playlist #N", green Create)
and the empty playlist invitation ("Let's start building your playlist",
"Add to this playlist") are from product familiarity, medium confidence; no
dated screenshot was read. Folders are an R1 adaptation (mobile Spotify does
not create folders).

Shared row swipe (`shell/rowswipe.c`): every swipe surface (song lists, search
results, the queue sheet, Your Library rows) uses one component. The row's
content follows the finger over a strip with the action icon; the strip is grey
until 96 px, then green (queue, pin) or red (remove); release past it commits,
otherwise the content springs back (170 ms). It claims a press at 8 px of
mostly sideways travel, before LVGL's 10 px scroll limit and the swipe back's
10 px, stops horizontal scroll chaining from the row, and the swipe back stands
down while it is active. The left 32 px stay the swipe back. Defects found on
the way: the old song-row swipe only drew a 3 px border and, on playlist pages,
lost the press to a scroll/back drag after 7 px; a long press fired on a finger
already moving and started multi-select; the queue's swipe-left removal had no
feedback; library pin swipes had none.

Sheets: collection sheets lost the "Done" button that sat on the sheet's close
control; menus are sized to their rows with icon rows and a green check on the
current choice (options, Sort by, Move to folder). Create is Spotify's sheet:
two described kinds in circles (Playlist, Folder). Naming is a centred question
with a large underlined field, a suggested default as its hint (empty Create
takes it), and Cancel/Create pills; rename says "Rename playlist" with Save.
Creating from the sheet opens the new playlist. Delete uses the shared
confirmation dialog, then a snackbar. Add to playlist has a white New playlist
pill, a rounded Find field and a green Done pill; rows are no longer tinted.
Add to this playlist (song picker) shows sleeve, title, artist and a circled
plus that becomes a green check; its keyboard waits for a tap.

Library rows: "Playlist · 6 songs" (middle dot), no per-row "..." (long press
and swipe), wider titles. Empty playlist: bold invitation with a white "Add to
this playlist" pill; empty Liked Songs: "Songs you like will appear here".
Liked Songs wears the violet-gradient heart cover; an empty playlist the grey
note; song rows without art sit on the grey placeholder square.

Verification: new `--journey library-create` (42 checks: Create sheet
geometry, naming pills, default name, new playlist opens, empty state and its
picker, options, rename, delete confirmation; held mid-swipe captures for
library pin, song queue and queue removal with strip colour, travel and
spring-back; short swipe and vertical drag commit nothing). `collections`
updated for the content-sized sheet handle and the new empty-state wording.

## 2026-11-04: Spotify cards and carousels on Home and Search

User direction: make the Home and Search title cards look and act like Spotify
Mobile's, with the ability to scroll through them.

Research (read 2026-11-04): Engadget 2020-03-09 (Home leads with favourites and
recently played, recommendations below); Spotify Community Android threads
(Browse folded into Search; your top genres at the top, Browse all below).
Exact card metrics (56 px shortcut tiles, 150 px shelf cards, tilted sleeves)
are from familiarity with the app, medium confidence; no dated screenshot read.

Home:
- Shortcut grid: up to eight 56 px tiles in two columns, sleeve flush left on
  #2a2a2a, name in up to two lines (one line centred), no "..." badge; a long
  press opens the menu. The playing collection's title is green.
- Shelves are carousels from the gutter to the screen edge: 150 px cards (round
  sleeve for artists), name and "Playlist · creator" below, no overlay buttons,
  the next card peeking, momentum and snap-to-start at the gutter, up to ten
  cards. Pages became full width with the 20 px gutter inside them.
- Placeholders are a grey square with a small grey glyph, hidden while a cover
  shows; Liked Songs uses its violet-gradient heart. Cover slots 190 → 196
  (Home 8 → 14) so a screen of tiles and a shelf can all show art.
- Card menu and Customize Home use the shared sheet rows (icons; switches and a
  move-up control). Audiobook "All audiobooks" is two 212 px columns.
- Play moved from the card overlay to the long-press menu, as on Spotify; Home
  playback still stays in preview (`home-play` updated to that path).

Search browse: "Your top genres" is a carousel of 196 x 112 tiles (up to ten,
edge to edge, snap); "Browse all" is one two-column grid of 214 x 112 tiles
(All songs, Liked Songs, Albums, Artists, Playlists, Genres, Most played, Files
& folders), Spotify-palette colours, bold two-line titles and a sleeve tilted
25 degrees into the corner with a shadow; sleeves are real covers sampled from
each list where the library has one (loaded through the instant-result slots).

Test notes: the harness `drag` hold argument doubles as long-press time; a
0.4 s hold opened a card menu during a "scroll". Scroll drags now use 0.05 s and
journeys wait for a settled layout (`Session.settled()`) after flings, since a
press on a coasting list only stops it (device momentum tuning, `main.c`).
The fixture now gives its six tracks six genres for the carousel.

Verification: new `--journey home-cards` (26 checks: tile size/columns, no
badges, green playing title, edge-to-edge shelf, gutter alignment, peeking
card, sideways flick and snap, no side effects, vertical drag scrolls the page,
long-press menu, scrim dismiss, genre carousel geometry/peek/flick, Browse all
grid and labels).

## 2026-11-04: Profile drawer, Recents, Listening stats, Settings categories, Autoplay

User request: implement the Spotify features first, and make the cards on Home
and Search lead to settings that look more like Spotify's. The "cards" are the
destinations behind the round button beside the tab titles (the former device
sheet: Settings, Devices, Audiobooks, Browse music, More).

Research (accessed 2026-11-04; see records/feature-gap-research-2026-11-04.md):
- 9to5Google 2023-10-04 (read): tapping the profile picture reveals a side menu
  with the profile, settings, listening history and What's New.
- Spotify support "Listening stats" (read): swipe left or tap the profile
  picture, then Listening stats; top artists and songs. Spotify newsroom
  (snippet): the window is the last four weeks; outlets disagree (7 days).
- SlashGear 2025-09-14 (read): Settings categories in order: Account, Content
  and display, Playback, Privacy and social, Notifications, Apps and devices,
  Data-saving and offline, Media quality, Advertisements, About and support.
- Engadget/How-To Geek 2024-11 (snippet): Recents keeps 90 days of history in
  date order with content-type filters. Spotify Community: Recents has moved
  in and out of the profile menu between versions (medium confidence).
- Spotify newsroom 2025-09-05 (read): after the queue ends, recommendations
  play; Autoplay can be switched off in settings.
Row visuals (summary line under each category, no chevrons, 28 px outline icons
in the drawer) are from familiarity with the app, medium confidence.

Decisions:
- Profile drawer replaces the device sheet: a 400 px panel slides in from the
  left over a 60 % scrim (220 ms ease-out). Header: Spotify's default avatar (an
  initial on a coloured disc), "Sonix R1", "View device info" (System page).
  Rows: Listening stats, Recents, Settings; a divider; then the R1's own
  Devices, Audiobooks, Browse music, More. Close by tapping the scrim or
  dragging the panel left past a third. The tab-title button is the same
  avatar. Spotify's Account, What's new, Add account and privacy rows have no
  offline meaning and are omitted.
- Recents: a pushed page; day headings (Today, Yesterday, weekday within a
  week, then "Oct 26"); one row per song per day, newest first, from
  PLAY_HISTORY; at most 100 rows from the newest 320 entries. Rows: cover,
  title, "Song · artist"; tap plays the song in its album (as Search), swipe
  right queues. No content-type chips: music is the only content in the
  history. Entries without a set clock are grouped under "Earlier".
- Listening stats: "Last 4 weeks"; three tiles (plays, songs, artists); "Top
  artists" carousel of round cards ranked with play counts; "Top songs" ranked
  list with play counts; a top song plays the ranked list as "Your top songs".
  Counts come from HOME_PLAYS (Last.fm's counted-play rule). An artist is
  credited through ARTIST_LINK where a track has split credits and through its
  artist tag otherwise, matching the artist page's membership. Unlike Spotify,
  local files count.
- Settings root: Spotify categories, each a 72 px row with a quiet summary line
  and no chevron: Content and display, Playback, Audio quality, Your Library,
  Apps and devices, Controls, Device. Category pages reuse the existing setting
  pages. Music settings were unreachable from Settings (only from the classic
  library and Now Playing); its library and playback rows moved to Your Library
  and Playback, and the remainder is "Audio quality" (Spotify's Media quality).
- Autoplay: on by default, as Spotify; Settings → Playback switch with an
  explanation, persisted in [player] autoplay. When the queue is spent (repeat
  off, no album/folder chaining), up to 25 local songs follow: the finished
  song's artist or genre first, then the rest, shuffled, skipping the last 50
  songs heard. The queue is named "Autoplay". Repeat-one and audiobooks never
  autoplay.

Verification: new `--journey profile-activity` (53 checks: avatar, left drawer,
dimmed page, scrim and drag close, Recents grouping/dedupe/order/art/preview
playback, stats totals/ranking/artist credits/heading layout/ranked-list
playback, category rows and summaries, Playback and Audio quality pages,
Autoplay continue, off, and restart persistence). `device-settings` updated for
the drawer and the Content and display route to Screen. A new probe request,
`end-track`, runs the same advance as a finished track.

## 2026-11-04 (afternoon): Spotify iOS captures -- header, Settings, drawer, stats, Recents

User supplied 78 screenshots of the current Spotify iOS app ("for you to better
recreate them"). Observations and status per screen are in
records/spotify-ios-captures-2026-11-04.md; the images are not checked in
(account, friends, messages).

Corrections to the morning's decisions, from that evidence:
- Settings category rows DO carry a chevron, and an outline icon before the
  name. The morning's "no chevron" (medium confidence, from memory) is
  withdrawn. Settings also has a search disc in its header and three quick
  tiles above the categories.
- The profile menu is about 90% of the width with the profile on a raised,
  rounded card; it was 400 px with a divider under a bare header.
- Listening stats is weekly: "This week" (Monday to Sunday) with minutes
  listened, the top artist and the top song as tiles, then "Past weeks". The
  four-week totals page is replaced.
- Recents groups each day's plays by where they were played from, folded,
  with "N songs played · Playlist"; it listed single songs.

Decisions:
- Shared header: every settings-scaffold page (Settings, its categories,
  Recents, Listening stats, the search page, setting pages) centres its title
  between the back disc and the right corner; the back chevron sits on a dark
  56 px disc on pushed pages (Now Playing's collapse chevron stays bare).
- Quick tiles: Wi-Fi, Bluetooth, Private session. Private session is new: in
  memory only; while on, plays are kept out of PLAY_HISTORY, the play counts
  (HOME_PLAYS, PLAY_COUNTS), LISTEN_TIME and Last.fm, exactly as an audiobook
  is.
- Search settings: a fixed index of every category page and its rows, matched
  case-insensitively on the name, the category and a few words of what it is
  about ("equalizer" finds Audio quality).
- Minutes listened: playcount's measured listening (not skipped through, not
  an audiobook) is added to LISTEN_TIME(day, seconds) a minute at a time;
  400 days are kept.
- Recents context: PLAY_HISTORY gains ctx_kind and context, taken from the
  queue's context when a song starts. Older rows have neither and group as
  "N songs played".
- A week's page (from a tile or a past week) is the former stats content --
  top artists carousel, ranked songs that play as "Your top songs" -- for that
  week.

Verification: `--journey profile-activity` rewritten to the new contract
(73 checks: drawer width and raised card, Recents grouping/folding/indent/art/
playback, weekly tiles and minutes, past weeks and their pages, centred titles,
category icons and chevrons, Private session leaving no history, Search
settings by description and by name, Autoplay as before). A probe request
`field-text <text>` types into the page's first visible field.

## 2026-11-04 (evening): Queue sheet, Go to radio, countdown, Controls, real card

User request: continue the captures; fix the Controls UI in Settings, which
was not centred. The R1 was connected as its microSD card (stock firmware);
the user chose to run the simulator on the card's library rather than flash.

Decisions:
- Controls row: its icon was the control-centre line glyph drawn at full
  size, wider than the icon column, so it ran into the name. Every category
  icon now sits in one 28 px box; Controls and Playback got their own outline
  icons (sonix-controls: the player with its wheel; sonix-volume: Spotify's
  speaker). The Buttons page's Volume steps knob was cut in half at the first
  step by the row edge; the shared slider row now insets the track by half a
  knob on each side.
- Queue sheet (records/spotify-ios-captures-2026-11-04.md): "Queue" over
  "Playing <context>"; Edit and Clear (only with queued songs) pills in the
  corner; the handles are always there, so the separate reorder mode is gone
  from the reference layout; Shuffle, Repeat and Timer tiles along the bottom
  (green when on; Repeat shows repeat-one). Edit opens "Edit Queue": a round
  selector on each row that can be chosen, Move up and Remove along the
  bottom, dimmed until something is chosen. Reshuffle stays in the Now
  Playing menu. Timer is the music sleep timer (5 min to 1 hour, Turn off).
- Now Playing counts down the time left ("-2:52"), as Spotify.
- Now Playing menu: "Go to radio" plays the song, then up to 25 library songs
  chosen as Autoplay chooses them (its artist or genre first, recent plays
  skipped), as "<song> Radio" (context kind RADIO); "Sleep timer" opens the
  same timer choices as the queue tile.
- Scan: the page reports the scan finished once the tracks are indexed; the
  lyric pass for search continues in the background. On the user's card
  (2,387 FLAC) the track pass took 7.5 minutes in the simulator and the lyric
  pass more than 17 more, during which the page sat on Cancel with nothing
  moving.

Real-card run (simulator on the user's card through WSL, 2026-11-04): 2,374
FLAC + 12 MP3, 95 LRC, 2 playlists, 1,811 folders; 2,387 tracks indexed;
Home (shortcuts, Recently added, Made for you, Rediscover), Search (top genre
"anime"), All songs (opened in 2.9 s on the host), playback with artwork and
lyrics, and the new queue all worked on it. Host timings over WSL's file
bridge are not R1 timings. Two things seen: tracks with the same title in two
folders appear twice (":3", "_lullaby"); the index lives in E:\.local.

## 2026-11-04 (night): Search bar, Smart Filters, playlist pills, R1 additions

User request: finish the UI/UX from the captures in the simulator (flashing
later), and add features Spotify lacks that suit the R1.

From the captures (records/spotify-ios-captures-2026-11-04.md):
- Search bar: white, radius 8, black glyph and text, placeholder "What do you
  want to listen to?".
- Your Library: a round filter button before the chips opens "Filter by".
  Offline there are no moods or activities, so it lists the library's genres
  from the tags with album + artist counts; the chosen genre is a green chip
  with a close mark that clears it (not kept across a restart). Albums and
  artists match by their tracks' genre, a playlist by its most common genre;
  folders and the Liked Songs / All songs shortcuts step aside while a genre
  filters. Paging still counts unfiltered rows (a page can show fewer).
- Playlist page: Add, Edit, Sort, Name & details pills in a sideways row under
  the header, running the options sheet's own actions; not on an empty
  playlist, whose "Add to this playlist" already sits there.

R1 additions (research section B; not in Spotify):
- Format badge: the decoded format ("16/44.1 FLAC", "DSD64", "320 kbps MP3")
  centred in Now Playing's shortcut row; tap opens Track information.
- Sound sheet: an equalizer button beside the queue opens Equalizer,
  Parametric EQ, MSEB, Crossfeed and High gain switches, Volume normalization
  Off / Track / Album, and All audio settings. The same calls as the control
  centre; nothing new in the audio path.
- Play next: library song menu, ahead of everything queued by hand
  (Rockbox's insert next); Add to queue still appends.
- Save queue as playlist: Now Playing menu; the playing song and everything
  after it (up to 500) as "Queue <Mon D>", numbered when the name is taken.
- Library health: Settings > Your Library > Library health; read-only counts
  of songs missing an artist or album, with no genre, possible duplicates
  (same title and artist) and lossy files, each opening up to 20 examples
  (file and folder). Never changes or deletes anything.

On the user's card (simulator, 2026-11-04): Library health found 2,385 of
2,387 songs with no genre tag, 233 possible duplicates (mostly a single and
its album both on the card), 30 without an artist or album and 13 lossy
files. With almost no genre tags, the genre filter and "Your top genres"
have little to work with on this library; Autoplay and radio fall back on the
artist.

Known gap: the toast after Save queue as playlist is not seen over Now
Playing (the playlist is saved; checked in the database).

## 2026-11-05: Search clear, Now Playing cleanup, Home card text

User report: clearing the search bar and tapping away left the songs on
screen instead of the browse cards; Now Playing looked messy and showed colour
banding.

- Search: the field has a length limit, and with one LVGL's
  lv_textarea_set_text("") empties it without sending VALUE_CHANGED, so the
  empty search never ran. The clear button now runs the empty search itself
  (Browse returns, chips go), and hiding the keyboard over an empty field
  does the same, whatever emptied it. Reproduced and covered in
  `local-search`.
- Now Playing (Spotify iOS captures):
  - Background: one flat, darkened tone of the sleeve. The fade from that
    tone to #121212 showed as bands on the R1's RGB565 panel; LVGL 9 has no
    dithering to hide them.
  - Header: the context's name alone, 18 px, centred on the collapse
    chevron; the "PLAYING FROM ..." capitals line is gone (Spotify 2026 shows
    only "study music"). The menu dots are horizontal, as Spotify's.
  - Lyrics: the cramped card at the foot is gone; the current line sits
    between the sleeve and the title in 20 px bold at 80%, as Spotify's
    preview does, and the lyrics shortcut opens the full lyrics.
  - The freed space goes to the stack: lyric line, title row, progress,
    controls, and the shortcut row (lyrics, format badge, Sound, queue)
    14 px above the bottom edge.
- Home shelf cards: what it is in grey ("Album", "Playlist"), the name in
  20 px bold, the creator in grey, as Spotify's "Recommended for today"; the
  cards grew 16 px to hold the third line. Artist cards and mixes keep their
  own text.

Verified on the fixture and on the user's card (flat brown from a real
sleeve, no bands).
- Playlist "Recommended songs" (Spotify iOS captures): under the last row of
  a playlist, five library songs by its artists or in its genres that it
  does not hold, each with a (+) that adds it, and Refresh for a new random
  five; anything else in the library when nothing matches. Built on the list
  canvas below the rows (body height grows by the section). Verified in
  `profile-activity` ((+) adds a row to the playlist).

## 2026-11-05 (later): Long-track resume, lyrics timing, album shuffle, artist shelf, Snooze

Goal set by the user: implement what is left from the captures and from the
R1 list.

- Long-track resume (R1): a track of 20 minutes or more ([player]
  long_resume_secs) left partway keeps its place (LONG_RESUME, newest 200)
  and starts there when played again from another track; left within 30 s of
  either end it forgets it. A replay of the same file starts over; books keep
  their own places.
- Lyrics timing (R1): Now Playing menu > Lyrics timing: Earlier / Reset /
  Later in 0.25 s steps, kept per song (LYRIC_OFFSET); applied to the lyric
  line shown above the title, the full lyrics view and tap-to-seek.
- Album shuffle (R1): Settings > Playback > Shuffle by album
  ([player] shuffle_albums). A shuffle deal keeps each album together in list
  order and shuffles the albums; the playing album continues first. The
  album is the album tag, the folder for files the library does not know.
- Home artist shelf (captures: "Discover more from Garoad"): a round
  initial, "Discover more from" in grey over the artist played most these
  four weeks, then that artist's releases as cards. Offline there are no
  similar artists, so it is not "More like".
- Snooze (Spotify, Sep 2025): Now Playing menu > Snooze for 30 days /
  Unsnooze; snoozed songs are left out of Autoplay, Go to radio and playlist
  recommendations (SNOOZE table).

The Now Playing menu now runs past the screen and scrolls, as Spotify's
does.

## 2026-11-05 (evening): Storage page, Daylist

- Storage (R1): Settings > Device > Storage: card space used of total
  (statvfs), songs per file type from the index, and the index's own size.
  Read-only; also found through Search settings.
- Daylist (Spotify, offline): first card of Made for you, named for the
  moment ("Wednesday evening"): songs played within two hours of the current
  local hour over the last 60 days, most played first, snoozed songs left
  out (LIBRARY_FILTER_HOME_DAYLIST, appended to the filter enum so stored
  values keep their meaning).
- Fixed: a raw NUL byte had been written into settings.c by an earlier edit
  (the Library health sheet's ' '); it compiled the same but made the file
  binary to tools. Now the escape.

All of these were built later the same day (see "the rest of the captures and the R1 list").

- Hide in this playlist (Spotify, Sep 2025): the song menu inside a playlist
  offers Hide / Show in this playlist (PLAYLIST_HIDE). The song stays in the
  playlist and on the card; when the playlist plays, the automatic advance
  goes past it (bounded, so an all-hidden playlist still ends). Next pressed
  by hand still lands on it, and hidden rows are not dimmed yet. Verified in
  `player-core`.

## 2026-11-05 (night): the rest of the captures and the R1 list

User request: finish implementing everything before flashing.

- Hide in this playlist, finished: Next by hand also goes past hidden songs,
  and hidden rows are faded (40%) in the playlist.
- Liked Songs genre chips (Spotify): the genres most liked songs carry, as
  chips under the Liked Songs header; a chip opens "<Genre> · Liked Songs"
  (filtering in place is replaced by a list of its own).
- Up next and Credits (captures): Now Playing does not scroll on the R1, so
  Spotify's two cards open as a sheet from "View song credits" in the Now
  Playing menu: Up next (the next song, Open Queue) and Credits (artists,
  album artist, composer, album and year).
- Composers (R1): COMPOSER (Vorbis) and TCOM (ID3) are read and kept per
  track (TRACK_COMPOSER); Search > Browse all > Composers lists them with
  track counts and opens a composer's tracks. A card indexed before this
  shows composers after its next scan.
- Playlist cover (captures, Name & details): playlist options > Choose cover
  lists the playlist's albums (and "First song"); the choice is used for the
  playlist in Your Library and on its page. Home cards still show the first
  song's art.
- Sound per output (R1): [output_wired] / [output_bluetooth] keep the EQ,
  PEQ, MSEB and crossfeed switches and the graphic EQ bands; switching output
  keeps the old and restores the new (a new output starts from the sound in
  use). The Sound sheet says which output it is kept for. Detection is
  Bluetooth audio active or not; verified with a probe override only.
- Smart Shuffle (Spotify): the shuffle control steps Shuffle, Smart Shuffle,
  off (Now Playing and the queue tile, which reads "Smart Shuffle"). Smart
  Shuffle puts up to ten library picks into what is still to come, one after
  every three: a playlist's recommended songs, otherwise Autoplay's picks for
  the playing song; marked with a star in the queue; taken out when it is
  turned off. Tied to the context it was started in.
- Smart playlists (R1): Your Library > Create > Smart playlist: Most played,
  Recently added, Never played, Lossless files, Genre is..., Artist is...,
  Year is...; the rule is kept (SMART_PLAYLIST) and the playlist is filled
  again from it each time it opens (at most 500 songs).
- Fixed: medialist_open_paths() switched the panel to a path list before
  clearing the previous list's head, which repainted rows from an empty list
  and crashed (found opening a composer after an album).

## 2026-11-05 (device): first flash -- freezes, scan, duplicates, display size

User report after flashing the R1: everything too small; touch not very
responsive; heavy lag, stutter and freezes needing resets; music not appearing
properly in the library; search not working as intended; Scan staying at 0
tracks; duplicate albums.

Found (evidence: a copy of the device's library.db, measured on the PC):

- Duplicates: the index held the card twice -- 2,387 tracks under /mnt/e (the
  simulator indexed this card from the PC on 2026-11-04) and 2,384 under the
  device's /mnt/sd_0. Every album was listed twice and the database was 44 MB,
  376,361 of its rows the search word index.
- Freeze and Scan at 0: a scan begins by deleting MEDIA_TABLE, and each deleted
  row's trigger deleted from SEARCH_DOC by path with no index on path -- a full
  read of every song's lyrics per row, under the one lock every page reads the
  library through. 73 s for the whole delete on the PC; on the R1 the count
  never left 0 and the next page that touched the library waited for good.
- Background search indexing (one card commit per song for lyrics, 688 ms per
  16-song batch even on the PC) and pages polling library_revision() under the
  same lock made the interface wait whenever indexing ran.
- Home was rebuilt -- every shelf's queries and widgets -- on every return to
  it: each Back, each tab tap.

Changed:

- A scan empties the search, release and dependent tables by truncation before
  the media rows, so the triggers find nothing (220 ms on the real index on the
  PC; the count moves at once). SEARCH_DOC gets an index on path; an index too
  big to build it in place is emptied and rebuilt in the background.
- On the device, Detect changes first forgets index rows under any root but the
  card's (in 200-row steps, the lock released between); no file is touched.
- library_revision() and library_is_open() read without the lock.
- The lyric pass reads files with the lock free and writes 4 songs per commit,
  and it and the post-scan search catch-up run only after 3 s without a touch.
  Lyrics are de-duplicated word by word before indexing. A search keystroke
  catches up at most 64 tracks; lyrics match from the third letter against 48
  candidates.
- Home is rebuilt only when something it shows changed (tracks, likes,
  playlists, plays, history, collections), and keeps its scroll position.
- Display size (Settings > Appearance): Large (default on the R1) lays the
  interface out at 384x640 and draws it up 1.25x to the 480x800 panel,
  bilinear, turned with Rotate screen; Standard is the previous pixel-for-pixel
  layout. The R1's screen is 3 inches (480x800, about 311 ppi; HiBy guide,
  retailers). A change restarts the player (on the R1 the device restarts:
  sonix_launch reboots when the player exits). Large also draws 36% fewer
  pixels per frame.
- The Search bar's clear button appears only once something is typed, as in
  Spotify; empty, the field has the whole bar and the placeholder is not cut
  short (it was, at Large).
- Known at Large: a single word too long for a Home shortcut card wraps by
  letter ("Unillustrat / ed"); settings summaries are cut at the edge.
- Diagnostics in the log (Developer options > Log to microSD): a database wait
  or hold of 150 ms or more, with the function; an interface pass of 150 ms or
  more, with the page; how long Home took to build.

Not yet measured on the R1: frame times, the scaling's cost per frame, touch
latency, and how long the first background rebuild of this card's search index
takes. Touch responsiveness is expected to follow from the interface no longer
waiting on the database; the log will say whether it does.

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


## 2026-11-05 (device, round 2): artists, duplicate songs, Now Playing, scrolling, touch

Owner report after the second flash: touch working (after the fix below);
artists duplicated whenever a song has a featured artist; songs duplicated in
albums; Now Playing not the Spotify design; Your Library needs scrolling, not
"More". Evidence: a copy of the device's library.db and collections.db, pulled
over adb, and the touch panel's axis ranges read on the device.

- Touch (Display size Large): the R1's panel (hyn_ts) reports multi-touch axes
  only and advertises 0..720 x 0..1280, but its coordinates are the panel's
  pixels (0..479 x 0..799): taps were right passed through at Standard, and on
  upstream Sonix. The calibration is the panel's pixel range, scaled by LVGL
  to the layout; the advertised range is logged.
- Artists: the card's tags are Spotify's -- "Bad Bunny, Diplo" in one ARTIST
  value -- and splitting was off by default (and did not split at commas), so
  every combination was an artist of its own; the release credits search and
  the artist pages read were stored joined the same way. Splitting artists and
  album artists is now on by default, at ; / , and feat. -- not &, which these
  tags keep inside names ("Zion & Lennox", "Jowell & Randy") -- with default
  exceptions for well-known comma names ("Tyler, The Creator", ...). Credits
  are split with the same rules. An index filed under the old settings is
  filed again on the next start and its credits re-split (890 on the device's
  copy: 521 artists, none of them a combination; "Bad Bunny" once).
- Songs in albums: the card holds 61 tracks twice -- the same album in two
  folders ("Artists/AZALI/..." and "Artists/study music/AZALI/..."). Track
  lists show a track once when another with the same album, disc, number and
  title was indexed before it. Both files stay; untagged tracks are never
  folded together.
- Now Playing: the Spotify arrangement was the default only at exactly
  480x800, so Large (384x640) brought up the old one. It is the default at
  every size, and with it the controls always get the 320 pixels they need;
  the artwork gives up height on a short layout (about 255 panel pixels wide
  at Large, 364 at Standard).
- Your Library scrolls continuously: pages of 24 are read as the list nears
  its end, kept in a window of 96 rows (a ring, so callbacks keep pointing at
  their items); far down, the first pages go and come back on the way up, the
  list held still on screen as rows come and go. Verified on the device's copy
  (about 340 items down and back to the same top).
- The lyric pass no longer treats a file that is not there right now as
  changed (it emptied every song's lyrics when the music was not mounted).

## 2026-11-05 (device, round 3): lyrics, the catalogue, and the lag measured on the R1

Owner report: a grey block behind the sleeve after the lyrics; the line above
the title cut with "..."; the artist catalogue paged with More; the chip row
dragging the whole artist page; Search to Home taking 2-3 s.

Measured on the device itself (adb): taps injected into the touch node and the
visible framebuffer page timed until it changes and settles (scratchpad
taptime.c); the player's log on the card for passes, database waits, repaints
and each step of a wake; test builds run from the card without flashing.

- Lyrics: closing them took the Spotify arrangement's clear panels back to the
  theme's grey card. They stay clear.
- The line above the title shows a long line whole, in pieces that each fit,
  one after another across the time the line is sung (a 107-character line:
  four pieces, three seconds each).
- The artist catalogue: "All" is every release in one list with a heading
  where the kind changes; pages of 12 are read as the list nears its end, with
  no More button; covers only for the rows on screen. The chips scroll
  sideways in their own row; the page does not move. A name split out of one
  artist tag on a release someone else owns is a guest (Featured On), as
  Spotify lists appearances; an index split before this is reclassified once.
- The lag. Search's first open took 2.9 s: ten tiles each opened a whole track
  index for one cover (230-465 ms each on the R1). Your Library took 1.2-1.3 s
  per visit: rebuilt every time, and each artist row's cover opened that
  artist's whole index (up to 1.2 s). Home rebuilt in 0.9-1.0 s. Repaints took
  100-345 ms. Changes: a one-row cover query (library_sample_path); Your
  Library refreshed only when something it lists changed; Search and Your
  Library built ahead, and stale hidden pages refreshed, while the screen is
  left alone; Browse kept while the library is the same; repeated tracks
  worked out once per scan into TRACK_REPEAT instead of a subquery per row;
  "recently added" from the newest 600 tracks by an index; no corner clipping
  on Home and Search tiles (a layer per tile per frame; artists stay round).
  Result on the R1: every tab switch starts drawing within 113-167 ms and
  Home settles, covers included, within about 300 ms; repaints mostly under
  80 ms.
- A wake takes about 500 ms, 300 of them the kernel bringing the panel and
  its touch controller back up; a tap in the first half second lands before
  the controller is on. Logged step by step.

## 2026-11-05 (device, round 4): seventeen items from the owner

Each was reproduced on the R1 over adb where it could be (injected touches,
framebuffer captures, Display size Large) or in the 480 x 800 simulator at
Large, then fixed in the shared component.

- Now Playing opened on scroll-ups away from the mini-player: the opening drag
  was attached to whole pages and lists. It now starts only on the mini-player
  (player_sheet_set_handle), or within 48 px of the bottom edge on a page with
  none. Spotify opens Now Playing from the mini-player only.
- Singles & EPs: the category needed a RELEASETYPE tag the owner's files do
  not carry. Untagged records are filed by size, after the published rule
  (1-3 tracks a single, 4-6 an EP, 7 or more an album) on distinct titles, or
  by a name ending "- Single" / "- EP". No lengths are indexed, so the
  30-minute part is not applied; a partly copied album can read as a single.
- Images: each list size (48 ... 78 px) was its own decode of the same file.
  Below 128 px a thumbnail is now shrunk from one master thumbnail per file,
  decoded once and kept in the on-disk cache.
- Your Library: a fast-scroll handle on the right edge, shown while the list
  moves; dragged, a bubble names the record under it (the letter in A-Z), and
  on release the window is read from there. Tapping the Your Library tab again
  goes back to the top.
- FLAC badge: its text sat high in the outline (a fixed 28 px label box); it
  is now one line of the font, centred both ways.
- Queue: "Next from <long album>" wrapped onto a second line under the first
  cover; the heading is one line, cut with an ellipsis.
- Sound sheet: capped at the screen's height at Large, the sheet cut off its
  last rows. Every bottom sheet's body now scrolls what does not fit.
- Settings keep their place: a page left for one of its subpages stays where
  it was; only a page left by going back reopens at the top.
- Update from SD card is back under Settings > Device > System (card only,
  charge check, confirm, recovery installs it).
- Equaliser: twelve more presets (R&B, Hip-Hop, Electronic, Latin, Deep,
  Lounge, Piano, Loudness, Spoken word, Small speakers, Bass and Treble
  reducer) and a row of preset chips above the curve, the one put on filled,
  with "+ Save" for one's own. The reset icon went: Flat is the same, and the
  title needed the room at Large.
- The cover, tapped: shown as large as the screen allows over the dimmed
  player, sharp from a decode at that size; the X or any tap closes it.
- The dots: at Large the artwork panel is narrower than the screen, so the
  dots hung beside the title; they are in the screen's corner now.
- The "..." menu stuttered: the player and every sheet live on LVGL's top
  layer, and LVGL drew the screen under it, the whole player and the veil on
  every frame of a scroll. A local LVGL patch (sonix-player/patches/lvgl)
  starts drawing from the topmost opaque object on the top layer as well.
- The progress line: a 4 px line was the whole target. It now catches 24 px
  around it, so the knob at either end too, and the knob grows while held.
  New setting Settings > Playback > Now Playing > Progress bar: Line (default)
  or Waveform.
- The swipe-down panel at Large: 88 px circles wrapped three to a line and the
  lines were cut off top and bottom. The circles are sized to fit four to a
  line and two lines (67 px at Large); the brightness track is thicker with a
  wider catch area.
- About: the device's name and picture (Name and Picture rows; the picture
  from images at the top of the card or in its Pictures, Photos, Images, DCIM
  folders). Both head Settings in a card, the drawer and the avatar beside
  each tab title. The owner's picture was copied to the card as
  .local/profile.jpg, which is used until a picture is chosen.
- Storage: the card in one bar split into Music, Audiobooks, Photos, Books,
  Player data, Other and Free, a line each, and the R1's internal memory (the
  OS) apart. The card is walked on its own thread when the page opens.
- Also found: the profile drawer was 432 px wide on the 384 px Large layout;
  it is now at most the screen less 48 px.
- Open observation: once, after an album started from its page, the queue
  read "Playing Autoplay" while the album's next tracks were listed; not
  reproduced, no log of that run on the card.
