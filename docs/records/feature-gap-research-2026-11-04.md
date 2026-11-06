# Feature gap research: Spotify features to adopt and R1-only additions

Research date 2026-11-04. Scope: (A) current Spotify Mobile features that the
local player lacks and that work offline; (B) features Spotify does not have
that suit a local DAP. "Read" means the page itself was fetched; "snippet" means
only search-result text was available. This is a candidate list, not an
implementation record: each item still needs its own feature record per
`UI_RESEARCH.md` before it is built.

## Sources

| Source | Date | Access | Used for |
|---|---|---|---|
| [9to5Mac: four music-focused features](https://9to5mac.com/2026/05/28/spotify-releases-four-music-focused-features-including-background-downloads-on-ios/) | 2026-05-28 | read | Mobile playlist folders, in-playlist bulk actions, queue multi-select, one-tap reshuffle |
| [Spotify newsroom: new user controls](https://newsroom.spotify.com/2025-09-05/new-user-controls-personalize-listening/) | 2025-09-05 | read | Smart Filters, Hide in this playlist, Snooze 30 days, Discover Weekly genre refresh, redesigned queue (shuffle/Smart Shuffle/repeat/sleep timer, recommendations after the queue ends), playlist cover/customization, playlists from Liked Songs by genre/mood |
| [Spotify newsroom: personalization tag](https://newsroom.spotify.com/tag/personalization), [BetaNews 2025-05-08](https://betanews.com/2025/05/08/spotify-reveals-new-tools-to-give-you-greater-control-over-your-music-discovery-journey-and-banish-earworms) | 2025-05 | snippet | Exclude from Taste Profile, Snooze test |
| [GSMArena: weekly listening stats](https://www.gsmarena.com/spotify_now_gives_you_weekly_listening_stats-news-70189.php), [Dataconomy](https://dataconomy.com/2025/11/07/spotify-rolls-out-weekly-listening-stats-for-60-countries/) | 2025-11-06/07 | snippet | Listening stats: profile picture → Listening stats; top artists/songs, shareable highlights; window reported as 4 weeks (Spotify) or 7 days (some outlets) |
| [TechCrunch: Mix transitions](https://techcrunch.com/2025/08/19/spotifys-latest-feature-lets-you-add-your-own-transitions-to-playlists) | 2025-08-19 | snippet | Per-playlist transitions (fade/rise/blend) with waveform/beat data |
| [TechCrunch: running mode](https://techcrunch.com/2026/07/30/spotify-adds-a-running-mode-to-its-app/) | 2026-07-30 | snippet | iOS Premium running mode (Fitness Hub) |
| [Spotify Community: Smart Shuffle](https://community.spotify.com/t5/Your-Library/How-does-Smart-Shuffle-actually-work/m-p/6226321/highlight/true), [Engadget: Daily Mix](https://www.engadget.com/2016-09-27-spotify-daily-mixes.html), [The Star: Daylist](https://thestar.com.my/tech/tech-news/2024/09/11/spotify039s-039daylist039-goes-global-what-is-it-and-should-you-use-it) | 2016-2025 | snippet | Smart Shuffle inserts recommendations; Daily Mix up to six genre mixes; Daylist changes by time of day |
| [Spotify Community Live Ideas (queue history, play counts, local files in Wrapped)](https://community.spotify.com/t5/Live-Ideas/How-many-times-have-you-listened-to-a-certain-song/idc-p/6771317/highlight/true) | various | snippet | Requested and missing in Spotify: per-track play counts, played-songs history, local files counted in stats |
| [HiBy R1 spec](https://guide.hiby.com/en/docs/products/audio_player/hiby_r1/spec) | n/a | snippet | X1600E, CS43131, 64 MB, BT 5.0, DSD256/PCM 384k, MSEB, HiByLink |
| [HiBy R1 firmware changelog](https://guide.hiby.com/en/docs/products/audio_player/hiby_r1/firmware) | 1.0-1.3 (2024-10 to 2025-01) | read | Stock firmware added a most-played list, EQ, sample-rate/bit-depth display, sleep countdown |
| [HiFi Oasis R1 review](https://www.hifioasis.com/reviews/hiby-r1-review/), [Head-Fi R1 thread](https://www.head-fi.org/threads/hiby-r1-85-cs43131-dap-released.974698/page-10) | 2025 | snippet (Head-Fi pages blocked by bot check) | Owner wishlist: Last.fm scrobbling, PEQ, add to queue on the fly, UI customization, scanning that does not block the OS; 50,000-track index limit on stock firmware |
| Rockbox manual, [Working with playlists](https://git.lenord.me/devs/foxbox/raw/commit/2824bd5f1644a6b9129a0c0fcfe2bafab91a7225/manual/working_with_playlists/main.tex) (fork mirror) | n/a | read (official download.rockbox.org blocked) | Insert next/last/shuffled, queue, save current playlist, reshuffle, resume |
| [Wikipedia: Rockbox](https://wikipedia.com/wiki/RockBox) | n/a | snippet | Gapless, crossfade, crossfeed, ReplayGain, PEQ, pitch-corrected speed, tag database |

Unavailable: Spotify's official Playback-settings support article was not
returned by search (third-party guides only list Crossfade 1-12 s, Automix,
Volume normalization Quiet/Normal/Loud, Equalizer, Mono under Settings →
Playback; medium confidence). Head-Fi and download.rockbox.org refused automated
access.

## Already present (excluded from the gap list)

Verified in source on 2026-11-04: reshuffle and queue bulk edit
(`trackmenu.c`), sleep timer, playlist folders, crossfade/gapless, ReplayGain
(track/album/track-when-shuffled), EQ/PEQ/MSEB, crossfeed, balance/mono, volume
limit, DAC filters, DSD gain, USB DAC, Bluetooth receiver, AirPods, Last.fm with
an offline play queue on the card (`lastfm.c`), CUE sheets, waveform, playback
speed, audiobook bookmarks, "Track information / File details" (`player.c`),
pause on output unplug (`player_output_unplugged`), On repeat / Rediscover /
mixes on Home, Most played tile.

## A. Spotify features that are missing and work offline

| Priority | Feature | Spotify reference | Offline/R1 adaptation | Cost |
|---|---|---|---|---|
| 1 | Recents (listening history) | Recents in the profile menu; queue-history idea in Community | Time-stamped play log in SQLite (bounded, e.g. 2,000 rows); a Spotify-style list grouped by day, tap to play in its original context | Low |
| 1 | Listening stats | Profile → Listening stats (Nov 2025) | Top artists/songs/albums over 4 weeks from the play log; also a year recap that, unlike Spotify, counts every local file | Medium |
| 1 | Settings → Playback in Spotify layout | Crossfade, Automix, Normalization, Equalizer, Mono | Backends exist; expose them as one Spotify-style Playback page | Low |
| 2 | Smart Shuffle | Queue / play bar; recommendations interleaved and marked | Insert library tracks that share artist/genre/decade with the context; mark them with a sparkle; one tap to remove or add | Medium |
| 2 | Autoplay after the context ends | Redesigned queue shows recommendations after the end; toggle in Settings | When repeat is off, continue with a similar-track mix instead of stopping; Settings toggle, default on, as on Spotify | Medium |
| 2 | Go to song/artist radio | Track menu "Go to radio" | Local radio seeded from a track or artist using tags and play history | Medium |
| 2 | Hide in this playlist / Snooze | Now Playing menu (Sep 2025) | Hide skips the track in that playlist (non-destructive flag); Snooze keeps it out of Home mixes, radio and Smart Shuffle for 30 days | Low |
| 3 | Smart Filters in Your Library | Filter icon, mood/genre/activity | Genre, decade and format filters from tags (no mood data offline) | Low |
| 3 | Liked Songs by genre | Generate playlists from Liked Songs (select markets) | Genre chips on Liked Songs that filter in place | Low |
| 3 | Daily Mix / Daylist | Up to six genre mixes; Daylist changes by time of day | Extend "Made for you" into genre clusters; a time-of-day mix from the hour of past plays | Medium |
| 3 | Custom playlist cover | Playlist customization | Choose any album cover in the library or a 2x2 mosaic of the first four albums | Low |

Not suitable offline or on the R1: Jam, DJ voice, AI playlists, Talk to
Spotify, Canvas video, Countdown pages, social sharing, lossless toggle (local
files are already lossless), downloads, running mode (no confirmed motion
sensor on the R1). Mix-style per-transition editing is possible but expensive
for a 480x800 touch screen; crossfade covers the common need.

## B. Features Spotify does not have that suit the R1

| Priority | Feature | Why it fits a DAP | Notes |
|---|---|---|---|
| 1 | Sound sheet from Now Playing | The DSP is a reason to own a DAP; today it lives in settings pages | One Spotify-style sheet: EQ/PEQ preset, MSEB, ReplayGain mode, crossfeed, output; reuse existing backends |
| 1 | Format and signal-path badge | Stock firmware shows sample rate/bit depth; audiophile users expect it | "FLAC 24/96" chip under the title, tap opens the existing file-details sheet |
| 1 | Save queue as playlist | Rockbox and HiBy Music offer it | Queue sheet action; reuses the naming layer |
| 1 | Smart playlists | Local tags make rule-based lists reliable (foobar2000, Rockbox database) | Rules: genre, artist, year, date added, play count, last played, format, duration; bounded SQL; rendered by the shared collection page |
| 2 | Album shuffle | Album listeners shuffle albums, not tracks (Rockbox/foobar2000 convention) | Shuffle setting: Songs / Albums; albums play in track order |
| 2 | Play next vs add to end | Rockbox insert next/last | Spotify has only Add to queue (FIFO). Consider "Play next" as a second sheet row |
| 2 | Per-output EQ profiles | Wired IEMs and Bluetooth headphones need different tuning | Remember an EQ/PEQ preset per Bluetooth device and for wired output |
| 2 | Lyrics timing offset and .lrc sidecars | Local LRC files are often out of sync | +/- offset saved per track; prefer an .lrc file next to the audio |
| 2 | Library health | Large card libraries collect missing art, untagged files and duplicates | Read-only report in Your Library settings; never deletes audio |
| 3 | Composer / work browsing | Classical libraries | Filter chip from COMPOSER tags when present |
| 3 | Long-track resume | DJ mixes and long classical tracks | Remember the position for tracks longer than ~20 min, as audiobooks do |
| 3 | Storage overview | Spotify has Settings → Storage for its cache | Card usage, library size by format, index size |

## Open questions

- Spotify's own Playback-settings support page was not reachable; confirm the
  labels before building the Playback page.
- The Listening stats window (4 weeks or 7 days) differs between sources.
- Smart Shuffle and radio quality depend on tag coverage; they need a fallback
  (same artist, then same genre, then random) and must stay bounded on 64 MB.
- Play-log writes must not wear the card or stall playback; batch them.

## Implemented 2026-11-04

Recents, Listening stats, the Spotify Settings categories (with Playback and
Audio quality) and Autoplay were built from this list; see UI_DECISIONS.md
"Profile drawer, Recents, Listening stats, Settings categories, Autoplay" for
the additional sources (9to5Google 2023-10-04, Spotify support Listening stats,
SlashGear 2025-09-14) and the R1 adaptations. Still open from section A: Smart
Shuffle, song/artist radio, Hide in this playlist / Snooze, Smart Filters,
Liked Songs genre filters, Daily Mix / Daylist, custom playlist covers.

## Implemented 2026-11-04 (night)

From section B: format badge, Sound sheet, Play next, Save queue as
playlist, Library health (read-only). From section A: song radio, Smart
Filters (genres only). Still open: Smart Shuffle, Hide / Snooze, Liked Songs
genre chips, Daily Mix / Daylist, custom covers; and from section B smart
playlists, album shuffle, per-output EQ, lyrics offset, composer browsing,
long-track resume, storage overview.

## Implemented 2026-11-05

Section A: Snooze (30 days; out of Autoplay, radio and recommendations),
playlist recommended songs. Section B: long-track resume, lyrics timing
offset, album shuffle. Still open: Smart Shuffle, Hide in this playlist,
Liked Songs genre chips, Daily Mix / Daylist, custom covers; smart playlists,
per-output EQ profiles, composer browsing, storage overview.

Also built 2026-11-05: Daylist (offline, by hour of past plays) and the
storage overview.

## Implemented 2026-11-05 (night)

Everything remaining from sections A and B: Smart Shuffle, Hide in this
playlist (skips on Next too), Liked Songs genre chips, custom playlist
covers, smart playlists, per-output sound, composer browsing; and the Up
next / Credits cards from the captures. Nothing from the list is open.
