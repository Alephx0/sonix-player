# SpotBye 2.0.0 inspection and native parity

Inspected 2026-10-28. This is a partial native adaptation, not full SpotBye
provider compatibility and not a flashed/tested R1 firmware.

## Evidence and coverage

The user-opened Windows executable was inspected with REA `inspect_go_binary`.
SHA-256: `fd46c09b4797b2ca7c645819a8b9e44220390a89efb8df98beeb9f536f3dd9e1`.
[REA evidence projection](evidence/spotbye-runtime-rea.json) records the evidence ID.
Its embedded build metadata identifies an AMD64 Go/Wails application, with WebView2,
bbolt and tagging dependencies. Build strings do not authenticate its publisher.
The screenshots and exact full REA bundle are retained in ignored
`research/spotbye-runtime`; account identifiers and desktop paths are not published.

Live UI observations used Windows pointer input and screenshots. REA's native
UI driver does not support Windows. The running WebView2 exposed no CDP listener;
a graceful restart after observing zero active/paused downloads, with a process-local
debug flag, still exposed none. No authenticated network/API trace was obtained.
No SpotBye setting was saved, no download initiated, and no credentials read.
The app was left open. REA analysis was closed after export.

The supplied `spotbye/SpotiFLAC` and embedded `afkarxyz/spotbye` repository roots
both returned 404 during this review. No proprietary frontend/backend code was
copied. This is independent implementation of observed UI behavior and existing
Sonix mechanisms; static symbols do not prove provider protocols.

## Observed reference versus implementation

| Reference observation | Native status |
| --- | --- |
| Tracks / Albums / Artists / Playlists catalog tabs, 50 results each in the supplied search | Not equivalent: bounded Archive release/creator search remains the only discovery provider |
| Sources separates Metadata, Lyrics, Downloaders, BYOA | Capability boundary retained; no fake working Tidal/Qobuz/Amazon/Deezer switches |
| Spotify Web metadata and MusicBrainz genre source visible | Not integrated into discovery |
| Tidal, Amazon, Deezer downloader switches; Qobuz Maintenance | Observed UI only; provider contracts, authentication and authorized download support unresolved |
| BYOA Tidal/Qobuz/Deezer/Amazon require authentication/account/cookie | Account state untouched; no credential extraction or access-control/DRM bypass implemented |
| General / Naming / File Management / Metadata settings, explicit Save Changes and Reset to Default | Native Sources / Transfers / Files groups with Save, Discard and draft Defaults |
| Filename and folder templates, many tokens | Three safe filename presets; source suffix distinguishes recordings; fixed microSD Downloads directory |
| Rate-limited downloads | Native Unlimited / 128 / 512 KiB/s options; one transfer, interruptible throttling |
| Existing File Check: Hybrid; redownload suffix, cover, M3U and failed-log options | Size+checksum verification and no-replace finalization retained; other switches not copied as inert controls |
| Tag toggles, separator/date format, conversion/resampling/ReplayGain | Original source tags/audio preserved plus source sidecar; tag editing and conversion not implemented |
| Queue All / Active / Paused / Done / Failed; Start All | Native five filters, counts, Start all, Pause all, individual pause/resume/retry/cancel/play |
| Download queue text search and type selector | Pending |

Native start-paused and bounded 1-3 network attempts are practical R1 controls;
they are not claimed to reproduce an unobserved SpotBye algorithm. Preference
changes are drafts until Save. Leaving/reopening discards unsaved changes. Defaults
also requires Save. Disabling discovery stops new queries; transfers have separate
pause controls. Preferred format changes ordering, never converts a recording.

Filename choice is captured per job and persisted; changing preferences never
renames an active/completed file. Existing version-1 manifests lacking a destination
retain their previous naming. New destinations reject path separators. Downloads
stay separate from playback. Restarted jobs continue to require explicit Resume,
including re-verification of retained completed files; exact SpotBye history
restoration is not claimed.

## Verification

- [Native settings/filter journey](evidence/spotbye-settings-ui.json): 16 checks,
  including Save/Discard/Defaults, real download admission while paused, filtering,
  playback preservation, process restart and persisted destination.
- [Live provider UI regression](evidence/spotbye-settings-live-ui.json): 12 checks.
- [Provider/queue/TLS tests](evidence/spotbye-settings-provider.txt): sanitizers,
  saved preferences, naming stability, limited transfer timing, retry limit,
  atomic release admission, integrity, interruptions and restart.
- [Fresh transfer and offline decode](evidence/spotbye-settings-offline.txt):
  6,419,897 source bytes, checksum, indexing, 1,024 decoded frames after host
  seccomp denies new network connections.

Inspected 480 x 800 native renders: [sources](screenshots/online-settings-sources.png),
[transfers](screenshots/online-settings-transfers.png),
[files](screenshots/online-settings-files.png),
[paused queue](screenshots/downloads-paused.png),
[empty filter](screenshots/downloads-done-empty.png),
[after restart](screenshots/online-settings-restored.png).
These are candidate captures, not approved baselines. Host RSS was 40,184 KiB,
with 1% of one core over a one-second sample; neither proves R1 performance.
Host and MIPS builds and the glibc <=2.22 ABI gate passed. Device flashing,
Wi-Fi/DAC behavior and full provider parity remain unverified.


## Supplied-source follow-up

The user subsequently supplied an accessible MIT-licensed SpotiFLAC fork.
[Source audit and native catalog implementation](spotiflac-source-audit.md) supersedes
the earlier statement that no source reference was accessible. It does not establish
a source-to-binary relationship for SpotBye 2.0.0. A new Deezer metadata adapter
adds actual track/album search, direct links, IDs/ISRC, release lists and artwork.
Full-audio download providers and exact SpotBye parity remain unresolved.
