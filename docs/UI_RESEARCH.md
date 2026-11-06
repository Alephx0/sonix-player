# Spotify Mobile research protocol

## 2026-11-06: Waveform, Library index, and count fallback

Accessed 2026-11-06. Mobile visual evidence: owner-supplied iOS captures documented
in `docs/records/spotify-ios-captures-2026-11-04.md` (Library filter chips, sort
control, artwork rows, Now Playing title/progress/transport ordering). Apply to
these shared components; this is not a new Android pixel reference.

- https://support.spotify.com/us/article/sort-and-filter/ — Mobile Library has
  category filters, alphabetical/creator sorting, pins and list/grid selection;
  preferences persist. High confidence. A draggable A-Z grip is an R1 adaptation;
  use the same case-folded key for the list and its letter indicator.
- https://support.spotify.com/us/article/now-playing/ — Mobile mini-player opens
  the larger player; transport, queue and save controls remain available. High
  confidence. No claim that Spotify uses an audio waveform: the optional waveform
  is the owner's R1 extension, and must preserve this established hierarchy.
- https://support.distrokid.com/hc/en-us/articles/360013534594-How-Streaming-Services-Categorize-Singles-EPs-and-Albums
  — Distributor documentation describes singles as 1-3 tracks, EPs generally as
  4-6 with duration qualifications, and albums as 7+. High confidence for the
  published general definition, not universal service-specific edge cases. The
  owner's explicit 7-track boundary takes precedence over duration exceptions
  for missing local tags. Mark estimates and retain manual/tagged overrides.

## 2026-11-06: Navigation latency and discography recovery

Accessed 2026-11-06. Visual layout references remain the owner's iOS captures
recorded in `docs/records/spotify-ios-captures-2026-11-04.md` (Home category chips,
artwork shelves, Search field/tiles, and persistent navigation). No Android pixel
parity or smartphone frame-rate claim is implied.

- https://newsroom.spotify.com/2022-08-09/spotifys-new-home-feeds-make-discovering-your-new-favorites-easy/
  documents and illustrates Home feeds, recent favorites, and in-place playback;
  dated Android-first rollout with iOS following. Preserve these journeys while
  preparing offline recommendations asynchronously. High confidence for documented
  behavior; the source does not specify latency targets.
- https://support.spotify.com/us/article/search/ documents songs, albums, artists,
  playlists, genres, and lyrics lookup. Keep Search usable while browse artwork
  and categories load; local-only scope remains an explicit adaptation.
- https://developer.spotify.com/documentation/web-api/reference/get-an-artists-albums
  distinguishes release types and artist-relative album groups including appears_on.
  High confidence for the catalog data distinction, not a mobile screenshot.
  Map explicit local release types and owner/performer credits separately.
- Actual card inspection: all 2,384 indexed tracks have empty RELEASE_META.type.
  Consequently, strict tag-only filtering hid their releases from Albums. The
  deliberate offline fallback is Albums with "Type not tagged", plus a persistent
  local type override. Track count alone still does not prove Single/EP status.

On-device baseline reproduced a 1,513 ms Home build and a 1,766 ms GUI pass during
hidden Library preparation. This is measured R1 evidence, not a simulator estimate.
See `artifacts/perf-discography-20261106/` for subsequent verification.

## 2026-11-05: R1 repair follow-up references

Accessed 2026-11-05. These observations apply to the gesture, discography,
settings, artwork and small-screen repairs. Visual reference is also the user's
dated iOS capture record, `docs/records/spotify-ios-captures-2026-11-04.md`;
that record does not establish Android pixel parity.

