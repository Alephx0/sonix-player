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
