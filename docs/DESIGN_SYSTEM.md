# Native visual design system

Canvas: 480 × 800. Dark background #121212, sheet #202124, raised surface #282828,
primary ink white, secondary #b3b3b3, active green #1ed760. Artwork may tint the
collection and Now Playing background. Avoid blur and bright empty lyric panels.
New shared geometry belongs in `src/gui/shell/ui_metrics.h`.

Use existing `font_ui_*` assets: 28 collection title, 24 bold sheet heading,
20 track title, 18 artist/action, 16 secondary. Set explicit line bounds and
ellipsis; text height must accommodate the font's line height. Never squeeze
several artist credits into a single tappable row.

Collection artwork is 220 square; primary play is 64 square with a centered 28
glyph. Collection action band is 68 high. Track rows are 76 high in the reference
layout. Action hit areas should be at least 44 square. Preserve bottom clearance
for the shared mini-player and navigation. Titles scroll with the collection;
the fixed heading appears only after the expanded title scrolls out.

## Existing component ownership

| Element | Implementation to reuse |
| --- | --- |
| Album/playlist collection, windowed tracks | `gui/library/medialist.c` |
| Playlist song selection | `gui/library/playlistview.c` |
| Library filters, grid/list, folders, menus | `gui/library/collectionview.c` |
| Discography / individual artist navigation | `gui/library/discography.c`, player sheets |
| Home, persistent footer and mini-player | `gui/shell/main_menu.c` |
| Modal layer, scrim, dismissal | `gui/shell/bottomsheet.c` |
| Artwork loading and bounded shared slots | `gui/nowplaying/coverloader.c` |
| Transport and Now Playing | `gui/nowplaying/player.c` |

Paths above are relative to `sonix-player/src`. Prefer these components over new
parallel implementations. Legacy Sonix layouts remain supported; this specification
targets reference layout 3. Extract shared helpers incrementally when touched.

Bottom sheets animate Y and scrim opacity only, approximately 220 ms, eased.
One modal layer owns cleanup. Only the handle drags the sheet; scrollable content
owns vertical scrolling and queue handles own row dragging. Artwork decoding is
asynchronous. Collection rows reuse a fixed pool; do not allocate one widget per
track or preload all artwork.


Reference collection cover/title/artist/metadata/action widgets are now built by
`gui/library/collection_header.c`, used by medialist and the native online release
page. Online pages retain the shared footer; source details use the existing sheet.
The online placeholder deliberately uses no remote artwork buffer in this milestone.


Online settings retain the existing native column/button system. Three 46 px
category controls sit above independently scrollable settings; Save/Discard/Defaults
remain above the persistent mini-player. Values fit single 52 px rows. Queue
status filters use five equal touch targets. Do not port desktop-only controls
as inert switches or replace the local playback/navigation state.
