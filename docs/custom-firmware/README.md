# R1 native discovery development fork

This is an implemented, testable development branch, **not a production firmware
release**. It adds a native LVGL shell and an authorized download path to Sonix.
The latest-board kernel/radio compatibility and physical-device acceptance
criteria remain blocked. No device has been flashed and no `r1.upt` is supplied.

Baseline: `Jepl4r/sonix-player@fe94fe796d93c6efa50d1107a734864c9c6a03d5`.
Fork: [Alephx0/sonix-player](https://github.com/Alephx0/sonix-player), branch
`feature/r1-discovery`. Original Sonix code and licensing remain in place.

## Implemented workflow

1. Home offers recent listening, recently added albums, favourites and playlists.
2. Search has All / Local / Online filters and the native touch keyboard.
3. Online finds Internet Archive releases with explicit supported public licences.
4. Open a release and tap an available audio format to enqueue its source file.
5. Your Library > Downloads shows progress, pause/resume, retry, cancel and play.
6. A completed, checksum-verified file is imported into the existing local library.
   Existing embedded tags are retained; missing title/artist/album use an attribution
   sidecar. The original audio bytes are preserved and can be decoded offline.

Tools exposes all six former top-level destinations. Existing Now Playing,
waveform and Studio layouts, streaming integrations and audio controls remain
in the source. Their full hardware behavior has **not** been regression-certified.
This fork does not implement a Spotify service client.

## Evidence and guides

- [Architecture and interfaces](architecture.md)
- [Build, test and maintenance commands](development.md)
- [Test results and performance limits](test-report.md)
- [Original and requested feature matrix](feature-matrix.md)
- [Hardware, packaging blocker, installation and recovery status](hardware-and-packaging.md)
- [REA / SpotBye / Mornye investigation](reference-investigation.md)

## Simulator screenshots

Actual 480 × 800 SDL/LVGL captures; no browser mockup. Empty collections reflect
the small test library. Screenshots are a layout check, not a hardware test.

| Home | Search | Library |
|---|---|---|
| ![Home](screenshots/home.png) | ![Search](screenshots/search.png) | ![Library](screenshots/library.png) |

| Online releases | Source formats | Downloads |
|---|---|---|
| ![Releases](screenshots/online-results.png) | ![Source files](screenshots/online-release.png) | ![Downloads](screenshots/download-verified.png) |

Also captured: [keyboard](screenshots/keyboard.png) and
[existing Now Playing](screenshots/now-playing.png).

## Remaining product work

The feature matrix is the authoritative scope ledger. In particular, online
artist pages, independent track/playlist search, remote artwork caching,
streaming previews, provider configuration, a newly designed Now Playing layout,
configurable Home shortcuts, pinned collections and translations of new strings
are incomplete. SpotBye dynamic network tracing needs an isolated Windows VM.
Hardware testing, prolonged playback/download stress and latest-board recovery
are still required. Do not interpret successful host tests or MIPS compilation
as completion of the original production-firmware acceptance criteria.
