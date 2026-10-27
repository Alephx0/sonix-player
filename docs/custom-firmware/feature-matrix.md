# Upstream compatibility inventory

Baseline commit: `fe94fe796d93c6efa50d1107a734864c9c6a03d5`. Generated before application changes. Each documented top-level feature is retained as its own row; descriptions include sub-capabilities. Preserved source is not proof of operational compatibility. Hardware-only behavior requires physical testing.

## Requested additions and acceptance ledger

Status meanings: **host-tested** has explicit executable/screenshot evidence;
**implemented/unverified** exists in source but lacks the required coverage;
**partial** omits named behavior; **unsupported** is outside the current adapter's
capabilities; **blocked** requires hardware or another unavailable dependency.
No row marked preserved source implies all of its behavior was tested.

| Requested capability | Status | Evidence or remaining work |
|---|---|---|
| Fork, version control, complete upstream inventory | Implemented | Fork/branch recorded in README; 128 baseline entries below, including their sub-capabilities |
| R1 model/revision, partition and runtime verification | Blocked | Stock 2.0 inspected; no physical board identity, partition limits or whole-system memory measurement |
| REA installed and actually used on SpotBye | Host-tested | MCP evidence and Go/PE inspection in reference report |
| SpotBye dynamic/network workflow traces | Blocked | No isolated Windows VM; executable was not launched |
| Mornye desktop/core/extensions investigation | Implemented | Source/licence/capability report; no Flutter/QuickJS runtime port |
| Three primary tabs and secondary Tools/Settings | Host-tested | Actual 480×800 simulator captures; all six former destinations routed through Tools |
| Home recent listening, recent albums, favourites/playlists shortcuts | Partial | Six history entries, four thumbnail cards and fixed shortcuts implemented; empty/small-library screenshots; populated history not stress-tested |
| Configurable Home shortcuts/personalized or pinned collections | Partial | Existing customizable classic library remains; new Home configuration and pins absent |
| Unified Local / Online / All search and native keyboard | Host-tested | Local artist hit plus live release search; keyboard and results captured |
| Search suggestions, ranking, pagination | Unsupported | Literal release query, maximum 24 candidates; refine-query message; no suggestions or pagination |
| Online artists, tracks, albums, playlists, podcasts | Partial | Release search/listing supports title/creator and track variants; no dedicated online artist pages or global track/playlist API; podcasts stay in existing service screens |
| Album detail / format selection / source download action | Host-tested | Live release audio variants and format/size labels; no new artwork-rich detail page |
| Local library sorting/filtering, playlists and favourites | Implemented/unverified | Routes reuse existing pages; exhaustive editing/sorting regression pending |
| Persistent mini-player | Partial | Artwork/title/artist/state/transport/progress implemented on primary tabs; existing navigation owns legacy pages; actual audio-output state testing pending |
| New Spotify-inspired full-screen Now Playing | Partial | Existing functional Standard/Studio/waveform layouts retained; no new custom layout delivered |
| Now Playing lyrics/queue/favourite/quality/shuffle/repeat | Implemented/unverified | Existing code and actions retained; simulator opens player; comprehensive behavioral verification pending |
| Remote artwork cache and embedded artwork/tag enrichment | Partial | Local thumbnail loader reused; source metadata sidecar implemented; no remote-art cache or embedded retagging |
| Native replaceable provider capability interface | Partial | Bounded native API and explicit capabilities implemented; one adapter, no runtime registry/provider selection UI |
| Provider settings, quality/fallback/network configuration | Unsupported | Source-format selection exists; no general provider configuration or fallback order |
| Authenticated providers and companion service | Unsupported | No companion or additional authenticated provider; existing Sonix services retained |
| Online streaming preview from discovery | Unsupported | Adapter exposes downloads only; user-facing explanation provided |
| Authorized source selection and input validation | Host-tested | Explicit public licence policy, HTTPS/source checks, restricted/private filtering and malformed-input tests |
| Download queue/progress/cancel/pause/resume | Host-tested | Eight jobs, deterministic regression, simulator progress cards; physical Wi-Fi behavior pending |
| Persistent queue and restart recovery | Host-tested | Restored jobs stay paused; final files reverified on resume; cancelled entries skipped |
| Interrupted transfers, retries, corruption, storage removal/full | Host-tested | Mocked transfer/storage cases plus real TLS range/status/length tests; hardware failure injection pending |
| Duplicate and concurrent-destination protection | Host-tested | URL+digest queue deduplication; existing-file digest; atomic no-replace regression |
| Organization, tagging and automatic indexing | Partial | Fixed Downloads folder, safe filenames, attribution sidecar, real incremental library import; configurable folders/embedded retagging absent |
| Offline playback of downloaded audio | Host-tested, decoder only | Fresh 6,419,897-byte licensed MP3, verified checksum, index and 1,024 decoded frames; DAC output unverified |
| Networking without UI/audio blocking | Partial | Catalog and transfer workers; transfer nice 10; DNS/connect cancellation and device-detach joins may wait; actual underruns unmeasured |
| Memory/graphics/power optimization | Partial | Bounded result/queue buffers, lazy widgets, four small artwork slots, Wi-Fi-use hook; physical memory/power/fragmentation tests pending |
| Localization, accessibility and long-label handling | Partial | Upstream support retained; long result/card labels clipped cleanly; new English strings not localized or accessibility-tested |
| Host simulator and MIPS build/ABI | Host-tested | Host runs; native ELF32 MIPS32r2/o32 build and glibc 2.22 check pass |
| Broad regression, long sessions and large libraries | Blocked/remaining | New-path tests pass; 128 upstream feature rows are not a full regression certification |
| Latest-board Wi-Fi/Bluetooth/kernel compatibility | Blocked | Stock selector supports AIC8800; Sonix overlay/startup currently Broadcom-only |
| Validated R1 image, physical boot and stock recovery | Blocked | Packer refuses incompatible stock; no flash image or physical recovery trial |
| Build, architecture, tests, maintenance, installation docs | Implemented | Linked from project README; installation explicitly deferred until compatibility is established |

