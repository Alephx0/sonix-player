# Validation report — 2026-10-28

Status: host-tested development implementation; production/device acceptance is
incomplete. Test commands are in [development.md](development.md). Captured output
is under [evidence](evidence/) and actual simulator images under [screenshots](screenshots/).

The subsequent [reference UI validation](reference-ui.md) records the redesigned
screens, album mini-player navigation checks and refreshed build evidence.

See [sliding-panel validation](sliding-panels.md) for the newer modal, artwork
search and metadata discography checks.

## Offline integration update

Host and MIPS builds pass after the offline queue/library additions, along with
queue/session, offline-library, import, discovery/TLS sanitizer and packaging
regressions. New touch captures cover priority queue gestures, selection move-up,
instant search before submission, playlist membership, saved-collection artwork,
lyrics and all four player layouts. See [offline-player.md](offline-player.md)
for current evidence and limitations. The older performance samples below
predate these additions and must not be treated as current memory measurements.

## Passed checks

| Check | Result | Evidence |
|---|---|---|
| Native host compile/link | Pass, real Sonix/SDL/LVGL executable | `make host`, simulator startup/navigation |
| R1 CPU target build | Pass, ELF32 little-endian MIPS32r2, o32, hard-float | Target ELF inspection and binary hashes |
| Target glibc ceiling | Pass, no imported symbol beyond 2.22 | [ABI output](evidence/target-abi.log) |
| Discovery/download deterministic suite | Pass with ASan + UBSan | [suite output](evidence/discovery-test.log) |
| Real local HTTPS transport | Pass with certificate verification | Same suite: ranges, lengths, redirects, bad certificate, downgrade and encoding rejection |
| Incremental library import | Pass against actual host app objects | [library output](evidence/library-test.log) |
| Fresh live-source integration | Pass: download, MD5, indexing, offline decode | [live output](evidence/live-fresh-test.log) |
| Packer compatibility/size guard | Pass, two test methods with multiple cases | [packaging output](evidence/packaging-test.log) |
| Touch keyboard, release search/listing, navigation | Pass by capture/review | Home/Search/Library, keyboard, online release/audio variants and Downloads PNGs |
| Provider preferences and repeated navigation | Pass | Saved enablement/MP3 preference asserted from config; 20 Home/Search/Library cycles without process exit |
| Sonix Now Playing with/without artwork | Pass by capture/review | Uses synthetic original artwork fixture, inset art and existing transport widgets |
| Original three player layouts | Host layout checks pass | Standard, Waveform and Studio captured with the same fixture; this does not certify playback behavior |
| Stock OTA integrity | Pass, 85 chunks | [stock output](evidence/stock-integrity.log); integrity is not compatibility/authenticity |
| Paged catalog and creator releases | Pass, deterministic and live provider tests | [live catalog output](evidence/live-catalog-test.log); touch paging/artist captures |
| Touch-initiated download and automatic library insertion | Pass on host with actual HTTPS and scanner | [touch output](evidence/touch-download-test.log), 3,176,440 bytes with matching MD5 |
| Failed import fairness and retry | Pass with ASan + UBSan | [suite output](evidence/discovery-test.log); removed/undecodable jobs do not starve later completions |

The deterministic queue suite exercises licence/source rejection, malformed
metadata, incremental MD5 vectors, filename sanitization, exact file bytes,
duplicate enqueue, interrupted transfer with resumed offset, pause/resume,
cancel, corrupt data, simulated full storage, removed/replaced card root,
persisted-paused recovery, symlinked download directory rejection, and a
concurrently created destination that must not be replaced. The full-storage
fixture wraps both `fstatvfs` and `fstatvfs64`, matching the app's large-file flags.

