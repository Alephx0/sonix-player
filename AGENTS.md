# HiBy R1 UI engineering

Read `docs/UI_SPEC.md`, `docs/DESIGN_SYSTEM.md`, `docs/UI_TEST_MATRIX.md`, and
`docs/UI_DECISIONS.md` before UI changes. The latest explicit user requirement
overrides historical screenshots and older custom-firmware notes.

- Develop the real C/LVGL application in `sonix-player`, not a web mockup.
- Use 480 × 800 SDL renders. Compilation alone is not UI verification.
- Reuse existing components; albums and playlists share the collection page.
- Preserve playback and navigation when browsing or opening bottom sheets.
- Keep database relationships independent of files. Removing a collection must
  never delete audio. No hardware flashing without explicit authorization.
- Use bounded rows, metadata queries, and artwork caches for the 64 MB target.
- Run Understand → Implement → Build → Render → Interact → Inspect → Fix →
  Retest for the affected journey. After three failed corrections, investigate
  the underlying component or test assumption instead of arbitrary restyling.
- Record recurring corrections and evidence in the UI documents. Distinguish
  rendered/behaviorally verified work from unverified hardware behavior.
- Use `tools/test_ui_collections.py` for repeatable native UI evidence and the
  targeted native model tests listed in the matrix. New captures are candidates,
  not approved visual baselines. Do not redesign unrelated completed screens.
