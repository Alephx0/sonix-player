# Reference application investigation

Investigation date: 2026-10-27. This report separates observed artifacts, source
behavior, and unverified runtime behavior. No reference executable was launched.
No credentials, authentication stores, protected audio, or DRM keys were extracted.

## Reproducible inputs

| Input | Identity observed |
|---|---|
| SpotBye.exe, supplied temporary path | SHA-256 `fd46c09b4797b2ca7c645819a8b9e44220390a89efb8df98beeb9f536f3dd9e1`; 33,954,816 bytes |
| Mornye source | `c90ec6478ad3e808ef22ba9464f8ad05054dde7b` |
| Mornye-Desktop | `43b31adfa03eaac6ba180257042865bf75a3f1af` |
| REA documentation/source checkout | `baec375c347f129cb109823cff31831e906039f0` (package.json says 6.8.0) |
| REA actually installed/invoked | official npm `rea-agents@6.7.0`; MCP server reports 6.7.0 |
| Analysis host | Windows, Python 3.13, Node v24.13.1 |

Clones live in locally ignored `research/`, not in the firmware or distributable
source tree. Repository URLs:
[REA](https://github.com/morluto/rea),
[Mornye source](https://github.com/mornyeapp/Mornye),
[Mornye Desktop](https://github.com/mornyeapp/Mornye-Desktop).

## REA setup and actual MCP use

Read the official README and `docs/installation.md`. Ran:

```powershell
npx.cmd --yes rea-agents setup --client codex --skill=false --dry-run --json
npx.cmd --yes rea-agents@6.7.0 setup --client codex --skill=false --yes --json
```

The scoped setup reported `ready`, installed the REA MCP registration in the
existing Codex configuration, and preserved a configuration backup. The bundled
skill was omitted intentionally; the user requested the analysis tools and the
documented CLI supports this option. No Hopper, IDA, or Ghidra engine is configured.
The latest source documentation requires a newer Node patch than this host; the
installed npm version nevertheless successfully completed setup and the calls
below. These results establish the exercised tools, not all REA capabilities.

To use the MCP server immediately without restarting this agent, the reproducible
client `tools/reference-audit/rea_probe.py` launches only the official REA server, performs
`initialize`, `notifications/initialized`, `tools/list`, and `tools/call`.
`inspect_go_binary` actually inspected the supplied executable and returned
evidence. The script handles paginated tool discovery. `inspect_desktop_application`
is described by the newer checkout but was not advertised by the installed
server, so it was not claimed as used. No native disassembly provider was used.

```powershell
python tools/reference-audit/rea_probe.py 'C:\path\to\SpotBye.exe' docs/custom-firmware/evidence
python tools/reference-audit/spotbye_static.py 'C:\path\to\SpotBye.exe' docs/custom-firmware/evidence/spotbye-pe.json
```

Evidence includes [server handshake](evidence/rea-initialize.json),
[selected tool schema](evidence/rea-selected-tools.json),
[setup result](evidence/rea-setup.json), and the full
[Go metadata response](evidence/inspect_go_binary.json). This is MCP evidence,
not a claim inferred from installing a package.

## SpotBye identity and safety screening

The supplied file exists. Authenticode returned **NotSigned**. PE version resources
claim ProductName/CompanyName `SpotBye`, FileVersion `2.0.0`. These unauthenticated
fields do not establish the publisher or release provenance.

The independent PE parser found machine `0x8664` (AMD64), PE32+, and a static import
descriptor for `kernel32.dll`. This is a direct import inventory, not a complete
dynamic DLL dependency inventory. REA found the Go build-information header at
file offset 32,097,792 and decoded:

- Compiler identity `go1.27.1`, `GOOS=windows`, `GOARCH=amd64`, `CGO_ENABLED=1`.
- Main module `github.com/afkarxyz/spotbye`, version `(devel)`.
- Wails `v2.15.0`, WebView2 bridge `v1.0.23`, and build tags
  `desktop,wv2runtime.download,production`.
- Metadata libraries: `bogem/id3v2/v2`, `go-flac/flacpicture`,
  `go-flac/flacvorbis`, `go-flac/go-flac`, `go.senan.xyz/taglib`.
- Other declared dependencies include bbolt, MP4 tooling, and wazero.

This is a **Go/Wails Windows application using WebView2**, not evidence of an
Electron, .NET, Python-packaged, or MIPS application. Wails bindings are also
visible as embedded `main.App.*` strings. The module path was tested as a public
Git repository and returned “Repository not found”; source/build equivalence
and an authentic release checksum therefore remain unverified.

Microsoft Defender reported antivirus and real-time protection enabled. A custom
scan of this exact executable completed successfully; querying detections found
no matching SpotBye resource. [Screening record](evidence/spotbye-screening.json).
This is not a clean bill of health. The binary remains unexecuted because no
isolated analysis VM was established. No network trace or dynamic behavior is
asserted. A Windows sandbox/VM with snapshots, no account credentials, and
controlled networking is still needed for dynamic work.

### SpotBye workflow evidence and feasibility

The [PE/symbol inventory](evidence/spotbye-pe.json) records file offsets for 908
candidate identifiers. Its extraction deliberately restricts outputs to symbol
identifiers rather than dumping arbitrary strings or token-like data. Names show
candidate interfaces, not proof that a feature works or which endpoint it uses.

| Area | Static evidence | Conclusion / R1 adaptation |
|---|---|---|
| Search | `SearchSpotify`, `SearchSpotifyByType`, `SearchDirectLink`, `SearchDirectLinkByType` | Distinct catalog/direct-link search bindings exist. Suggestions, ranking, request parameters and authorization remain unverified. Use independently documented native provider APIs. |
| Metadata | `GetSpotifyMetadata`, `GetDirectLinkMetadata`, `ReadFileMetadata` | Catalog enrichment/local metadata interfaces are present. Provider identity must remain separate from playable/downloadable source identity. |
| Queue | `SubmitManagedDownloads`, `PauseManagedDownloads`, `StopManagedDownloads`, `GetDownloadManagerProgress` | Managed asynchronous queue interface is visible; exact transitions/retry policy require tracing. Native bounded queue is suitable. |
| Persistence | `LoadPersistentDownloads`, `ReplacePersistentDownloads`, `ApplyPersistentDownloadChanges` | Durable queue bindings exist; do not infer persistence format solely from the bbolt dependency. |
| Artwork and lyrics | `DownloadCover`, `DownloadLyrics`, `ReadEmbeddedLyrics`, `ExtractLyricsToLRC`, `SaveLyrics` | Separate enrichment stages are plausible and supported by bindings; format support and failure semantics unverified. |
| Naming and tags | `RenameFilesByMetadata`, ID3 and FLAC dependencies | Reimplement safe filenames and bounded tag handling; existing file clobber policy is unknown. |
| Quality | Provider-specific normalization/quality-label symbols | A label is not proof of actual codec or losslessness. Preserve source bytes and derive technical information from the audio. |
| Duplicate/error/auth behavior | No dynamic evidence | Do not claim duplicate detection, interruption recovery, region handling or authenticated download rights as verified. |

Protected-media dependencies also appear in the build inventory. They were not
used, decompiled into bypass procedures, or adopted. Any commercial catalog
integration needs its documented authorization and media rights; a catalog ID
does not grant download permission. SpotBye code was not copied, and its license
has not been established. The appropriate deliverable is independent native
application logic for sources that explicitly permit the operation.

## Mornye source investigation

Mornye-Desktop contains a README and screenshots and directs development to the
main Mornye repository. The main repository contains Flutter/Dart UI and Rust
backend implementation. Source inspection is preferable to binary reverse
engineering here; no Mornye desktop executable was needed or run.

**License discrepancy:** the Desktop README refers to MIT, but the inspected main
repository `LICENSE` contains LGPLv3 and `rust_backend/Cargo.toml` declares
`LGPL-3.0-only`. Treat the pinned source as LGPL-3.0-only unless the maintainer
clarifies otherwise. No source/assets were copied into the player in this audit.
Independent implementation of generic architecture avoids importing the complete
runtime or assuming the README's license label is authoritative.

Source links below use the inspected immutable revision. Paths abbreviated in
the table are relative to `rust_backend/crates/extensions/src/` unless stated.

| Concern | Inspected source | Observed behavior / consequence |
|---|---|---|
| Repository discovery | [repository.rs](https://github.com/mornyeapp/Mornye/blob/c90ec6478ad3e808ef22ba9464f8ad05054dde7b/rust_backend/crates/extensions/src/repository.rs) | Accepts registry URL or GitHub owner/repository; resolves default branch and `registry.json`; requires HTTPS; owns cache locking and concurrent refresh state. |
| Package loading | [package.rs](https://github.com/mornyeapp/Mornye/blob/c90ec6478ad3e808ef22ba9464f8ad05054dde7b/rust_backend/crates/extensions/src/package.rs) | ZIP-based `.mornye-ext`, `.spotiflac-ext`, `.sflx`; root `manifest.json` and `index.js`; inspect before extraction; rejects symlinks and unsafe paths; limits entries/extracted bytes. Those desktop limits are too large for an R1 import pipeline. |
| Manifest and capabilities | [manifest.rs](https://github.com/mornyeapp/Mornye/blob/c90ec6478ad3e808ef22ba9464f8ad05054dde7b/rust_backend/crates/extensions/src/manifest.rs) | Distinguishes `metadata_provider`, `download_provider`, `lyrics_provider`; declares network/file/storage permissions, optional HTTP, settings/secrets, quality options, search behavior, matching and capability fields. A metadata extension is not automatically a downloader. |
| Runtime | [runtime.rs](https://github.com/mornyeapp/Mornye/blob/c90ec6478ad3e808ef22ba9464f8ad05054dde7b/rust_backend/crates/extensions/src/runtime.rs), extensions Cargo.toml | Rust `rquickjs` worker VM, cancellation and bounded input queue. Default per-runtime heap allowance is **64 MiB**, stack 512 KiB, timeout 30 s. This is a configured upper bound, not measured consumption, and does not fit an unmodified R1 design with only 64 MiB total RAM. |
| Lifecycle | [manager.rs](https://github.com/mornyeapp/Mornye/blob/c90ec6478ad3e808ef22ba9464f8ad05054dde7b/rust_backend/crates/extensions/src/manager.rs) | Native manager owns installed entries, enabled/error status, initialization, source and runtime lifetime, mutation locks and metadata revisions. |
| Priorities | [manager/providers.rs](https://github.com/mornyeapp/Mornye/blob/c90ec6478ad3e808ef22ba9464f8ad05054dde7b/rust_backend/crates/extensions/src/manager/providers.rs) | Separate metadata/download orders and optional fallback allowlist; normalization and deduplication; URL handler selection honors metadata priority. |
| Search/enrichment | [manager/metadata.rs](https://github.com/mornyeapp/Mornye/blob/c90ec6478ad3e808ef22ba9464f8ad05054dde7b/rust_backend/crates/extensions/src/manager/metadata.rs) | Validates enabled metadata provider, bounds search limit, calls `searchTracks`, truncates results. Availability inputs include ISRC, provider IDs, title/artist and duration. Enrichment can retain original metadata on an ordinary failure; cancellation is propagated. |
| Download invocation | [manager/downloads.rs](https://github.com/mornyeapp/Mornye/blob/c90ec6478ad3e808ef22ba9464f8ad05054dde7b/rust_backend/crates/extensions/src/manager/downloads.rs) | Request separates track ID, quality, output path, item ID and prepared context; checks status and preflight; reuses healthy worker runtimes only if generation/settings still match. Account-owned mode is explicitly distinct from fallback. Do not port shared-account mechanisms. |
| Scheduling | [download_scheduler.dart](https://github.com/mornyeapp/Mornye/blob/c90ec6478ad3e808ef22ba9464f8ad05054dde7b/lib/services/download_scheduler.dart) | Tracks active IDs, pauses dispatch, prevents concurrent attempts for the same ID; concurrency clamped 1–3; observes completion. R1 should begin with one worker and measurements. |
| Durable queue | [download_queue_persistence.dart](https://github.com/mornyeapp/Mornye/blob/c90ec6478ad3e808ef22ba9464f8ad05054dde7b/lib/services/download_queue_persistence.dart) | Database-backed snapshots, ordered writes, corrupt-row decode handling, persisted paused state, 350 ms write debounce. Failed writes retain prior durable state. |
| Format/finalization | [download_file_finalizer.dart](https://github.com/mornyeapp/Mornye/blob/c90ec6478ad3e808ef22ba9464f8ad05054dde7b/lib/services/download_file_finalizer.dart) | Probes ISO-BMFF codec before extension normalization and avoids renaming over an existing target. Contains optional conversion/decryption stages; those are outside this adaptation. |
| Metadata | [download_metadata_embedding.dart](https://github.com/mornyeapp/Mornye/blob/c90ec6478ad3e808ef22ba9464f8ad05054dde7b/lib/services/download_metadata_embedding.dart) | Separates cover, lyrics, tag edits and optional ReplayGain/FFmpeg; reuses fetched lyrics for sidecar writing; honors embedding settings and album-artist metadata. |
| Duplicate handling | [library_duplicates.dart](https://github.com/mornyeapp/Mornye/blob/c90ec6478ad3e808ef22ba9464f8ad05054dde7b/lib/services/library_duplicates.dart) | Groups normalized ISRC across local/download history, excludes unavailable network copies, deduplicates physical paths and orders quality information. ISRC alone identifies recording candidates, not identical file bytes. |

The [extension documentation](https://mornye.com/docs) also describes independent
metadata, download and lyrics extensions; manifests and one JavaScript entry
point; catalog object schemas; optional album, playlist and artist functions;
and explicit domain permissions. Its older package/runtime terminology should
not override the pinned Rust implementation. No arbitrary community extension
was downloaded, authenticated or executed. Provider availability and rights
remain provider-specific.

## Native adaptation decisions

1. Keep Sonix's audio and LVGL architecture. Neither Wails/WebView2 nor
   Flutter/QuickJS/FFmpeg reference stacks belong in the new on-device workflow.
2. Use a small versioned C provider contract with explicit capability bits for
   tracks, artists, albums, playlists, playback, lyrics and authorized download.
   Return unsupported distinctly from empty results, authentication failure and
   network errors. Cap results and field sizes before allocating/UI creation.
3. Treat metadata matching as a separate operation from source authorization.
   Prefer stable IDs, then normalized title/artist/duration with confidence; do
   not silently download a different recording because it has a similar name.
4. Support independently replaceable native providers for documented lightweight
   sources. Do not interpret a provider name or an unprotected URL as permission.
   Add only rights-scoped sources with visible license/source attribution.
5. Reuse Sonix HTTPS and storage primitives where suitable. One background
   transfer, bounded buffers, cancellation, `.part` files, integrity validation,
   same-directory finalization, and library refresh are appropriate equivalents
   of reference queue workflows. Never execute provider-supplied shell commands.
6. Cache cover thumbnails at render size, preserve supplied audio quality, and
   keep metadata/tag enrichment optional so a failure cannot destroy audio.
   Duplicate filenames must not overwrite unrelated files.
7. If a heavyweight provider is needed later, an explicitly configured HTTPS
   companion may expose normalized metadata and authorized media references.
   Companion authentication must not disable TLS verification or become a
   dependency of offline playback/basic native providers.

These are implementation recommendations informed by evidence; this report does
not claim that all these operations have already been implemented in this fork.
See the implementation feature matrix and test report for actual delivery state.

## Remaining investigation limits

- SpotBye publisher/release provenance, search ranking, API request/response
  contracts, duplicate policy, errors and download state transitions need an
  isolated VM/network trace or authenticated source access. Static names cannot
  establish those behaviors.
- No hardware, runtime memory, power or throughput measurements come from this
  reference audit. The 64 MiB Mornye VM setting is a source constant only.
- Mornye source review establishes architecture, not successful operation of
  third-party providers. No assertion is made that a streaming entitlement
  permits permanent downloads.
- Neither repository license notices nor source paths authenticate the supplied
  SpotBye file. Retain its exact hash if any later dynamic analysis is performed.
