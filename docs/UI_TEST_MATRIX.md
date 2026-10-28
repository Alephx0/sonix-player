# UI acceptance and evidence

Run native host build with `tools/build_wsl.py`; run the actual SDL executable
on a private 480 × 800 Xvfb display using `tools/test_ui_collections.py`.
Inspect its PNGs before accepting a change. JSON probes observe live LVGL bounds
and playback state; xdotool supplies real mouse/touch-compatible input. Do not
substitute browser renders. LVGL version is read from the checked-out headers.

| Journey | Acceptance | Automation |
| --- | --- | --- |
| Album / playlist | Same cover/title/action geometry, scrolling rows and footer | UI collection harness |
| Portrait / playback preview | Liked Songs and All songs open via cover; track/Play/Shuffle stay collapsed; explicit mini-player expands | `test_ui_collections.py --journey preview-online`, full collection journey |
| Online details | Metadata sheet does not download or interrupt playback; explicit queue navigation | `--journey preview-online` with parsed offline provider fixture |
| Online transport | Link validation, transfer integrity, pause/resume/import/decode | `tests/discovery/run.sh`, `test_live_download.py --fresh` |
| Empty / long / large playlist | No overlap, useful empty action, bounded widget count | UI collection harness |
| Playlist menu / editing | Add, remove, reorder, rename persist without file deletion | UI harness + `test_collection_library.py` |
| Artist selection | Individual selectable credits, bounded text | `test_collection_library.py`, rendered artist evidence |
| Discography | Correct owner, featured, type and chronology | `test_discography.py` |
| Queue | Priority before context; shuffle/repeat preserve invariants; restart | `test_priority_queue.py` |
| Offline library / lyrics | Membership, saved state, lyric search and timestamping | `test_offline_library.py` |
| Home | Local ranking, pins, import dates, resume and conditional shelves | `test_home_feed.py` |
| Import | Incremental additions and changed metadata | `test_library_import.py` |
| Sheets | Upward entry, close/scrim/handle dismissal, nested continuity | UI harness and targeted captures |
| Now Playing | Controls, expanded art, lyrics, artist/album navigation | Targeted simulator interaction; full automation still incremental |
| Library | Filters, sorting, list/grid, pins, folders, persistence | Model suite + targeted simulator interaction |
| Search | Incremental results, thumbnails and correct destinations | Model suite + targeted simulator interaction |
| Hardware | RAM, frame timing, standby, physical buttons, DAC | Pending connected-device validation |

Use synthetic/local licensed fixtures only. Include collaboration, release types,
embedded/missing art, long metadata, synchronized lyrics, empty/large playlists.
Snapshots taken after transitions settle; paused deterministic fixtures avoid
progress drift. Candidate screenshots are NOT approved baselines. An optional
baseline directory can compare normalized captures after explicit baseline
selection; never silently overwrite it. Functional assertions remain required.

Performance reports record host RSS and idle CPU over a stated interval and
visible/total widget counts. These are regression signals, not device budgets.
Each completed run writes its actions, assertions, screenshots and metrics to an
artifact report. See `UI_DECISIONS.md` for current evidence and remaining gaps.


Online extension journeys: `--journey online-release` verifies shared cover geometry,
scrolling, sheet dismissal, playback preservation, local-first results during an
artificially delayed real provider request, editing during that request, and stale
response rejection on Local. The delay is host-only and does not replace responses.
`--journey online-live` uses real Archive HTTPS and native UI taps to download,
automatically index, play locally and disable discovery without interrupting audio.
`test_live_download.py --fresh` additionally denies new network connections with
host seccomp before importing/decoding the completed file. This is not an R1 Wi-Fi
or DAC test. Provider tests cover atomic release capacity and stable worker slots.
