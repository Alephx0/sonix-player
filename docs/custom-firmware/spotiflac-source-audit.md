# Supplied SpotiFLAC source: native integration follow-up

Reviewed 2026-10-28 at commit
`90f51a4da5bff0ec51183dd5e789a78e6f6a009d` in
[1111Windows/SpotiFLAC](https://github.com/1111Windows/SpotiFLAC/tree/90f51a4da5bff0ec51183dd5e789a78e6f6a009d).
The trailing period in the supplied repository URL was treated as punctuation.
The fork is accessible; its [LICENSE](https://github.com/1111Windows/SpotiFLAC/blob/90f51a4da5bff0ec51183dd5e789a78e6f6a009d/LICENSE)
is MIT, copyright 2026 afkarxyz. It is a source reference, not proof that the
running SpotBye 2.0.0 executable was built from this commit.

The [SpotBye gist](https://gist.github.com/afkarxyz/b2f7b815b1560d7a58d7dd847f073f00)
contains a Proton Drive download link and antivirus-disable advice. It contains
no application source or API contract. That advice was not followed, and no
replacement executable was downloaded or run. The exact supplied executable's
REA hash remains documented in [the runtime audit](spotbye-parity.md).

## Source findings

All paths below refer to the pinned fork, not Sonix files.

| Source | Established behavior |
| --- | --- |
| [spotify_metadata.go](https://github.com/1111Windows/SpotiFLAC/blob/90f51a4da5bff0ec51183dd5e789a78e6f6a009d/backend/spotify_metadata.go) | Separate track/album/artist/playlist search; at most 50 per request with offset. Metadata preserves catalog IDs, album/artist relationships, contributor arrays, duration, dates and preview/explicit fields. |
| [spotfetch.go](https://github.com/1111Windows/SpotiFLAC/blob/90f51a4da5bff0ec51183dd5e789a78e6f6a009d/backend/spotfetch.go) | Spotify web-client initialization, token/session handling and persisted GraphQL queries. This is not an official unauthenticated Spotify API contract. |
| [songlink.go](https://github.com/1111Windows/SpotiFLAC/blob/90f51a4da5bff0ec51183dd5e789a78e6f6a009d/backend/songlink.go) and [link_resolver.go](https://github.com/1111Windows/SpotiFLAC/blob/90f51a4da5bff0ec51183dd5e789a78e6f6a009d/backend/link_resolver.go) | ISRC lookup, Deezer public track metadata, and independently ordered Songstats/Songlink resolution. A service URL is not an audio transfer. |
| [community_session.go](https://github.com/1111Windows/SpotiFLAC/blob/90f51a4da5bff0ec51183dd5e789a78e6f6a009d/backend/community_session.go) and [community_endpoints.go](https://github.com/1111Windows/SpotiFLAC/blob/90f51a4da5bff0ec51183dd5e789a78e6f6a009d/backend/community_endpoints.go) | Persistent sessions, browser verification, grant exchange, signed requests and provider cooldowns. These flows have not been exercised or ported on the R1. |
| [tidal.go](https://github.com/1111Windows/SpotiFLAC/blob/90f51a4da5bff0ec51183dd5e789a78e6f6a009d/backend/tidal.go) and [amazon.go](https://github.com/1111Windows/SpotiFLAC/blob/90f51a4da5bff0ec51183dd5e789a78e6f6a009d/backend/amazon.go) | Provider-specific source resolution, quality/fallback, manifest handling and finalization; some workflows invoke FFmpeg. A single generic download URL adapter cannot replace them. |
| [queue_db.go](https://github.com/1111Windows/SpotiFLAC/blob/90f51a4da5bff0ec51183dd5e789a78e6f6a009d/backend/queue_db.go) | Transactional bbolt items/order, replacement and incremental upsert/removal; rejects missing and duplicate job IDs. Sonix retains its bounded native queue instead of adding Go/bbolt. |
| [filename.go](https://github.com/1111Windows/SpotiFLAC/blob/90f51a4da5bff0ec51183dd5e789a78e6f6a009d/backend/filename.go) | Template expansion, artist separators, sanitization and collision suffixes. Sonix's three presets remain a subset. |
| [download_validation.go](https://github.com/1111Windows/SpotiFLAC/blob/90f51a4da5bff0ec51183dd5e789a78e6f6a009d/backend/download_validation.go) | Downloaded recording duration is checked independently from transfer completion. Full cross-provider duration/version matching is still pending in Sonix. |

## Implemented native catalog path

`system/discovery/catalog.c` is an independently written C adapter for public
Deezer metadata. A live `api.deezer.com/search` request succeeded; the public
Songlink API probe returned HTTP 401. No token extraction, service signup,
community verification or provider audio transfer was performed.

Search -> Sources now offers saved Archive releases / Deezer tracks / Deezer
albums choices. Archive remains the default. Each online request captures an
immutable provider descriptor before dispatch; Local stays independently usable.
Deezer track and album links select their provider without changing the saved
preference. Exact host, path, numeric identifier and page checks prevent arbitrary
URL fetches; server-provided next URLs are not followed.

The adapter retains stable track/album/artist IDs, duration and validated ISRC.
Track details can open the album or artist release list. Albums reuse the native
collection header and track rows. Catalog rows and releases load existing source
art through Sonix's shared asynchronous image loader; only visible result covers
are retained, and the album keeps one bounded decoded cover. Missing artwork uses
the existing placeholder. No artist photographs are fetched.

Catalog entries are explicitly non-downloadable, and the native audio validator
rejects them independently of UI state. Public metadata, readable flags and
preview URLs do not become full-recording download sources. Existing Archive
transfers still use their original validation/import pipeline. There is no silent
search-by-title substitution from a Deezer recording to an Archive file.

Bounds: 24 results/page, at most 1,000 pages, 256 KiB response cap and a conservative
8,190 structural-token cap before JSON allocation. Native HTTPS uses the existing
certificate-verified stack. No desktop runtime or companion is needed for this
metadata path. The shared image loader's disk-cache retention is unchanged;
bounded decoded memory does not mean an automatically bounded lifetime disk cache.

## Remaining differences

This is not exact SpotBye compatibility. Spotify web metadata, playlist search,
full online discography categories/featured credits, complete contributor graphs,
global release-date ordering, cross-service source matching, preview playback and
Tidal/Qobuz/Amazon/Deezer full-audio downloads remain unfinished. Artist browsing
currently displays the provider's paged release list, with no inferred ownership
or featured categorization. Source licensing does not authenticate a binary or
establish an audio provider entitlement.

The native download settings and queue are retained; additional tag, folder,
conversion and source-priority controls should only be exposed when implemented.
R1 deployment, DAC/Wi-Fi behavior, memory/CPU and standby are unverified. No flash
was attempted.

## Verification evidence

- [Live native catalog journey](evidence/deezer-catalog-ui.json): 19 checks,
  including artwork, metadata/download separation, direct recording link,
  album geometry, artist release lookup, preference restart and local playback.
- [Local-first and Archive release regression](evidence/deezer-local-regression-ui.json):
  14 checks, including fast local hits during delayed networking and rejection
  of stale online responses after selecting Local.
- [Native provider, transfer, TLS and live catalog tests](evidence/deezer-catalog-model.txt):
  ASan/UBSan enabled; hostile URLs/IDs, response limits, invalid ISRC, source
  identity mismatch, paging and download rejection; real track/album/artist API
  calls. Existing download queue, integrity, restore and import tests also pass.
- Host and MIPS builds and glibc <=2.22 ABI verification pass. A pre-existing
  Archive filename format-truncation warning remains; no new catalog warnings.

Inspected native 480 x 800 captures:
[track results](screenshots/catalog-live-tracks.png),
[details](screenshots/catalog-track-details.png),
[album](screenshots/catalog-release.png),
[direct link](screenshots/catalog-direct-link.png),
[artist releases](screenshots/catalog-artist-releases.png),
[album search](screenshots/catalog-live-albums.png), and
[source selection](screenshots/catalog-settings.png).
These are candidate renders, not approved baselines. The catalog journey measured
39,624 KiB RSS and 4% of one host core over one second after its restart step;
this is neither peak catalog memory nor R1 hardware performance.

Test correction: the host probe originally described textareas as generic objects.
It now reports the native textarea class, so the long-link geometry assertion
checks the actual input viewport. Direct-link checks wait for the one-result
response instead of accepting a matching title left over from the previous search.
