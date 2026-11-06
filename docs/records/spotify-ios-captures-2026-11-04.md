# Spotify iOS captures, 2026-11-04

Source: 78 screenshots the user took on their own iPhone with the current
Spotify iOS app (Premium Family, Data saver on), supplied in this session on
2026-11-04. Status-bar times run from 01:54 to 14:14 on that day; the Listening
stats page reads "This week, Nov 2 - 8" and Recents "Nov 2, 2026". These are
first-hand, dated mobile captures: the strongest visual evidence the project
has. Confidence: high for layout and wording. Pixel sizes below are scaled
from the 738 px-wide captures to the R1's 480 px (factor 0.65), so they are
approximate.

The images are not checked in: they show the user's account name, friends,
messages and listening. This record keeps the observations only.

## Observations by screen

| Screen | What the captures show | R1 status |
|---|---|---|
| Pushed page header | Round dark disc (~56 px) holding the back chevron; the title centred, bold; a second disc at the right where the page has an action (Settings: search) | Built: shared back disc and centred titles for every settings-scaffold page |
| Profile menu | Panel about 90% of the width (664/738); a raised rounded card with the avatar, name and "View profile" (plus "Add account"); rows with 28 px outline icons: Your Premium [Family], Listening stats, Recents, Taste Profile, Your Updates, Settings and privacy; then friends' activity and Messages | Built: 432 px panel, raised profile card, 60 px rows. No account, Premium, Taste Profile, Updates, friends or Messages offline |
| Settings | Back disc, centred "Settings", search disc. Three tiles: Data saver mode (Always on, green), Offline mode, Private session. Category rows: outline icon, name, grey summary joined with " · ", chevron. Account, Parental controls, Content and display, Privacy and social, Playback (Gapless playback · Autoplay), Notifications, Apps and devices, Data-saving and offline, Media quality, About; white "Log out" pill | Built: tiles (Wi-Fi, Bluetooth, Private session), icon + summary + chevron rows. This corrects the 2026-11-04 morning decision of no chevrons |
| Search settings | Back disc, "Search"; a rounded field "Search settings" with a magnifier; empty state "Search settings / Find settings by name or description"; keyboard up | Built |
| Listening stats | "This week (i)" + "Nov 2 - 8", share icon. Masonry tiles: "Minutes listened 85 >", "Top artist AZALI" (round picture), "Top song Princesa" (square art), friends tiles. "Your latest insights" story cards. "Past weeks": cards with art, "Oct 26 - Nov 1", top song, "3 plays", chevron | Built: week Monday-Sunday, minutes from new LISTEN_TIME, tiles open the week's ranked artists/songs, past weeks (3, from the 30-day counts). No friends, insights or sharing |
| Recents | "Music" chip. Day headings ("Yesterday", "Nov 2, 2026"). One row per place played from: cover, name, "60 songs played · Playlist · Kroaky", round chevron; opened, the songs indented under it with "..." | Built: grouped by the queue's context (new PLAY_HISTORY ctx_kind/context), folded, round chevron; songs from no known place group as "N songs played" |
| Queue sheet | "Queue", "Playing study music"; Clear and Edit pills; now playing in green with a white play disc; rows with drag handles; fixed tiles at the bottom: Shuffle, Repeat, Timer (and Mix when available). Edit Queue: radio circles, handles, "Move up" and "Remove" pills | Built (evening): no Mix (no transitions offline); section headings kept |
| Now Playing | Context name only at the top ("study music"); like as (+) or green check disc; remaining time as "-0:48"; devices left, share and queue right; Lyrics card, About the artist; further down Up next (Open Queue), Credits (Show all), Explore cards | Partial: remaining time built; bottom row and Up next/Credits cards are gaps |
| Track menu | Header art, title, "artist · album". Share, Add to playlist, Exclude from taste profile, Remove from this playlist, Add to Queue, Go to Queue, Start a Jam, Go to radio, Go to album, Go to artist, Explore SongDNA, Go to artist concerts, View song credits, Sleep timer, Show Spotify Code | Built: Go to radio (local) and Sleep timer; credits are the existing Track information page |
| Saved in sheet | "Saved in" + green "New playlist"; Liked Songs with a green check; divider; playlists with (+); expanded: Cancel/Done, "Find playlist" + Sort, floating green Done | Partial (existing membership sheet) |
| Home | Chips All/Music/Podcasts/Audiobooks (Music adds a joined "Following"); 2x4 shortcut grid; shelves with a grey type line above a bold title ("Album / ZELDA..."); "More like X" with a round avatar; big "Made for you" card with Preview, (+) and play | Partial |
| Search | Avatar + "Search" + camera; a white field "What do you want to listen to?"; coloured category tiles with tilted art; active field with Cancel pill; Recent searches with (+) and x; suggestions with an arrow | Built: white field and wording; camera, Cancel pill and suggestions are not |
| Your Library | Avatar, title, search, +; a filter disc then chips (Playlists, Podcasts, Albums, Artists, Downloaded); "Recently added" with sort icon, grid/list toggle. Filter sheet: types, Genres (counts), Moods, Activities. Sort sheet: Recents, Recently added, Alphabetical, Creator. Create sheet: Playlist, Collaborative, Mixed (Beta), Blend, Prompted (Beta), Jam, Folder | Built: filter button and sheet with genre counts (no moods or activities offline) |
| Playlist page | Art, name, description, owner, duration; download, share, ..., shuffle, play; chips Add, Mix, Video, Edit, Sort, Name & details; "Recommended songs, Based on the songs of this playlist" with (+) and Refresh. Sort sheet: Custom order, Title, Artist, Album, Recently added. Name & details sheet: cover with pencil, name, description, Make private, Delete playlist | Partial: pills built (Add, Edit, Sort, Name & details; no Mix or Video); recommended songs open |
| Album page | Art, title, artist avatar + name, "Album · Jan 5, 2025"; save, download, ..., shuffle, play; "17 songs · 1hr 2min"; More by artist | Present (shared collection page) |
| Artist page | Hero image, name, listeners; Following, ..., shuffle, play; Popular numbered with Show more; Popular Releases with See discography; Releases page with Albums / Singles and EPs / Compilations chips | Present (discography) |
| Lyrics | Full-screen in the art's colour, large bold lines | Present |

## Deliberate adaptations (this round)

- Quick tiles: the R1 has no data plan and is never online for music, so
  Wi-Fi and Bluetooth take the Data saver and Offline places; Private session
  is Spotify's and is not kept across a restart (Spotify ends a session on its
  own). While it is on, nothing reaches Recents, the play counts, minutes
  listened or Last.fm.
- Weeks are counted in UTC days, as the play counts are; at the R1's local
  midnight a play near the boundary can fall in the neighbouring day.
- Recents rows open in place (fold/unfold); Spotify's tap on the row opens the
  playlist, which needs a lookup by name the history does not carry yet.
