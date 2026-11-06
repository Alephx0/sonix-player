# Local LVGL patches

The Makefile clones LVGL 9.6.0 into `lvgl/` and applies every `*.patch` here,
in name order, once (a patch that already applies in reverse is skipped).
A patch that applies neither way stops the build.

| patch | what it changes |
|---|---|
| `0001-refr-top-layer-cover.patch` | An area covered by an opaque object on the top layer is drawn from that object, as LVGL already does for screens. The player and every bottom sheet live on the top layer, so before this each frame of a scrolling sheet also drew the screen under the player, the whole player and the dimming veil, only to paint over them (device round 4: the "..." menu scrolled choppily). |
