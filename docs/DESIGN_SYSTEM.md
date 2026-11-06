# Native visual design system

November 6 repair: Library fast-scroll geometry is resolved after layout and its
travel cannot be negative. Keep the grip within the list viewport, below filters
and above the persistent player. Alphabetical ordering and the letter bubble use
the same Unicode case/accent folding and leading-article convention.

The optional waveform is an R1 adaptation, not a Spotify visual-parity claim.
Its compact 32-layout-pixel seek envelope must preserve title -> progress ->
transport ordering. The artwork yields the additional height; transport and
utilities stay reachable at Normal and Large. Repeated style changes must not
reorder children or introduce scrolling inside the fixed controls block.

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
| Reference palette, accent ink | `gui/shell/theme.c` |
| Settings/utility pages, rows, switches, sliders, chips | `gui/shell/settingsrow.c` |
| Hub destination lists (Devices, Audiobooks, Browse music, More) | `gui/shell/gridpage.c` |
| Device menu (profile position) | `gui/shell/main_menu.c` |

Paths above are relative to `sonix-player/src`. Prefer these components over new
parallel implementations. This specification targets reference layout 3. Legacy
layouts may remain internally for compatibility, but routing from the Spotify-style
product into unchanged legacy presentation is a confirmed gap. Adapt every retained
destination, including settings and utilities, to this system. Reuse backend APIs
and improve shared UI components rather than cloning a new shell for each feature.

Bottom sheets animate Y and scrim opacity only, approximately 220 ms, eased.
One modal layer owns cleanup. Only the handle drags the sheet; scrollable content
owns vertical scrolling and queue handles own row dragging. Artwork decoding is
asynchronous. Collection rows reuse a fixed pool; do not allocate one widget per
track or preload all artwork.


Reference collection cover/title/artist/metadata/action widgets are built by
`gui/library/collection_header.c`. Local search uses 58 px embedded thumbnails,
a bounded result pool and horizontally scrolling category chips. Artist results
reuse release artwork; there is no external portrait service.

Search text stays inside its viewport between the search and clear icons. Empty
results are quiet text, with no provider errors or unavailable online actions.

Home places the Audiobooks category chip immediately after Music, using the
existing rounded category controls (132 px wide, 42 px high at 480 x 800). Its
selected state is green. Reuse Home's cover cards and horizontal shelves: 156 px
art, bounded title/author, a muted saved-position line, and a green Play button.
All audiobooks uses a two-column grid with 12 books per page. Reuse the shared
bottom sheet for details and Resume. Keep Music customization on the music feed;
no additional main navigation tab or full-screen legacy browser handoff.

Collection search sits above the first track, initially hidden. After reveal,
keep its 48 px field and sorting control below the fixed heading, including
while the keyboard is visible. Use muted raised surfaces and the same green
accent for the keyboard action and enabled Shuffle. The fast-scroll rail has a
24 px touch area, a 6 px visible thumb, and stays above the mini-player. Missing
cover art uses a centered native-size glyph on the shared 220 px cover tile.
Reference headers explicitly retain white text after tint changes or missing art.

## Reference palette and settings kit (2026-11-02)

`theme.c` has a fixed reference palette used whenever `screen.player_layout_alt`
is 3: page #121212, raised surface #282828, pressed #3e3e3e, secondary #b3b3b3,
accent #1ed760 with black `accent_ink`, switch-off track #535353. Every
`theme_style_*` consumer (settings, hubs, dialogs, keyboard keys, quick panel)
inherits it. Text drawn on an accent fill uses `theme()->accent_ink`, never a
hard-coded white. The dark/light, accent and dynamic-tint preferences remain
stored for legacy layouts; Appearance hides those controls in the reference
layout because they would have no effect there.

