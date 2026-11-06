# HiBy R1: Spotify-inspired product engineering

## Mission and authority

Build a cohesive local music player that looks, feels, and behaves as close to
Spotify Mobile as practical on the HiBy R1. Improve both interaction design and
implementation quality. A dark theme and green buttons alone are not fidelity.
The deliverable is the real C/LVGL application in `sonix-player`, not a web mockup.

Latest explicit user requirements take precedence over these documents, research,
historical screenshots, and old implementation notes. Read, in order:

1. `docs/UI_SPEC.md` - product requirements and deliberate offline adaptations.
2. `docs/DESIGN_SYSTEM.md` - shared visual and interaction language.
3. `docs/UI_RESEARCH.md` - mandatory internet research and reference records.
4. `docs/UI_AUDIT.md` - whole-product inspection, gaps, and handoff records.
5. `docs/UI_TEST_MATRIX.md` - verification procedures and acceptance gates.
6. `docs/UI_DECISIONS.md` - dated decisions, corrections, and historical evidence.

Study the current source, navigation, playback/data ownership, build scripts,
fixtures, and relevant tests before changing behavior. Inspect git status and
preserve unrelated work. Read older notes as history, not a mandate to restore
retired features. The active product is local-only; do not reintroduce SpotBye,
downloaders, provider matching, or verification flows.

## Scrutinize the existing product

Do not assume any implementation, comment, test, screenshot, or previous claim
of completion is correct. Trace each reachable control through its destination,
state mutation, persistence, and return path. Reproduce the actual user journey.
A passing test may encode the wrong requirement; assess it against user intent
and researched behavior before relying on it. Never weaken a valid test just to
make the current implementation pass.

Audit the whole reachable product, including secondary menus, keyboards, settings,
audiobooks, scan/import flows, dialogs, empty/error/loading states, and hardware
utility pages. Existing completed screens may be corrected when evidence shows a
defect or inconsistency. Prioritize coherent journeys and shared-component fixes;
avoid arbitrary restyling or unrelated rewrites. Record gaps rather than silently
excluding difficult screens or calling the product complete after a happy path.

## Research Spotify Mobile automatically

For EVERY feature audited or implemented, proactively browse the internet for its
mobile appearance AND behavior before choosing its interaction contract. Do not
wait for the user to request research. Follow `docs/UI_RESEARCH.md`: prefer Spotify
support/product sources, use dated mobile visual evidence, distinguish Android/iOS,
versions and experiments, and record direct URLs, access dates, observations,
confidence, and the R1 adaptation. Research may be shared across related components
when its applicability is explicit. Memory, a desktop screenshot, and generated
mockups are not sufficient evidence. If a source is unavailable, try alternatives,
mark the uncertainty, and continue independent work; never invent verification.

## Every feature belongs to the same interface

Every user-facing feature retained in this product MUST use the Spotify-inspired
UI and navigation system. A styled tab that routes to an unchanged native Sonix
screen is unfinished work, not an integration. Audit nested destinations as well
as first-level screens. Reuse Sonix's backend, database, decoders, hardware APIs,
and playback services; replace or adapt their presentation and interaction layer.
For R1-only functions with no Spotify equivalent, use the same visual language,
controls, sheets, hierarchy, and navigation, and document the adaptation.
Legacy layouts may exist internally, but are not acceptable fallbacks reachable
from the Spotify-style experience. Do not silently remove a feature to hide a gap.

## R1 engineering constraints

Use the repository's R1 target and 480 x 800 SDL renders; budget for 64 MB RAM.
Verify hardware specifications against official HiBy sources and checked-in build
configuration before making new hardware-dependent assumptions. Exact connected
revision/firmware and device performance remain unverified until measured.
Use bounded rows, paginated metadata queries, asynchronous artwork loading and
bounded caches. Keep network, scanning, decoding, and expensive work off the UI
thread. Avoid real-time blur, desktop runtimes, and unbounded per-track widgets.
Preserve responsive audio, browsing state, queue semantics, and physical controls.
Collection changes must never delete audio files. No hardware flashing without
explicit authorization.

## Execution and completion

Work autonomously through Study -> Research -> Audit -> Implement -> Build -> Render ->
Interact -> Inspect -> Fix -> Retest. Do not stop at a plan or cosmetic screenshot.
Reuse and improve shared components: albums/playlists share their collection page,
and features share the mini-player, bottom sheets, keyboard, and navigation.
After three failed corrections, investigate the underlying component or test
assumption instead of applying further arbitrary styling changes.

Use `tools/test_ui_collections.py` for repeatable real 480 x 800 LVGL evidence and
the relevant native model suites in the matrix. Inspect rendered pixels AND live
bounds/state; compilation alone is not verification. Test the journey while audio
plays, across repeated entry/exit, and across restart where persistence matters.
Extend meaningful tests for newly discovered failure modes. Candidate captures
are not approved baselines. Host memory/timing are not R1 measurements.

Update the research/audit records and affected product docs as work proceeds.
Handoffs must identify verified journeys, concrete remaining gaps, source/evidence
paths, and hardware-only uncertainties. Claim completion only against the written
acceptance gates, never from a percentage estimate or passing-test count alone.
