# Offline Home and vertical player

Home now uses a compact two-column shortcut grid and horizontal artwork shelves.
Cards open collections without starting audio; green Play controls start them.
The mini-player and Home/Search/Your Library navigation stay visible.

## Feed selection

- Quick access shows up to six unique collections: existing pins first, then
  recently played/frequently opened albums and playlists, with Liked Songs,
  imported albums and user playlists filling a new library.
- Jump back in offers saved album/playlist sessions. Its Play control restores
  the saved song and position; tapping the artwork browses the collection.
- Recently added combines albums and playlists using library arrival/creation
  time. New tracks retain their first indexed timestamp through metadata edits.
  Older databases use their existing arrival timestamp when migrating.
- On repeat contains songs with at least two qualified local plays in the last
  30 calendar days. It is hidden until there are matches; old lifetime counts
  are not presented as recent listening.
- Made for you creates up to four genre mixes from indexed metadata. Rediscover
  selects unplayed tracks or tracks not played in 90 days. Both use references
  to existing audio, without duplicating files or creating saved playlists.
- All, Music and Podcasts & Shows filter Home in place. The podcast filter opens
  followed shows through the existing podcast browser. A separate unfinished
  audiobook/podcast shelf is not included in this update.

Collection menus provide Play/Resume, Add to queue, Pin/Unpin and Go to artist
where applicable. Pins share Your Library's four-slot limit. Adding a collection
to the priority queue appends up to 64 tracks, bounded by remaining queue space.
The Home settings icon opens section visibility/order controls; changes persist.

The feed rebuilds when entering Home. Completed imports can refresh an idle feed
at its top; an active user or scrolled shelf keeps its order until returning.
History changes do not continuously reorder the feed. Rendering is bounded to
36 cards and eight decoded artwork slots, requested only for visible thumbnails.

## Now Playing

The player opens vertically from below the display and dismisses downward.
Its title-adjacent plus/check is the only playlist membership control. Lyrics
sits below transport on the left; queue sits opposite on the right. Transport
glyphs and filter labels are centered within their controls.

The inline lyrics card is dark and appears only when synchronized lyrics are
available and previews are enabled. Missing or unsynchronized lyrics do not
produce an empty preview. The Lyrics icon still opens full available lyrics,
or a small unavailable message. Queue and other contextual panels retain the
shared sliding overlay and do not interrupt playback.

## Validation

Host and MIPS builds and the glibc ABI check pass. `test_home_feed.py` covers
cold-start results, genre mixes, rolling-window expiration, access ranking,
saved position after reopening the database, and import time independent of
release year or filesystem timestamp changes. Queue, offline-library and
discography regression suites also pass.

Native 480 x 800 simulator checks cover Home shelves, saved-position resume,
section hide/reorder, vertical dismissal, the centered player controls and
lyrics present/absent states. Evidence includes:

- [Home dashboard](screenshots/home-dashboard.png) and [smart shelves](screenshots/home-mixes.png)
- [Opening upward, mid-animation](screenshots/home-opening.png)
- [Resume with synchronized lyrics](screenshots/home-resume.png)
- [Player without lyrics](screenshots/home-no-lyrics.png) and [unavailable message](screenshots/home-lyrics-unavailable.png)
- [Customization](screenshots/home-customize.png) and [changed section order](screenshots/home-reordered.png)
- Regression logs in `evidence/home-*.log` and binary hashes in `evidence/home-builds.json`

This is a simulator/native executable update. Physical R1 acceptance and the
existing latest-board firmware packaging limitations remain unresolved.