`settingsrow.c` builds Spotify-style pages in the reference layout: a 24 px bold
title centred beside the shared back button, flat 72 px rows on the page
background (pressed #2a2a2a, radius 8), 20 px row names, 18 px secondary values,
22 px bold white section headings, 52 × 32 switches, 6 px slider tracks, and
40 px chips (48 px touch) that are green with black ink when selected. Pages and
gridpage hubs mount the shared mini-player and navigation and end their list
above them; the library/audiobook scan screens are excluded because their own
bottom buttons are the only exit. Gridpage hubs render as the same flat rows
with chevrons instead of picture tiles. Geometry is chosen when a page is built;
colours follow a later layout change immediately.

The R1 device menu sits where Spotify places the profile avatar: a 44 px #282828
circle with the gear glyph left of every tab title. It opens the shared bottom
sheet with Settings, Devices, Audiobooks, Browse music and More. It replaces the
legacy tools grid and the separate gear; there is no account, so it does not
pretend to be a profile.

## Cohesion across the whole product

Audit hierarchy, density, baseline alignment, contrast, cover treatment, icon family,
active/disabled states, touch areas, scrolling, motion and feedback across screens.
Use consistent semantics: the same icon/action must mean the same thing everywhere.
Keep navigation and playback state distinct; browsing or dismissing a sheet must
not silently start a track, reset a queue, or lose the user's place.

Every feature needs intentional populated, empty, loading, failure and recovery
states where applicable. Unsupported actions should not masquerade as functional
controls. Device-only settings use the same headers, rows, toggles, sliders,
keyboard, sheets and return behavior as the rest of the player. Do not transplant
phone status bars, tiny desktop controls, or online-only actions onto the R1.

Adapt mobile proportions to the physical R1 display and touch input; do not shrink
a phone screenshot mechanically. Use motion to explain transitions, with bounded
LVGL animations and reduced work during playback. Lightweight placeholders must
look intentional. Never hide clipping by reducing all text or overriding one
screen's colors without examining the shared component and its state transitions.

## Row swipe and sheet kit (2026-11-03)

- Row swipe (`shell/rowswipe.c`): content slides over an action strip; grey
  #2a2a2a until 96 px, then #1ed760 (queue, pin; black icon) or #e91429
  (remove; white icon); spring back 170 ms; claims at 8 px with 2:1
  horizontal dominance; the left 32 px belong to the swipe back.
- Sheets size to their content; the close control is the sheet's own (no Done
  in the header). Option rows: 56 px, grey 26 px icon, 20 px text, green check
  for the current choice; described rows (Create): 76 px, icon in a 52 px
  #3e3e3e circle, bold title, #b3b3b3 subtitle.