Pagination fixtures cover literal escaping, creator-only queries, page bounds,
final and empty-but-not-final catalog pages, and a 30-file release split into
24/6 variants. Live catalog pages returned 11 and 12 eligible releases with
distinct first identifiers. The creator phrase returned one eligible release.
The full touch workflow downloaded the selected Bach release's MP3 variant into
a fresh card root and observed the main application's completion poll import it
into SQLite, without calling the import API from the test. This verifies UI-to-
worker-to-library integration on the host; it does not establish device playback.
The final touch run used the revised fair import scheduler and completed 20
additional Home/Search/Library cycles without process exit. Next/Previous
gestures were checked in a separate [paging run](evidence/paging-test.log).

TLS tests use an actual temporary certificate/server and the app's HTTP/TLS code.
They check accepted/rejected ranges and totals, ignored-Range restart, exact
length, encoded/chunked refusal, HTTPS redirects, redirect limits, insecure
redirect refusal, HTTP 404, and bad peer identity. Mocked queue transport tests
and actual TLS tests are separate, not presented as one physical-network test.

Library tests import two generated WAVs into an empty database, preserve existing
tracks, avoid duplicates, fill metadata from an attribution sidecar, retain the
Detect changes setting and verify persistence after reopen.
They also verify newest-first history is capped at 64 rows and a cached absent
thumbnail becomes a decoded image when artwork is added beside unchanged audio.

The live test uses Archive `loc-jukebox-733632-toccata`, whose metadata declares
the public-domain mark. In a **new card root**, the real worker downloaded
6,419,897 bytes, passed the provider MD5 in one attempt, indexed the result and
decoded 1,024 PCM frames at 44,100 Hz/stereo/MP3. The source filename was the
available track title. No credentials, companion or mock transport were used.
Decoding a prefix proves that path works; it is not an entire-track integrity
or gapless/audio-output test. Rerunning without `--fresh` verifies cached bytes.

## Host performance observations

Both captures use 480×800, Xvfb, SDL software rendering and Ubuntu x86-64 under
WSL. The baseline was measured before changes, using the same rendering mode.

| Metric | Baseline | Integrated sample |
|---|---:|---:|
| Process launch to visible SDL window | 0.141 s | 0.133 s |
| Resident memory at final sample | 36,020 KiB | 39,480 KiB |
| Process high-water RSS | 36,020 KiB | 40,352 KiB |
| Threads at sample | 22 | 23 |

These are single observations, not statistically controlled benchmarks. The
integrated sample includes 20 navigation cycles, online search, an indexed audio
fixture and synthetic artwork; the baseline has less work. Final RSS difference
is 3,460 KiB (about 9.6%). Virtual size
is much larger because of host library/thread mappings and is not resident RAM.
Window visibility is not device cold boot or a guarantee that every page is ready.

A provisional host gate is visible window within 1 second, no crash through the
scripted route, and sampled RSS below 45 MiB under this fixture. These samples
pass. It is explicitly not the R1's whole-system 64 MB budget. Target ELF text,
data and BSS are recorded in [target-size.log](evidence/target-size.log); ELF sizes
do not include kernel, services, dynamic buffers, graphics or decoder working sets.

## What remains unverified

- Physical cold boot, total RAM/fragmentation, flash partition space, source
  filesystem atomic rename support, battery life, suspend/wake and radio behavior.
- Actual DAC/USB/Bluetooth sound output and quality, gapless transitions, audio
  underruns while downloading, native DSD and every decoder/DSP path.
- All 128 original feature rows, paid streaming authentication, firmware updating,
  hardware buttons, screen rotation/accessibility and multilingual new UI strings.
- Long listening sessions, large libraries, repeated network/power failures,
  download throughput distribution, scrolling FPS, navigation/search percentiles,
  artwork timing and repeated cancellation/hotplug stress.
- Full application sanitization: sanitizer coverage currently targets the new
  provider/queue and HTTP fixture binaries, not every upstream GUI/audio object.
- SpotBye dynamic behavior: static REA evidence is documented; no sandboxed
  executable/network trace was collected.

Simulator logs include unavailable ALSA `hw:0` and device sysfs warnings, expected
in this host environment. No claim of successful physical audio output is made.
No flash/recovery procedure has been tested. New-board firmware packaging remains
blocked as detailed in [hardware-and-packaging.md](hardware-and-packaging.md).
