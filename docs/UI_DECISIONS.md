# UI decisions and recurring corrections

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