| Primary source | Observation / confidence | R1 application |
|---|---|---|
| https://support.spotify.com/us/article/now-playing/ | Mobile mini-player above navigation opens the larger view on tap; dots expose options; Queue controls what plays next. High for behavior, no exact gesture dimensions. | Restrict expansion to the mini-player's actual bounds; independent scrollable sheets. Format badge and Sound are device additions. |
| https://support.spotify.com/us/article/sort-and-filter/ | Mobile library filtering, sorting, list/grid, swipe/hold pinning. High for behavior. | Retain those controls; add the fast-scroll grip for a long offline library. A vertical scroll must not pin a row. |
| https://newsroom.spotify.com/2021-04-29/listeners-can-explore-their-spotify-collections-faster-and-easier-with-a-new-your-library-2/ | Dated mobile library layout and organization. High for that release, not a current-version screenshot. | Support hierarchy/chips and visual scanning; current user captures take precedence. |
| https://support.spotify.com/us/article/equalizer/ | iOS/Android offer presets and manual adjustment. High; exact ten-band curves are not specified. | Genre presets plus saved custom curves; curves are Sonix choices, not claimed Spotify DSP values. |
| https://support.spotify.com/us/article/profile-picture/ | Mobile profile permits editing name and picture. High for behavior. | Local device identity shown consistently in Settings and the profile drawer; no account required. |
| https://support.tunecore.com/hc/en-us/articles/115006689928-What-is-the-difference-between-a-Single-an-EP-and-an-Album | Distributor describes release-level count AND duration rules. High for published rules, not evidence that a partial local collection can be classified from count. | Use metadata, explicit suffixes, otherwise Release. Do not omit the duration/completeness constraints to invent a single. |
| https://support.apple.com/en-us/108429 | Storage categories, media/books/photos/system and a segmented usage bar; article links device/storage imagery. High. | SD usage categories and separate internal-system summary. No storage cleanup or deletion actions added. |

R1-only adaptations: SD updater preflight/confirmation; waveform option (line
default); wider seek/brightness hit areas; preserved parent scroll position;
focused artwork overlay with X/outside dismissal. These follow the shared
visual system and explicit user requirements, not invented Spotify features.


This is a required workflow, not a claim that current screens already match
Spotify. Research accompanies every feature audit or implementation. The user
wants an offline R1 adaptation of the mobile experience, not the desktop UI or a
connection to Spotify's service.

## Gather evidence before choosing behavior

1. Define the journey and states: entry, action, state change, feedback, dismissal,
   return, persistence, empty/loading/error behavior, and playback implications.
2. Browse current Spotify support and product sources for the relevant behavior.
   Useful starting points are https://support.spotify.com/ and
   https://newsroom.spotify.com/; these are discovery links, not feature evidence.
3. Find mobile screenshots or video of the same journey. Prefer official material;
   when necessary use dated, attributable app demonstrations or community reports.
   A community complaint can reveal a regression, not establish universal behavior.
4. Identify platform, app version/date, account tier, region, and experiment status
   where known. Mark unknowns. Do not merge contradictory variants into an invented
   specification. Choose and record a coherent reference for each flow.
5. Compare appearance AND function with the actual running LVGL implementation.
   Translate layout and gestures to the R1's display, physical buttons and memory.
6. Record what to keep, change, adapt or omit and why. Explicit user requirements
   override live Spotify behavior (for example, passive collection Shuffle,
   offline-only search, and discography-only artist pages).

Use search tools and accessible pages automatically. Read source contents; search
snippets alone are not verification. Follow technical questions with primary
library/platform documentation and verify hardware claims with official HiBy
material and the target configuration. Recheck references when they conflict or
when a later task depends on potentially changed behavior. Reuse a prior record
only after checking its date, scope, platform and continued applicability.

If browsing is unavailable or evidence is inconclusive, record the failed attempt
and precise uncertainty. Implement only justified local adaptations, label them
as provisional, and do not claim Spotify parity for the unresolved behavior.
No research finding authorizes restoring retired online acquisition services.

## Feature record template

Append records below, or link a dedicated research file for a substantial journey.
Do not mark a record researched until its sources have actually been read.

- Feature/journey and audit ID:
- Research date; mobile platform/version and other known context:
- Direct source URLs, source title, publication/update date if known, access date:
- Observed appearance: hierarchy, spacing, typography, imagery, controls, states:
- Observed behavior: triggers, mutations, ordering, navigation, feedback, recovery:
- Evidence quality: documented / visually observed / community report / inferred:
- Conflicting variants, missing evidence, confidence and chosen reference:
- Explicit user requirements that differ from Spotify:
- R1 adaptation: offline data, geometry, input, bounded resources, performance:
- Current implementation and confirmed discrepancy:
- Acceptance criteria and linked code, tests, captures, and final decision:

## Records

No feature research is asserted by this document's creation. Existing captures and
implementation notes must be reassessed through this protocol. Historical links
and claims in conversation or older documents are leads, not verified references.

- 2026-11-02: [whole-product research records](records/spotify-research-2026-11-02.md)
  (97 feature records across shell, settings, music library, audio, connectivity,
  audiobooks, extras and core). Each source is marked read or snippet; inferred and
  unavailable records are provisional. Known gaps: Home Play semantics, music sleep
  timer, scan/empty-card onboarding, appearance/theme options, the R1 keyboard and
  destructive collection actions (see the critic section of the route inventory).
- 2026-11-04: [feature gap research](records/feature-gap-research-2026-11-04.md)
- 2026-11-04: [Spotify iOS captures](records/spotify-ios-captures-2026-11-04.md) (user-supplied, first-hand)
  (Spotify features to adopt offline and R1-only additions; candidate list, each
  item still needs its own feature record before implementation).
- 2026-11-05: Display size (search results only, accessed 2026-11-05; snippets, not
  full reads). Spotify Mobile has no in-app text or display size control and
  follows the phone's own setting, partially: iOS Dynamic Type "to a certain
  extent" (newsroom.spotify.com/tag/text-resizing), a staff-linked community
  reply that there is no in-app font size (community.spotify.com, iOS
  "Accessibility: New Text Resizing options" thread), and Android reports that
  the system font scale is not honoured (community.spotify.com, "Android
  Spotify does not honor font scale setting"). Confidence: medium. R1
  adaptation: the R1 has no system setting, so Display size lives in Settings >
  Appearance and scales the whole interface (Android's Display size, not only
  text); Large is the default because the panel is 3 inches, 480x800 (HiBy
  guide guide.hiby.com/en/docs/products/audio_player; retailers).

## 2026-11-05: Touch calibration (hardware plumbing)

No Spotify interaction contract changes in this repair. Primary technical
reference, accessed 2026-11-05:
https://docs.kernel.org/input/multi-touch-protocol.html
Linux documents ABS_MT_POSITION_X/Y as surface coordinates and MT as a distinct
input protocol. Local LVGL 9.6 evdev source reads these events but only probes
legacy ABS_X/Y calibration ranges. Confidence high for this code-path defect;
its responsibility for the owner's device symptom is pending live diagnostics.
R1 adaptation: use controller-reported ranges for the logical display and retain
them across 180-degree rotation; do not assume raw range equals framebuffer size.


## 2026-11-05: Device round 4 records

- Singles, EPs and albums (searched 2026-11-05; distributor help pages, not
  Spotify's own FAQ, which the search did not return): TuneCore
  (support.tunecore.com/hc/en-us/articles/115006689928), CD Baby
  (support.cdbaby.com/hc/en-us/articles/360008275672), AWAL
  (help.awal.com/hc/en-us/articles/7882058990867) and Amuse
  (support.amuse.io/en/articles/107733-singles-eps-and-albums) give 1-3 tracks
  under 30 minutes a single, 4-6 under 30 minutes an EP, otherwise an album;
  Amuse puts the line at 5/6 tracks. Spotify shows singles and EPs together
  under "Singles and EPs" on an artist page. Confidence: medium. R1
  adaptation: an explicit RELEASETYPE tag wins; otherwise the track count
  (distinct titles), as the index keeps no lengths.
- Equaliser presets (searched 2026-11-05): no source confirmed Spotify's full
  preset list, which differs between Android and iOS and by version
  (headphonesty.com/2022/03/best-spotify-equalizer-settings/ names Bass
  Booster and Bass Reducer on iOS). Confidence: low for the list. R1
  adaptation: the presets the owner asked for (Pop, Rock, R&B, Acoustic) and
  the usual music-player set, as chips; values are approximations, not copied
  from any Spotify source.
- Not researched again this round (records above apply): the mini-player as
  the only way up to Now Playing; Now Playing's top bar; the queue's "Next
  from" heading. Fast-scroll on Your Library, the focused cover, the device's
  picture and name, and Storage are R1 adaptations with no verified Spotify
  equivalent; they use the shared sheet, card and chip styles.