- Pills: 50 px tall, full radius; green primary (black text), white (sheet
  create action), outlined secondary (#7c7c7c border); pressed scale 96 %.
- Naming: centred 24 px bold question, 32 px centred text on a 2 px underline,
  default value as the hint, Cancel/Create pills above the keyboard.

## Cards and carousels (2026-11-04)

- Shortcut tile: 56 px tall, half the gutter width minus 4, #2a2a2a, radius 4,
  sleeve 56 px flush left, 18 px name in up to two lines; pressed #3a3a3a.
- Shelf card: 150 px sleeve (radius 4; circle for artists), 18 px name and
  16 px #b3b3b3 subtitle below; no overlay controls (long press = menu).
- Carousel: full screen width with the 20 px gutter as inner padding, 16 px
  between cards (12 px for tiles), momentum, snap to start; the next card
  always peeks at the right edge.
- Browse tile: radius 8, Spotify palette colour, 22 px bold title (two lines),
  78 px sleeve tilted 25 degrees at the bottom-right corner, 14 px shadow.
- Placeholder art: #333333 square with a #b3b3b3 glyph at 40 % size.
- Playing context: its card title is #1ed760.

## Profile drawer, personal pages and settings categories (2026-11-04)

- Avatar: 44 px disc #f573a0 with a bold black initial, beside every tab title.
- Drawer: 400 px #1f1f1f panel from the left over a black 60 % scrim; header
  56 px avatar, 24 px bold name, 16 px quiet "View device info"; 64 px rows
  with 28 px white outline icons at x 20 and 20 px labels at x 68; #3a3a3a 1 px
  dividers inset 20 px. Slide 220 ms in, 180 ms out; drag left to close.
- Personal pages use the settings scaffold (title, back, footer). Song rows:
  72 px, 56 px cover (radius 4, #282828 placeholder with a 40 % glyph), 20 px
  title, 16 px #b3b3b3 subtitle, optional 30 px rank column; swipe right to
  queue. Section headings 22 px bold with 18 px above.
- Stat tile: #1f1f1f, radius 8, 92 px tall, 36 px bold value, 16 px label.
- Settings category (`settingsrow_category`): superseded the same day by the
  iOS captures -- icon, name, summary and a chevron; see the section below.

## Pushed-page header, settings tiles and weekly stats (2026-11-04, from iOS captures)

- Pushed-page header: back chevron on a 56 px #1f1f1f disc (90%), the title
  bold 24 px centred on the screen; a right action uses the same disc
  (Settings: search). Now Playing's down chevron has no disc.
- Settings category row: 28 px outline icon at x 0, name 20 px and summary
  16 px #b3b3b3 at x 46, chevron at the right; 72 px.
- Quick tile: #242424, radius 8, 94 px, three across with 8 px gaps; 28 px
  icon, 16 px name and state centred; on = #1ed760 icon, name and state.
- Profile drawer: 432 px, #1a1a1a; profile card #242424 radius 12, inset
  12 px, 80 px; rows 60 px.
- Stats tile: #1f1f1f radius 8, 12 px padding, 16 px grey label with a small
  chevron, then the value (64 px bold number, or a 22 px bold name over
  artwork the tile's width, round for an artist). Two columns, 10 px apart,
  each as tall as its contents. Past-week card: 104 px, 64 px art, 20 px bold
  dates, 16 px grey song and plays, chevron.
- Recents group row: 84 px, 72 px cover, 20 px name, 16 px grey
  "N songs played · Kind", a 30 px outlined round chevron; its songs below,
  indented 24 px, folded until opened.

## Queue sheet (2026-11-04, from iOS captures)

- Header: "Queue" 24 px bold at the sheet's text edge; "Playing" #b3b3b3 and
  the context name white, 18 px, under it. Corner pills: #2a2a2a, 44 px,
  fully round, 18 px white text, 20 px side padding ("Clear", "Edit").
- Bottom bar: three tiles across, #2a2a2a, radius 8, 64 px, 26 px icon over a
  16 px name; on = #1ed760. Edit Queue swaps it for "Move up" and "Remove"
  pills, 40% while nothing is chosen.
- Selector: 26 px ring, 2 px #b3b3b3 border; chosen = filled #1ed760 with a
  black tick.
- Settings category icons and slider rows: icons in a 28 px box; slider track
  inset by half a knob (11 px) on each side.

## Search bar, filters, playlist pills, R1 additions (2026-11-04)

- Search bar: #ffffff, radius 8, 58 px; 26 px black magnifier; 20 px black
  text; placeholder #535353.
- Filter button: 42 px #2a2a2a disc with a 22 px sliders glyph, first in the
  chip row. Filter sheet chips: #2a2a2a, fully round, 42 px, 18 px "Name (n)";
  selected #1ed760 with black text. Active genre chip: #1ed760, black text and
  a 16 px close mark.
- Playlist pills: #2a2a2a, fully round, 44 px, 16 px side padding, 20 px icon
  and 18 px label, 8 px apart, scrolling sideways.
- Format badge: 28 px, radius 4, 1 px #b3b3b3 (70%) outline, 14 px #d0d0d0
  text, centred in the shortcut row.
- Sound sheet: 660 px bottom sheet, 20 px sides; switch rows; normalization
  pills 92 x 40, selected #1ed760.

## Now Playing (2026-11-05)

- Background: flat lv_color_darken(sleeve tone, 60%), #2a2a2a without a
  sleeve; never a gradient (RGB565 banding).
- Header: context name, 18 px white, centred on the chevron's centre line;
  horizontal "..." at the right.
- Lyric line: 20 px bold, white at 80%, one line, 10 px above the title row;
  only while synced lyrics are shown and the preview is on.
- Shortcut row: 42 px, 66 px from the bottom edge; player column padding
  74 px at the bottom.
- Home shelf card text: 16 px grey kind at +6, 20 px bold name at +26, 16 px
  grey creator at +52 under the sleeve; card CARD_W + 78, shelf CARD_W + 84.
