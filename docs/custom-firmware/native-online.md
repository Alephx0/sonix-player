# Native online workflow: first increment

The user selected adapting SpotBye's search/browse/download workflow into Sonix,
not connecting to an existing SpotBye backend. This is an independent C/LVGL
implementation. It uses the existing Internet Archive provider; it is not a
Spotify catalog or SpotBye service client.

## What changed

- Search accepts `https://archive.org/details/<release>` in All or Online mode.
  It extracts a bounded release identifier and uses the existing HTTPS metadata
  API. Arbitrary hosts, credentials, traversal, nested paths and unsupported
  Spotify links are rejected rather than fetched or silently treated as music.
- Opening an online audio result shows a bottom sheet with track, artist, album,
  provider, original format, file size and source licence. Opening details does
  not download or change playback.
- Download to microSD explicitly enqueues/resumes the selected source and opens
  Downloads. More by this artist runs creator search; View downloads opens the
  existing managed queue. Selection metadata is copied so asynchronous search
  completion cannot change the track acted on by the sheet.
- Existing pause/resume, retry, duplicate detection, checksum verification and
  post-download local indexing remain shared. No extra worker or runtime was
  introduced. Sources explains the supported capabilities and limitations.

The [Archive metadata contract](https://archive.org/developers/md-read.html)
defines release records and file metadata. The provider still checks supported
source licences and integrity metadata before accepting audio.

## Companion playback fixes

Liked Songs and All songs artwork now passes touches to its collection row in
both List and Grid. Starting a track, Play or Shuffle in a reference-layout
collection updates the mini-player while retaining the list. Only tapping the
mini-player expands Now Playing. All songs now shares the persistent footer.

## Reproduce

```sh
python3 tools/test_ui_collections.py /home/justi/sonix-r1-build --journey preview-online
python3 tools/test_ui_collections.py /home/justi/sonix-r1-build
cd sonix-player
bash tests/discovery/run.sh
cd ..
python3 tools/test_live_download.py /home/justi/sonix-r1-build --fresh
```

UI tests render actual LVGL 9.6.0 at 480 × 800. Online-sheet visual tests use
synthetic metadata parsed by the production provider, and do not download the
synthetic track. The independent live test downloads a supported public-domain
source, checks its bytes, imports metadata and decodes audio locally. Provider
tests cover malformed links, paging, transfer/resume failures and TLS validation.

Remaining: additional providers, Spotify catalog authorization, streaming preview,
online playlists, remote artwork thumbnails, and connected-device validation.
No claim of complete SpotBye compatibility or hardware readiness is made.

## Evidence

The focused UI journey passed 15 checks. Captures were inspected at native size:
[Liked Songs artwork](screenshots/portrait-liked.png),
[All songs artwork](screenshots/portrait-all.png),
[playlist playback preview](screenshots/playlist-preview.png),
[online track details](screenshots/online-track-details.png), and
[download queue](screenshots/online-downloads.png).

[UI report](evidence/preview-online-ui.json) and
[fresh download/import/decode log](evidence/preview-online-live.txt) preserve the
results. The live test transferred 6,419,897 bytes in one attempt, verified the
source MD5, indexed the file, and decoded 1,024 frames at 44.1 kHz stereo.
The simulator sample used 41,372 KiB RSS and about 5% of one host CPU core over
one second. These host observations do not establish a device memory budget.

The [full collection regression](evidence/preview-collections-ui.json) passed
71 checks, including Play/Shuffle remaining collapsed, priority isolation and
restart persistence. [Provider/queue/TLS tests](evidence/preview-online-provider.txt),
host build, MIPS target build and the glibc <= 2.22 ABI check also passed.


## Integrated search and release milestone (2026-10-28)

All / Local / Online now drive separate bounded local and online workers. Local
results are published first; new queries remain usable during slow network I/O.
Query generations discard stale responses. On the target, an unavailable Wi-Fi
connection suppresses online work while leaving the local results available.

The versioned native provider descriptor names capabilities and search/release
operations. Internet Archive remains the only implementation. Online release pages
reuse `collection_header.c` with medialist and mount the existing mini-player.
Track taps open the same source-details sheet. Download manager entries group by
release, show progress/format and support the existing individual transfer controls.
Completed indexed items use their embedded title and link to the indexed library.

A complete response of up to 24 files may be queued atomically. The fixed queue has
32 slots, one worker and the existing 16 KiB transfer buffer. A capacity failure
leaves the queue unchanged. Existing source URL + MD5 jobs are deduplicated; completed
indexed copies can be played directly, including after restart. The worker's slot
must never move during batch admission. Larger/paged releases use individual actions.
All source files remain explicit, including alternate formats; no cross-provider
recording substitution or format conversion is performed.

Remaining work is explicit: larger multi-page album jobs, a durable source-to-library
mapping independent of retained queue jobs, ISRC/version-aware candidate matching,
structured online discography categories, remote artwork thumbnails, additional
providers/authentication, and R1 timing/RAM/standby/DAC validation. Archive creator
search is not a verified artist identity or featured-credit graph.

### Source review

The supplied repository root and licence URL could not be fetched directly during
this run. Indexed primary-source pages for
[SpotiFLAC main.go](https://github.com/spotbye/SpotiFLAC/blob/main/main.go) show a Go
backend bound into Wails with embedded frontend assets. Its
[history.go](https://github.com/spotbye/SpotiFLAC/blob/main/backend/history.go) stores
source, path, quality, format, catalog ID and timestamps in a bounded bbolt history.
These support separation of UI, provider metadata and download history; they do not
establish the supplied SpotBye.exe's provenance or licence. No executable was run,
no Go source copied, and no private provider endpoint adopted. The existing static
binary audit remains the evidence for that exact executable.


### Integrated milestone evidence

- [Collection regression](evidence/online-collections-ui.json): 71 checks.
- [Online release and local-first search](evidence/online-release-ui.json): 14 checks.
- [Live release-download action, automatic import, existing-copy reuse and playback](evidence/online-live-ui.json): 12 checks.
- [Provider, atomic queue admission, restart and TLS](evidence/online-release-provider.txt).
- [Fresh transfer then network-blocked import/decode](evidence/online-release-offline.txt).

Inspected native 480 x 800 renders:
[release](screenshots/online-release-native.png),
[source details](screenshots/online-release-details.png),
[local hits while online loads](screenshots/unified-search-local-first.png),
[live offline playback](screenshots/live-download-playing.png), and
[existing local copy](screenshots/live-existing-copy.png).
The local-first journey sampled 42,084 KiB host RSS and 8% of one core over one
second. The live UI journey's separate measurements are retained in its report.
Host build, MIPS target build and glibc <=2.22 ABI verification passed. These
captures are candidates, not approved visual baselines; hardware remains untested.