## Upstream baseline: 128 entries

The main menu was replaced; the original six destination screens are still
reachable through Tools. The decoder, DSP and service implementations were not
replaced. Shared changes affect HTTP request options, incremental MD5, library
imports/metadata, storage lifecycle and Wi-Fi idle use, so those paths require
regression attention. MP3 decoding and basic WAV indexing have focused tests;
they do not certify every format or audio feature in the rows below.

| Area | Upstream capability | Evidence | Fork status |
|---|---|---|---|
| Supported audio formats | `.wav` `.flac` `.mp3` `.ogg` `.opus` `.m4a` `.m4b` `.mp4` `.aac` `.wv` `.wvc` `.ape` `.dsf` `.dff` `.aif` `.aiff` `.aifc` `.caf` | FEATURES.md:11 | Preserved upstream source; regression verification pending |
| Supported audio formats | **CUE sheets** - a whole disc ripped into one file with a `.cue` next to it shows up as separate tracks. | FEATURES.md:13 | Preserved upstream source; regression verification pending |
| Supported audio formats | **Tags in WAV and AIFF** are read too, cover included. | FEATURES.md:14 | Preserved upstream source; regression verification pending |
| First start | Choose the language, then connect to Wi-Fi so the date and time are set by themselves. You can skip the Wi-Fi step. | FEATURES.md:18 | Preserved upstream source; regression verification pending |
| First start | If there is no microSD card in the player, a message tells you. | FEATURES.md:19 | Preserved upstream source; regression verification pending |
| Music library | **Main page** - All tracks, Albums, Artists, Album artists, Genres, Years, Favourites, Most played, Playlists and Browse. You can rearrange the tiles and hide the ones you don't use. | FEATURES.md:25 | Preserved upstream source; regression verification pending |
| Music library | **Browse** - go through the folders of the card as they are. | FEATURES.md:26 | Preserved upstream source; regression verification pending |
| Music library | **Search** across tracks, albums and artists. | FEATURES.md:27 | Preserved upstream source; regression verification pending |
| Music library | **Sorting** - by name (A-Z or Z-A), by date added (newest or oldest first) or by year. | FEATURES.md:28 | Preserved upstream source; regression verification pending |
| Music library | **A-Z bar** on the side when you scroll a long list. | FEATURES.md:29 | Preserved upstream source; regression verification pending |
| Music library | **Sorting works with every alphabet**, ignores upper and lower case and accents, and skips words like "The" at the start (you can turn that off). | FEATURES.md:30 | Preserved upstream source; regression verification pending |
| Music library | **Albums with the same name stay apart** when they belong to different artists or folders. | FEATURES.md:31 | Preserved upstream source; regression verification pending |
| Music library | **Split artists and genres** - a track with "A & B" or "A feat. B" shows up under both names. You choose the symbols that split them, and you can add names that must never be split. | FEATURES.md:32 | Preserved upstream source; regression verification pending |
| Music library | **Join albums** - tracks of the same album in the same folder become one album, even if the artists are different. | FEATURES.md:33 | Preserved upstream source; regression verification pending |
| Music library | **Detect changes** - new tracks are added by themselves when the player starts, when the microSD goes back in, after USB or after Wi-Fi transfer. It can also pick up tracks you retagged. | FEATURES.md:34 | Preserved upstream source; regression verification pending |
| Music library | **Alternative album view** - the album opens with its big cover on top, with title, artist, genre and year, and the tracks underneath. | FEATURES.md:35 | Preserved upstream source; regression verification pending |
| Music library | **Tapping an artist, a genre or a year** can open its albums instead of its tracks. | FEATURES.md:36 | Preserved upstream source; regression verification pending |
| Music library | **Show artist** under the title in the lists you choose. | FEATURES.md:37 | Preserved upstream source; regression verification pending |
| Music library | **Quality badge** under the title: lossy, CD, Hi-Res or DSD. | FEATURES.md:38 | Preserved upstream source; regression verification pending |
| Music library | **Go to current track** - the lists open on what is playing. | FEATURES.md:39 | Preserved upstream source; regression verification pending |
| Music library | **Favourites** and **Most played** - the player counts how many times you listen to every track. | FEATURES.md:40 | Preserved upstream source; regression verification pending |
| Music library | **Selection** - long press a track, album or artist to select more, then add them all to the queue, to the favourites or to a playlist, or remove them from a playlist. | FEATURES.md:41 | Preserved upstream source; regression verification pending |
| Music library | **Delete tracks** from the card, after asking (turn it on in the settings first). | FEATURES.md:42 | Preserved upstream source; regression verification pending |
| Music library | **Playlists** - create, rename, reorder and delete. You can back them up to the card as `.m3u` files that other players can read, and import `.m3u` playlists back, even when some files were moved. | FEATURES.md:43 | Preserved upstream source; regression verification pending |
| Playback | **Playback modes** - normal, repeat all, repeat one, shuffle, and a continuous shuffle that deals a new order every round. | FEATURES.md:47 | Preserved upstream source; regression verification pending |
| Playback | **Play albums back to back** - when an album ends, the next one starts. | FEATURES.md:48 | Preserved upstream source; regression verification pending |
| Playback | **Play by folder** - when a folder ends, playback goes on with the next folder. | FEATURES.md:49 | Preserved upstream source; regression verification pending |
| Playback | **Add to queue** - as the next track or at the end. | FEATURES.md:50 | Preserved upstream source; regression verification pending |
| Playback | **Queue** - reorder it, select tracks in it, and see the **history** of what you played today, yesterday and before. | FEATURES.md:51 | Preserved upstream source; regression verification pending |
| Playback | **Gapless** - no silence between tracks of the same album, also over Bluetooth. | FEATURES.md:52 | Preserved upstream source; regression verification pending |
| Playback | **Fade** - the end of a track fades into the next one, from 1 to 12 seconds. | FEATURES.md:53 | Preserved upstream source; regression verification pending |
| Playback | **ReplayGain** - off, by track or by album, read from the tags. | FEATURES.md:54 | Preserved upstream source; regression verification pending |
| Playback | **Remember track** - the queue and the position are still there after a restart. You can also open Now Playing straight away at startup. | FEATURES.md:55 | Preserved upstream source; regression verification pending |
| Playback | **Sleep timer** - set a time and the playback pauses by itself when it runs out. There is one for music, one for audiobooks and one for podcasts. | FEATURES.md:56 | Preserved upstream source; regression verification pending |
| Now Playing | The cover fills the top of the screen, with the same picture blurred behind the controls. | FEATURES.md:60 | Preserved upstream source; regression verification pending |
| Now Playing | **Three layouts** - Standard, Waveform (the progress bar becomes the waveform of the track) and Alternative. | FEATURES.md:61 | Preserved upstream source; regression verification pending |
| Now Playing | **Cover colour** - the buttons and the bar take the main colour of the cover. | FEATURES.md:62 | Preserved upstream source; regression verification pending |
| Now Playing | **Cover Flow** - swipe up from the bottom edge of the Music page to flip through your albums. | FEATURES.md:63 | Preserved upstream source; regression verification pending |
| Now Playing | **Queue**, **Details** (format, bitrate, size, ReplayGain and more) and **Show album** from the menu. | FEATURES.md:64 | Preserved upstream source; regression verification pending |
| Now Playing | **Chapters** and **speed** when an audiobook or a podcast is playing. | FEATURES.md:65 | Preserved upstream source; regression verification pending |
| Lyrics | Swipe on the cover to show the lyrics. Synced lyrics follow the song line by line, and the screen stays on while you read them. | FEATURES.md:69 | Preserved upstream source; regression verification pending |
| Lyrics | Lyrics are read from a `.lrc` or `.txt` file with the same name as the track, or from the tags of the file. | FEATURES.md:70 | Preserved upstream source; regression verification pending |
| Lyrics | **Download from LRCLIB** - when a track has no lyrics, the player can look for them online and you pick the one you want (synced or text only). If Wi-Fi is off, it asks to turn it on and starts the search by itself. | FEATURES.md:71 | Preserved upstream source; regression verification pending |
| Sound | **Graphic EQ** - 10 bands, ±12 dB, with 10 presets (Flat, Rock, Pop, Jazz, Classical, Dance, Bass, Treble, Vocal, Acoustic) and as many of your own as you like. | FEATURES.md:75 | Preserved upstream source; regression verification pending |
| Sound | **Parametric EQ (PEQ)** - 10 bands, each with its own frequency, gain, Q and shape (peak, low shelf, high shelf, low pass, high pass), plus a preamp. The graph shows the real curve. Tap a number to type it on a keypad. | FEATURES.md:76 | Preserved upstream source; regression verification pending |
| Sound | **AutoEq** - type the name of your headphones and the player downloads their correction over Wi-Fi. | FEATURES.md:78 | Preserved upstream source; regression verification pending |
| Sound | **MSEB** - the same sound tuning as the stock HiBy player, with 10 sliders (Temperature, Extended bass, Bass texture, Note thickness, Vocals, Female overtones, Male vocal, Female vocal, Instruments, Air). | FEATURES.md:79 | Preserved upstream source; regression verification pending |
| Sound | **Soundfield** - make the stereo image narrower (down to mono) or wider. | FEATURES.md:80 | Preserved upstream source; regression verification pending |
| Sound | **Crossfeed** - mixes a bit of each channel into the other, like speakers do. | FEATURES.md:81 | Preserved upstream source; regression verification pending |
| Sound | **Channel balance** and **Mono audio**. | FEATURES.md:82 | Preserved upstream source; regression verification pending |
| Sound | The EQ, PEQ and MSEB also work on radio and streaming. | FEATURES.md:83 | Preserved upstream source; regression verification pending |
| Sound | **DAC settings** - 4 digital filters, DRE, NOS, high gain, and gain compensation for DSD. | FEATURES.md:84 | Preserved upstream source; regression verification pending |
| Sound | **DSD** - played natively, or converted to PCM when the output cannot take it (for example over Bluetooth). | FEATURES.md:85 | Preserved upstream source; regression verification pending |
| Volume and outputs | **The same volume curve** as the stock player. | FEATURES.md:89 | Preserved upstream source; regression verification pending |
| Volume and outputs | **Remember volume** - one level for each output: 3.5 mm, 4.4 mm, USB-C and Bluetooth. | FEATURES.md:90 | Preserved upstream source; regression verification pending |
| Volume and outputs | **Volume limit** - a maximum volume for each output. | FEATURES.md:91 | Preserved upstream source; regression verification pending |
| Volume and outputs | **Volume steps** - how much one press of the volume key changes the volume. | FEATURES.md:92 | Preserved upstream source; regression verification pending |
| Volume and outputs | **Line out** - the jack goes to a fixed level for an amplifier, and your volume comes back when you turn it off. | FEATURES.md:93 | Preserved upstream source; regression verification pending |
| Volume and outputs | **USB DAC** - connect the player to a computer and it becomes its sound card, from 32 kHz to 384 kHz. You can choose whether it charges at the same time. | FEATURES.md:94 | Preserved upstream source; regression verification pending |
| Volume and outputs | **USB audio out** - plug a USB DAC or USB-C headphones into the player. | FEATURES.md:95 | Preserved upstream source; regression verification pending |
| Streaming | **Tidal** - sign in with the code on screen or the QR code. Low, High, CD and Hi-Res quality. Search, albums, artists, playlists, favourites, new releases, most streamed and recommended. You can add and remove favourites and manage your playlists from the player. | FEATURES.md:101 | Preserved upstream source; regression verification pending |
| Streaming | **Qobuz** - sign in with your account. MP3 320, FLAC 16/44.1, 24/96 and 24/192. Search, albums, artists, playlists, favourites and new releases. You can add and remove favourites and manage your playlists from the player. | FEATURES.md:102 | Preserved upstream source; regression verification pending |
| Streaming | **Radio** - thousands of stations by country, language or genre, plus search, favourites and recently played. MP3, AAC and FLAC stations. | FEATURES.md:103 | Preserved upstream source; regression verification pending |
| Streaming | **Last.fm** - scrobbles what you listen to. Without Wi-Fi the plays are kept and sent the next time the player connects. | FEATURES.md:105 | Preserved upstream source; regression verification pending |
| Podcasts | **Search** and **Trending** from the Podcast Index catalogue, or add a podcast by its feed address. | FEATURES.md:109 | Preserved upstream source; regression verification pending |
| Podcasts | **Follow** up to 100 podcasts. | FEATURES.md:110 | Preserved upstream source; regression verification pending |
| Podcasts | **Download** episodes to the card, one at a time or many together, and listen to them without Wi-Fi. | FEATURES.md:111 | Preserved upstream source; regression verification pending |
| Podcasts | **Podcasts on the card** - your own podcasts are played too. | FEATURES.md:112 | Preserved upstream source; regression verification pending |
| Podcasts | **Continue listening** - every episode remembers where you stopped. | FEATURES.md:114 | Preserved upstream source; regression verification pending |
| Podcasts | **Heard and to hear** - filter the episodes and mark them as heard or not. An episode counts as heard when you stop in its last 30 seconds, 1 minute or 2 minutes (you choose). | FEATURES.md:115 | Preserved upstream source; regression verification pending |
| Podcasts | **Play next**, delete downloaded episodes, newest or oldest first. | FEATURES.md:116 | Preserved upstream source; regression verification pending |
| Podcasts | **Speed** and **stop at the end of the episode**. | FEATURES.md:117 | Preserved upstream source; regression verification pending |
| Podcasts | **OPML** - import the podcasts you follow in another app, and export yours. | FEATURES.md:118 | Preserved upstream source; regression verification pending |
| Audiobooks | **Library, Series, Authors and Continue**, each sortable. Continue shows the books you started, with the finished ones one tap away. | FEATURES.md:125 | Preserved upstream source; regression verification pending |
| Audiobooks | **A book can be one file or a whole folder** - an `.m4b`, an `.mp3` or any other file with chapters, or a folder of files (with `CD 1`, `Disc 2` subfolders too). | FEATURES.md:126 | Preserved upstream source; regression verification pending |
| Audiobooks | **Chapters** are read from the file, and the position is remembered for every book. | FEATURES.md:127 | Preserved upstream source; regression verification pending |
| Audiobooks | **Speed** from 0.5x to 2.0x without changing the pitch. | FEATURES.md:128 | Preserved upstream source; regression verification pending |
| Audiobooks | **Skip buttons** - choose how far back and forward they jump: 10, 30 or 60 seconds. | FEATURES.md:129 | Preserved upstream source; regression verification pending |
| Audiobooks | **Rewind after pause** by a few seconds, so you don't lose the thread. | FEATURES.md:130 | Preserved upstream source; regression verification pending |
| Audiobooks | **Stop at the end of the chapter**. | FEATURES.md:131 | Preserved upstream source; regression verification pending |
| Audiobooks | **Bookmarks** (up to 64 per book) and the **summary** from the tags. | FEATURES.md:132 | Preserved upstream source; regression verification pending |
| Audiobooks | The progress bar shows the whole book or just the current chapter. | FEATURES.md:133 | Preserved upstream source; regression verification pending |
| E-books | EPUB 2 and 3, with covers, pictures, headings, lists, quotes and the table of contents. | FEATURES.md:139 | Preserved upstream source; regression verification pending |
| E-books | Japanese books that are written vertically are shown vertically. | FEATURES.md:140 | Preserved upstream source; regression verification pending |
| E-books | Text size, line spacing, word spacing and margins. | FEATURES.md:141 | Preserved upstream source; regression verification pending |
| E-books | Three reading themes (Paper, Sepia, Night) and three page-turn animations. | FEATURES.md:142 | Preserved upstream source; regression verification pending |
| E-books | **Bookmarks** - double tap the top of a page. Up to 64 per book. | FEATURES.md:143 | Preserved upstream source; regression verification pending |
| E-books | **Reading bar** at the bottom with chapter, page, progress, battery and clock, each one optional. | FEATURES.md:144 | Preserved upstream source; regression verification pending |
| More | **File manager** - browse the card, create folders, rename, move, copy and delete. Pictures and text files open right there. | FEATURES.md:148 | Preserved upstream source; regression verification pending |
| More | **Metronome** - 30 to 250 BPM, up to 7 beats per bar, and tap tempo. | FEATURES.md:149 | Preserved upstream source; regression verification pending |
| More | **Game Boy** (Gearboy) - plays Game Boy and Game Boy Color games, with their real names, save states, five palettes, shaders and color correction. | FEATURES.md:150 | Preserved upstream source; regression verification pending |
| More | **Flappy Bird**, with sounds. | FEATURES.md:152 | Preserved upstream source; regression verification pending |
| Wireless | **Wi-Fi** - see the networks around you, connect, forget, and see the details of the one you are on. | FEATURES.md:156 | Preserved upstream source; regression verification pending |
| Wireless | **Bluetooth headphones and speakers** - pair, connect and forget. The best codec your device supports is chosen by itself, and you can see the list. The volume can be synced with the headphones, and you can rename your paired devices. | FEATURES.md:157 | Preserved upstream source; regression verification pending |
| Wireless | **AirPods** - battery for each earbud and the case, and noise control (off, noise cancellation, transparency, adaptive). You choose which modes the stem switches between. | FEATURES.md:158 | Preserved upstream source; regression verification pending |
| Wireless | **Bluetooth receiver** - a phone or a computer connects to the player and plays through it, like a speaker. The codec and the quality of what comes in are shown. | FEATURES.md:159 | Preserved upstream source; regression verification pending |
| Wireless | **AirPlay** - play from an iPhone, iPad or Mac on the player. | FEATURES.md:160 | Preserved upstream source; regression verification pending |
| Wireless | **DLNA** - send music to the player from an app on your phone (needs a microSD card). | FEATURES.md:161 | Preserved upstream source; regression verification pending |
| Wireless | **Wi-Fi transfer** - open `sonix-transfer.local` (or the address on screen) in a browser on the same Wi-Fi to upload files and folders, download, rename, move and delete, and see hidden files. Music stops while it is on. | FEATURES.md:162 | Preserved upstream source; regression verification pending |
| Wireless | **SonixLink** - control the player from the SonixLink app on your phone, over Wi-Fi or Bluetooth: browse the library, the queue and the favourites, with covers. | FEATURES.md:163 | Preserved upstream source; regression verification pending |
| Wireless | **USB storage** - with the player on, connect it to a computer or a phone with the cable and the microSD shows up as a drive. With Detect changes on, the library picks up the new music when you unplug it. | FEATURES.md:164 | Preserved upstream source; regression verification pending |
| Control centre and status bar | Pull down from the status bar (or tap it) to open the **Control centre**: what is playing, brightness, and buttons for Wi-Fi, Bluetooth, AirPlay, DLNA, SonixLink, Wi-Fi transfer, EQ, PEQ, MSEB, Fade, Gapless, Gain, Line out and the three sleep timers. | FEATURES.md:168 | Preserved upstream source; regression verification pending |
| Control centre and status bar | Tap the title or the artist of what is playing to open **Now Playing**. | FEATURES.md:169 | Preserved upstream source; regression verification pending |
| Control centre and status bar | Choose which buttons are shown and in what order. | FEATURES.md:170 | Preserved upstream source; regression verification pending |
| Control centre and status bar | The status bar shows the clock and the battery, and can stay on in Now Playing too. | FEATURES.md:171 | Preserved upstream source; regression verification pending |
| Buttons and remotes | **Remap buttons** - give each media and volume button another action. | FEATURES.md:175 | Preserved upstream source; regression verification pending |
| Buttons and remotes | **Double click** - two quick presses of a button do the action you choose. | FEATURES.md:176 | Preserved upstream source; regression verification pending |
| Buttons and remotes | **In-line remote** - on wired headphones: one click play/pause, two clicks next, three clicks previous. USB-C headphones with buttons work too. | FEATURES.md:177 | Preserved upstream source; regression verification pending |
| Buttons and remotes | **Lock the buttons** with the screen off: volume down + power. The power button and the remote always work. | FEATURES.md:178 | Preserved upstream source; regression verification pending |
| Buttons and remotes | **Keyboard** - eleven layouts (English, Italian, German, Spanish, French, Dutch, Polish, Brazilian Portuguese, Czech, Russian, Japanese), as a full keyboard or as a phone keypad (T9). Japanese types romaji as hiragana or katakana, has a ten-key kana pad (tap a key again, or flick it, for the next kana of its row) and offers kanji candidates for the hiragana before the caret. | FEATURES.md:179 | Preserved upstream source; regression verification pending |
| Buttons and remotes | **Screenshots** - turn them on in Settings > More, then press volume up + power to save the screen to the card. | FEATURES.md:180 | Preserved upstream source; regression verification pending |
| Screen and appearance | **Dark and light theme**, accent colours, and a **dynamic tint** that colours the whole interface with the accent. | FEATURES.md:184 | Preserved upstream source; regression verification pending |
| Screen and appearance | **Text size** normal or large, **rounded corners**, battery percentage, and clock on the left, in the centre, on the right or hidden. | FEATURES.md:185 | Preserved upstream source; regression verification pending |
| Screen and appearance | **Brightness** down to 1%, **screen-off time**, **double tap to wake**, and **rotate the screen** by 180°. | FEATURES.md:186 | Preserved upstream source; regression verification pending |
| Screen and appearance | **Screensaver** - shows the cover of what is playing, or pictures from the `Screensaver` folder. | FEATURES.md:187 | Preserved upstream source; regression verification pending |
| Screen and appearance | **Boot logo** - the default one follows your theme and accent colour, or pick your own picture. | FEATURES.md:189 | Preserved upstream source; regression verification pending |
| Power and battery | **Standby** after the time you choose, and **auto power-off** when nothing is playing. | FEATURES.md:194 | Preserved upstream source; regression verification pending |
| Power and battery | **Battery screen when off** - plug in the charger with the player off and you see the battery level. Press the power button to see it again, hold it to start the player. | FEATURES.md:195 | Preserved upstream source; regression verification pending |
| Power and battery | **Charge limit** - stop charging before 100% to make the battery last longer: at 80% on the R1, or anywhere from 80% to 100% on the R3 Pro II. On the R1 you can also limit the charging current to 500 mA. | FEATURES.md:196 | Preserved upstream source; regression verification pending |
| Power and battery | **Low battery** - a warning at 5%, and at 1% the player turns itself off with a 10 second countdown, also in standby. | FEATURES.md:197 | Preserved upstream source; regression verification pending |
| Power and battery | **LED** - on or off, and off in standby. | FEATURES.md:198 | Preserved upstream source; regression verification pending |
| Power and battery | **Power menu** - slide to power off or restart. | FEATURES.md:199 | Preserved upstream source; regression verification pending |
| System | **Date and time** - automatic over Wi-Fi (the time zone is found by itself) or set by hand, with 24-hour or AM/PM clock and daylight saving time. | FEATURES.md:203 | Preserved upstream source; regression verification pending |
| System | **Languages** - English, Italiano, Deutsch, Español, Français, Nederlands, Polski, Português BR, Čeština, Russian, Chinese and Japanese. Titles in Korean, Thai and Arabic are shown too. | FEATURES.md:204 | Preserved upstream source; regression verification pending |
| System | **Firmware update** - over the internet with the full list of changes, or from a `.upt` file on the card. | FEATURES.md:205 | Preserved upstream source; regression verification pending |
| System | **About** - device, serial number, card space and version. | FEATURES.md:206 | Preserved upstream source; regression verification pending |
| System | **Factory reset** - settings, Wi-Fi networks and Bluetooth devices are erased. The music on the card is not touched. | FEATURES.md:207 | Preserved upstream source; regression verification pending |
| System | **Developer options** - tap "Build number" five times in About: ADB, log to the microSD, database log, Bluetooth log, and running processes and memory. | FEATURES.md:208 | Preserved upstream source; regression verification pending |
