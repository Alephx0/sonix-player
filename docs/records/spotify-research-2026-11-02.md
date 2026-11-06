# Spotify Mobile research records 2026-11-02

Produced by per-subsystem research agents following `docs/UI_RESEARCH.md`. "Read" marks a page whose contents were fetched; "snippet" marks a search result only, which is a lead, not verification. Evidence quality is as reported by the researcher and has not been independently re-fetched; treat inferred/unavailable records as provisional adaptations.

## shell

### Cold boot, welcome and first-run onboarding (Welcome greeting, Start button)

- Audit IDs: SHELL-BOOT, SHELL-FIRSTRUN-WELCOME
- Research date 2026-11-02; context: Android (Android Police how-to, updated 2023-03-10) and iOS+Android (TechCrunch 2018-10-18). Spotify account sign-up flow only; no current official visual spec found. App version unknown. Experiments unknown. Free vs Premium not distinguished.
- Sources:
  - [How to use Spotify on Android (Android Police)](https://www.androidpolice.com/how-to-use-spotify-on-android/), published 2023-03-10 (updated), accessed 2026-11-02, read
  - [Spotify's Premium app gets a big makeover (TechCrunch)](https://techcrunch.com/?p=1734533), published 2018-10-18, accessed 2026-11-02, read
  - [Getting started on Spotify (support, search snippet only)](https://support.spotify.com/to/article/getting-started/), published unavailable, accessed 2026-11-02, snippet
  - [Taste onboarding tag (newsroom, snippet only)](https://newsroom.spotify.com/tag/taste-onboarding/), published unavailable, accessed 2026-11-02, snippet
  - [Spotify new-user onboarding teardown (Uxcel)](https://app.uxcel.com/showcase/spotifys-new-user-onboarding-feature-product-teardown-222), published unavailable, accessed 2026-11-02, snippet
- Observed appearance: Documented only in text: launch lands on a sign-up/log-in step (Sign up free / log in) then single-question steps ('What's your email?', 'Create a password', date of birth, gender, profile name, Create account), then an artist-picker. Colour/typography of those screens was NOT verifiable from any read source; the Spotify convention is black/#121212 background with a full-width green pill primary action. Tone is short, one task per screen.
- Observed behavior: One question per screen with a single primary action; after onboarding the app lands directly on Home with the last-used bottom tab. Taste-picker is account-driven and has no offline meaning. No 'cycling greeting word' pattern was found in any source.
- Evidence quality: inferred
- Conflicts / confidence: Low confidence on visuals (no dated screenshots read; Uxcel page returned no body). Sources conflict on artist-pick count (3 vs 5) which is irrelevant here. Medium confidence on the one-question-per-screen structure.
- User requirement overrides: none recorded
- R1 adaptation: Offline adaptation: no account, no taste picker. Welcome panel: #121212 full screen, centred 28px bold 'Welcome' (cycling translated word may stay as a subtle #b3b3b3 font_ui_20 subtitle, not the headline), one green #1ed760 pill 'Get started' 400x56 px at y~660 with black font_ui_18 label, radius 28. Then each first-run step is one task per screen with the same pill action at the bottom and a text-only 'Skip' (48 px high, #b3b3b3) where skippable. After the last step load the Spotify shell on the saved tab (default Home). Remove legacy theme_style defaults/accent from these panels.
- Acceptance criteria:
  - Every first-run panel background is #121212 and uses one green pill primary action >=56 px high
  - Skip/secondary actions are text buttons with >=44 px hit area
  - Completing first run lands on the saved shell tab with mini-player/nav present and no legacy panel reachable via Back
  - Language->Wi-Fi->Clock sequence shares one header/footer component and step indicator

### App language list (first-run Language step and Settings > Language)

- Audit IDs: SHELL-FIRSTRUN-LANGUAGE
- Research date 2026-11-02; context: Mobile iOS and Android 13+ (support article, undated in fetched text); Android 12 and older defer to system language. Premium/free identical.
- Sources:
  - [How to change the app language (Spotify Support)](https://support.spotify.com/us/article/change-language/), published unavailable, accessed 2026-11-02, read
- Observed appearance: Not described visually. Path: profile picture > Settings and privacy > Content and display > App language, then choose a language from a list.
- Observed behavior: Selecting a language applies it; the article states no restart and no separate Confirm step. Language lives inside the Content and display settings page, not a top-level row.
- Evidence quality: documented
- Conflicts / confidence: High confidence for path and no-confirm behaviour (official). No visual confidence for the list screen.
- User requirement overrides: none recorded
- R1 adaptation: Single-select list on #121212: rows 64 px high, label font_ui_18 white, native name plus #b3b3b3 font_ui_16 secondary line, trailing green check on the selected row, 16 px side padding. Tapping a row selects it; on first run a bottom-anchored green pill 'Continue' (replacing the legacy Confirm) applies it. In Settings tapping a row applies immediately with no Confirm (matching Spotify) and shows a toast. Large language lists scroll with bounded rows.
- Acceptance criteria:
  - Selected language shows a green check; only one check at a time
  - Settings > Language applies on tap with no extra confirm and persists across restart
  - First-run variant uses the same row component and a pill Continue
  - Row hit area >=44 px, list scrolls without overlapping the pill

### Connect device picker and Settings > Apps and devices (analog for Wireless grid: Wi-Fi, Bluetooth, AirPlay, SonixLink, first-run Wi-Fi)

- Audit IDs: SHELL-TOOLS-WIRELESS, SHELL-FIRSTRUN-WIFI, SHELL-VOLUME-HUD
- Research date 2026-11-02; context: Mobile iOS+Android. Support pages undated; Linn wiki last edited 2022-10-28 (Premium account, Spotify Connect view). Bluetooth/AirPlay/Wi-Fi join are OS-level on Spotify so R1-only for those functions.
- Sources:
  - [Spotify Connect (Spotify Support)](https://support.spotify.com/us/article/spotify-connect/), published unavailable, accessed 2026-11-02, read
  - [Bluetooth help for Spotify (Spotify Support)](https://support.spotify.com/us/article/bluetooth/), published unavailable, accessed 2026-11-02, read
  - [Spotify: Mobile App (LinnDocs)](https://docs.linn.co.uk/wiki/index.php/Spotify:Mobile_App), published 2022-10-28, accessed 2026-11-02, read
  - [Select device before starting (Spotify Community, snippet only)](https://community.spotify.com/t5/Android/Android-Select-device-home-speakers-for-example-before-starting/m-p/5378917/highlight/true), published unavailable, accessed 2026-11-02, snippet
- Observed appearance: A 'Connect device' icon sits at the bottom of Now Playing; the Connect view is a list of devices with the active one highlighted in green text; per-device volume button shows a slider at the bottom. Overflow (three dots) per device offers 'Log out of this device'. Troubleshooting toggle 'Local device visibility' is under Settings > Apps and devices.
- Observed behavior: Tap Connect icon > choose a device > return to Now Playing with green device name. Only one device plays at a time. Bluetooth/AirPlay pairing is done in OS settings, then sound routes automatically. Volume is controlled through an in-app slider because OS hardware buttons cannot drive Connect volume on iOS.
- Evidence quality: documented
- Conflicts / confidence: High for flow; medium for labels ('Connect device' vs 'Devices Available' vary by version/region). Layout details not verified visually.
- User requirement overrides: none recorded
- R1 adaptation: Wireless tools become a 'Devices' page in the shell visual language: header 'Devices' with back chevron, rows 72 px (icon 40 px circle #282828, label font_ui_18, status font_ui_16 #b3b3b3 'Connected' in #1ed760), sections: This device (speaker/line out), Bluetooth, Wi-Fi, AirPlay, SonixLink, Wi-Fi transfer. Tapping a device row connects; overflow (three-dot 44x44) opens a shared bottom sheet for Forget/Rename/Disconnect. Wi-Fi scan list and password step reuse the same row + shared keyboard with a bottom pill 'Connect' and 'Skip' text button on first run. Reach it from Settings and from a Connect-style device chip on Now Playing/quick controls, not from a grid.
- Acceptance criteria:
  - No gridpage tiles remain in the Wireless route
  - Active device row shows green status text and check
  - Row actions open the shared bottom sheet (scrim, handle, X)
  - Wi-Fi first-run step uses same rows/keyboard as Settings > Wi-Fi
  - Hit areas >=44 px and back returns to the originating page

### Date, time and sleep-timer style pickers (first-run Clock; Settings > Date and time; timer sheets)

- Audit IDs: SHELL-FIRSTRUN-CLOCK
- Research date 2026-11-02; context: Mobile iOS+Android, music sleep timer via Now Playing three-dot menu (How-To Geek 2021-09-28). Clock/date setting has no Spotify counterpart (OS-provided).
- Sources:
  - [How to set up a sleep timer in Spotify (How-To Geek)](https://www.howtogeek.com/749269/how-to-set-up-a-sleep-timer-in-spotify/), published 2021-09-28, accessed 2026-11-02, read
  - [Sleep tag (Spotify Newsroom, snippet only)](https://newsroom.spotify.com/tag/sleep), published unavailable, accessed 2026-11-02, snippet
  - [Counting sheep (guessed URL, HTTP 404)](https://newsroom.spotify.com/2019-05-27/counting-sheep-sleep-timer/), published unavailable, accessed 2026-11-02, snippet
- Observed appearance: Sleep timer opens a screen/sheet titled 'Stop Audio In' listing 5, 10, 15, 30, 45 minutes, 1 hour and 'End of Track'; confirmation 'Your Sleep Timer is Set'; running timer shows remaining time beside the sleep icon in the three-dot menu and a 'Turn Off Timer' action. Row-based, no spinner wheels.
- Observed behavior: Single tap on a preset applies it and dismisses with a toast; reopening shows the countdown and an off action. Not available when playing through Connect devices.
- Evidence quality: community report
- Conflicts / confidence: Medium: a 2021 how-to article, not official; Spotify support page for music sleep timer was not found. Visual layout not described.
- User requirement overrides: none recorded
- R1 adaptation: There is no Spotify date/time picker, so adapt the preset-list pattern: Date and time page = settings rows (Automatic time toggle, Date, Time, Time zone, 24-hour toggle). Tapping Date/Time opens a shared bottom sheet (#202124) with three 64 px-high scroll columns (font_ui_28 values, selected row white, others #b3b3b3, 44 px min row) and a green pill 'Done' 48 px high. First-run Clock is the same page with a bottom pill 'Continue' and 'Skip'. Sleep-timer-like pickers use preset rows with trailing green check.
- Acceptance criteria:
  - Clock page uses settings rows and a shared sheet picker, no legacy spinner/title bar
  - Done applies the time and returns to origin (Settings or first-run flow)
  - Picker values reflect live clock and persist after restart
  - Selected preset shows a green check

### Bottom navigation bar (Home / Search / Your Library) incl. footer mounted on collection pages

- Audit IDs: SHELL-MAIN-TABS
- Research date 2026-11-02; context: iOS+Android mobile. 3-tab bar introduced by Premium redesign 2018-10-18; later builds add a Create button (2025 community report, snippet only; not adopted per AGENTS user spec). Experiment/rollout dependent.
- Sources:
  - [Spotify's Premium app gets a big makeover (TechCrunch)](https://techcrunch.com/?p=1734533), published 2018-10-18, accessed 2026-11-02, read
  - [Get rid of Create button on mobile navigation bar (Spotify Community, snippet only)](https://community.spotify.com/t5/Implemented-Ideas/Mobile-Get-rid-of-Create-button-on-mobile-navigation-bar/idc-p/6526103/highlight/true), published unavailable, accessed 2026-11-02, snippet
  - [How to open Spotify settings on Android (SlashGear)](https://www.slashgear.com/1965140/how-to-open-spotify-settings-on-android), published 2025-09-14, accessed 2026-11-02, read
- Observed appearance: Bottom bar with three tabs and labels; inactive grey, active white with filled icon. Sits directly below the mini-player.
- Observed behavior: Tapping a tab switches root; tapping the active tab returns to top of that tab. Tab state preserved while switching. Bar remains visible on drill-down pages such as playlists.
- Evidence quality: documented
- Conflicts / confidence: High that the 3 root tabs exist (2018 official-reporting); newer Create tab variant low-confidence and not required.
- User requirement overrides: none recorded
- R1 adaptation: Keep Home/Search/Your Library: bar 64 px high at y=736..800 on #121212 (or slight gradient), three equal 160 px hit regions, icon 28 px + font_ui_16 label, active white / inactive #b3b3b3, no green background. Mounted footer on collection pages identical (same component, not a clone). Reselecting active tab scrolls to top and pops detail pages. Keyboard open hides mini-player and nav.
- Acceptance criteria:
  - Bar height and tab bounds identical on main and mounted pages
  - Reselect pops to tab root
  - Active tab persists across restart
  - No tab hit area under 44 px

### Mini-player (Now Playing bar)

- Audit IDs: SHELL-MINI
- Research date 2026-11-02; context: iOS+Android mobile; Spotify Support article undated; Android gestures article 2026-03-01.
- Sources:
  - [Now Playing view (Spotify Support)](https://support.spotify.com/us/article/now-playing/), published unavailable, accessed 2026-11-02, read
  - [Spotify shortcuts gestures Android (Android Authority)](https://www.androidauthority.com/spotify-shortcuts-gestures-android-3644492/), published 2026-03-01, accessed 2026-11-02, read
- Observed appearance: Bar sits just above the bottom menu, showing artwork, title/artist, a Connect device icon, a save (add/check) icon and play/pause; thin progress line along the bottom edge (not described in fetched text).
- Observed behavior: Tap opens the larger Now Playing view; swipe left/right on the bar skips to next/previous; swipe up/down on it reveals/hides Now Playing.
- Evidence quality: documented
- Conflicts / confidence: High for tap-to-expand and swipe-to-skip; icon set varies by version.
- User requirement overrides: none recorded
- R1 adaptation: Bar 64 px high above the 64 px nav (y=672..736), margin 8 px, radius 8, tinted from artwork colour (#282828 fallback), art 48 px, title font_ui_16 white, artist font_ui_16 #b3b3b3 (ellipsis), play/pause 44x44, 2 px green progress at bottom. Body tap opens Now Playing; horizontal swipe skip is optional but must not conflict with swipe-back. Hide while keyboard is up.
- Acceptance criteria:
  - Body tap opens Now Playing; play/pause toggles without navigating
  - Bar absent when nothing loaded
  - Progress updates while audio plays
  - Same component on main tabs and mounted pages

### Home profile menu / Settings and privacy entry (header vertical-ellipsis and gear)

- Audit IDs: SHELL-HDR-TOOLS, SHELL-HDR-SETTINGS, SHELL-EASTEREGG
- Research date 2026-11-02; context: Mobile iOS+Android 2025. Settings is reached from the profile avatar at the top-left of Home (SlashGear 2025-09-14). Older builds (2020 and before) used a gear. A moderator reply (about May 2024) also notes a left swipe opens the profile menu.
- Sources:
  - [How To Open And Adjust Your Spotify Settings On Android (SlashGear)](https://www.slashgear.com/1965140/how-to-open-spotify-settings-on-android), published 2025-09-14, accessed 2026-11-02, read
  - [How to change the app language (Spotify Support)](https://support.spotify.com/us/article/change-language/), published unavailable, accessed 2026-11-02, read
  - [How do I access settings? (Spotify Community)](https://community.spotify.com/t5/Android/How-do-I-access-settings/m-p/5646089), published unavailable, accessed 2026-11-02, snippet
- Observed appearance: Settings and privacy lists in order: Account, Content and display, Playback, Privacy and social, Notifications, Apps and devices, Data-saving and offline, Media quality, Advertisements, About and support; a 'Go Premium' button sits at the very top. Row style (toggle vs chevron) not stated by any read source.
- Observed behavior: Avatar -> profile menu -> Settings and privacy -> category page -> option. Categories are pages, not a tile grid. There is no vertical-ellipsis overflow on Home.
- Evidence quality: documented
- Conflicts / confidence: High for category list (Android, 2025-09); iOS ordering may differ; row visuals low confidence.
- User requirement overrides: none recorded
- R1 adaptation: R1 has no profile; replace the ellipsis+gear pair with ONE header affordance: a 40 px circular avatar-style 'Settings' button (top-left, hit 48x48) opening a shared bottom sheet (#202124) with rows: Settings and privacy, Devices, Music library, Audiobooks, More tools. 'Settings and privacy' pushes a Spotify-style Settings root: header 'Settings' + back, 64 px list rows with 40 px icon, label font_ui_18, secondary font_ui_16 #b3b3b3, chevron. Group adapted to R1: Account->(none), Content and display (language, theme, screen), Playback (EQ, crossfade/gapless, speed), Apps and devices (Wireless, keys, control centre), Data-saving and offline (library scan/storage), Media quality (output/DAC), About and support (system, developer options, version row with the five-tap easter egg preserved, donation QR in a shared full-screen sheet). Remove the tools gridpage from this journey.
- Acceptance criteria:
  - Header has one settings entry; no ellipsis -> gridpage route
  - Settings root is a list of category rows and each category page uses the same row component
  - Developer options and About still reachable
  - Back from any settings page returns to the originating tab at the same scroll position

### Settings sub-pages: Playback / Equalizer / Media quality / Content and display (EQ, PEQ, MSEB, gapless, fade, line out, power, system)

- Audit IDs: SHELL-QUICKPANEL, SHELL-HDR-SETTINGS, SHELL-TOOLS-MUSIC
- Research date 2026-11-02; context: Mobile iOS and Android; equalizer article notes Android shows it under 'Listening controls'; undated support pages. Audio settings cannot be changed when playing via Connect.
- Sources:
  - [Equalizer (Spotify Support)](https://support.spotify.com/us/article/equalizer/), published unavailable, accessed 2026-11-02, read
  - [Audio quality (Spotify Support)](https://support.spotify.com/us/article/audio-quality/), published unavailable, accessed 2026-11-02, read
  - [Spotify Connect (Spotify Support)](https://support.spotify.com/us/article/spotify-connect/), published unavailable, accessed 2026-11-02, read
- Observed appearance: Equalizer: toggle at top of Playback, a preset chooser (names not stated) and draggable dots on a curve. Media quality: separate Wi-Fi, cellular and download quality choices, an 'Auto-adjust' toggle, Lossless listed under Premium. Rows with toggles on the right.
- Observed behavior: Toggle takes effect immediately; presets replace the curve; dragging dots customises. Quality options are mutually exclusive radio-style choices.
- Evidence quality: documented
- Conflicts / confidence: High for structure; preset names unavailable; visuals not read.
- User requirement overrides: none recorded
- R1 adaptation: Playback page rows: toggles (Gapless, Fade, Crossfade) as 64 px rows with 52x32 pill switch (green when on #1ed760, track #535353), 'Equalizer' row with value text (e.g. 'Rock') and chevron to an EQ page: toggle, horizontally scrollable preset chips (40 px high, selected = white fill/black text), and band sliders (44 px touch targets). PEQ/MSEB stay as sub-pages using the same components. Quality/DAC choices open a single-select bottom sheet with check. All confirmations use the shared dialog.
- Acceptance criteria:
  - All toggles use one switch component with 44 px hit area
  - Preset chips and sliders respond live while audio plays
  - Settings values persist across restart
  - No legacy accent colours in EQ/PEQ/MSEB pages

### Local files, Music library and Storage (Tools > Music, scan/import, file explorer, Search 'Files & folders')

- Audit IDs: SHELL-TOOLS-MUSIC, SHELL-TOOLS-MORE, SHELL-SEARCH-ESCAPES
- Research date 2026-11-02; context: Mobile iOS+Android; support articles undated. Spotify Local Files are secondary and gated by Settings > Apps and devices > Local audio files; Android clear-cache lives in Data-saving and offline > Storage.
- Sources:
  - [Local files (Spotify Support)](https://support.spotify.com/us/article/local-files/), published unavailable, accessed 2026-11-02, read
  - [Spotify storage information (Spotify Support)](https://support.spotify.com/us/article/storage-information/), published unavailable, accessed 2026-11-02, read
  - [Your Library (Spotify Support)](https://support.spotify.com/us/article/your-library/), published unavailable, accessed 2026-11-02, read
- Observed appearance: Local files appear as a 'Local Files' entry inside Your Library once the 'Local audio files' switch is on. Storage page has a 'Clear cache' row with a trailing 'Clear' action; downloads storage location selectable on Android. No folder-grid UI.
- Observed behavior: Toggle > files appear in Your Library as a collection; may need OS permission. Clearing cache never removes user files.
- Evidence quality: documented
- Conflicts / confidence: High for toggle path; supported formats not stated; visuals unread.
- User requirement overrides: none recorded
- R1 adaptation: R1 is local-first, so Local Files IS the library: no gridpage. Your Library lists Songs, Albums, Artists, Genres, Years, Folders as playlists-style rows/chips (All, Albums, Artists, Genres, Folders, Playlists). Scan/import lives in Settings > Data-saving and offline (rename 'Library & storage'): rows 'Scan library', 'Choose folders', 'Rescan on boot' toggle, 'Storage used' (value), 'Clear artwork cache' with trailing 'Clear' button. Empty library shows a centred empty state (icon, font_ui_20 headline 'Add your music', font_ui_16 #b3b3b3 body, green pill 'Choose folders') that opens a folder chooser sheet, then a scan progress page (progress bar green, bounded). 'Files & folders' becomes a Folders filter of Your Library using the shared row component. Never delete files.
- Acceptance criteria:
  - Empty index shows the empty-state pill; scan completes and returns to Your Library
  - Folders browsing uses collection-style rows with art placeholders
  - Clear actions never delete audio files (confirm dialog states it)
  - Scan runs off the UI thread and progress is visible

### Audiobooks (Tools > Audiobooks: authors, books, scan; chapters, speed, sleep timer)

- Audit IDs: SHELL-TOOLS-AUDIOBOOKS
- Research date 2026-11-02; context: Mobile iOS+Android, Spotify audiobooks (US/UK/IE/AU/NZ at launch); PopSci 2023-09-12, support article undated. Premium listening-time rules are irrelevant to R1.
- Sources:
  - [How to listen to Spotify's huge library of audiobooks (PopSci)](https://www.popsci.com/diy/spotify-audiobooks-tips/), published 2023-09-12, accessed 2026-11-02, read
  - [Unlock Audiobooks (Spotify Support)](https://support.spotify.com/us/article/audiobooks-unlock/), published unavailable, accessed 2026-11-02, read
  - [Audiobooks in Premium plans (snippet only)](https://support.spotify.com/us/article/audiobooks-premium-plans/), published unavailable, accessed 2026-11-02, snippet
- Observed appearance: Home has an 'Audiobooks' chip beside Music and Podcasts & Shows; Search shows an Audiobooks browse tile; Your Library has an Audiobooks filter. Player shows chapters, +/-15 s skip buttons, playback speed pill ('1x', 0.5x to 3.5x) and sleep timer.
- Observed behavior: Skip next/previous jumps chapters; chapters can be queued individually; playback speed tapped from Now Playing opens options; unlocked titles live in Your Library > Audiobooks.
- Evidence quality: documented
- Conflicts / confidence: Medium: speed range from a 2023 article; sleep timer for audiobooks is confirmed via Spotify support search snippet only.
- User requirement overrides: none recorded
- R1 adaptation: Audiobooks become a chip (All/Music/Audiobooks) in Home and a filter chip in Your Library, plus a Search browse tile. Book grid or list uses the collection page (cover 120 px in 2-col grid, title font_ui_16, author font_ui_16 #b3b3b3); book page shows chapters as rows with check/progress and a green Play pill; player gets 15 s back/forward, speed pill (opens shared sheet 0.5x-3.5x with check) and sleep timer sheet. No legacy audiobook grid remains.
- Acceptance criteria:
  - Audiobooks reachable from Home chip, Library filter and Search tile
  - Chapter rows show progress and resume position persists across restart
  - Speed and sleep timer open shared sheets from the player
  - Scan reuses the library scan page

### Your Library: search, sort/filter, create (+) sheet, playlist folders

- Audit IDs: SHELL-HDR-LIBSEARCH, SHELL-HDR-LIBCREATE
- Research date 2026-11-02; context: Mobile iOS+Android; XDA 2021-04-29 (rollout), Android Authority 2026-04-24 (folders), support pages undated. 'Create' label and menu entries vary by version (Playlist, Folder; Blend/Jam on accounts with social features).
- Sources:
  - [Sort and filter (Spotify Support)](https://support.spotify.com/us/article/sort-and-filter/), published unavailable, accessed 2026-11-02, read
  - [Playlist folders (Spotify Support)](https://support.spotify.com/us/article/playlist-folders/), published unavailable, accessed 2026-11-02, read
  - [Create and edit playlists (Spotify Support)](https://support.spotify.com/ls/article/create-playlists/), published unavailable, accessed 2026-11-02, read
  - [Spotify library redesign (XDA Developers)](https://www.xda-developers.com/spotify-library-redesign-improved-navigation/), published 2021-04-29, accessed 2026-11-02, read
  - [Playlist folders mobile rollout (Android Authority)](https://www.androidauthority.com/spotify-playlist-folders-mobile-rollout-3660742/), published 2026-04-24, accessed 2026-11-02, read
  - [Your Library (Spotify Support)](https://support.spotify.com/us/article/your-library/), published unavailable, accessed 2026-11-02, read
- Observed appearance: Header: title and icons (search, '+'), filter chips below (Playlists, Albums, Artists...), then a Sort-by row with list/grid icon. '+' opens a menu with 'Playlist' and 'Folder'. Naming prompt: field then Create.
- Observed behavior: Search icon searches only the collection (pull-down also reveals search); sort options Recents, Recently added, Alphabetically, By Creator, remembered per filter; swipe right/long press pins; '+' > Folder: name it and press enter; tap-hold > Move to folder.
- Evidence quality: documented
- Conflicts / confidence: High for features and labels; the naming-dialog exact title text ('Give your playlist a name') was not confirmed by a read source.
- User requirement overrides: none recorded
- R1 adaptation: Header 56 px: title 'Your Library' font_ui_24 bold, search and '+' icons 44x44. Chips row 40 px high (selected white fill black text; unselected #282828 white text). Sort row: label 'Recents' + icon opening a shared bottom sheet (radio rows with check, scrim). '+' opens the shared Create sheet with rows 'Playlist', 'Folder' (64 px, icon circle #282828). Naming: full-height sheet with font_ui_28 field, shared keyboard, green pill 'Create' disabled until non-empty. Search icon expands the in-page field (no separate legacy page); results are collection rows; Liked Songs pinned first.
- Acceptance criteria:
  - Create sheet uses shared bottom sheet and returns to Library after creating
  - Sort choice persists per filter across restart
  - Search results open the correct collection and Back returns to results
  - Collection changes never delete audio files

### Search tab and on-screen keyboard (field, suggestions, 'See all results', keyboard hides mini-player)

- Audit IDs: SHELL-SEARCH-KEYBOARD, SHELL-KEYBOARD, SHELL-SEARCH-ESCAPES
- Research date 2026-11-02; context: Mobile iOS+Android; TechCrunch 2018-10-18 (Browse folded into Search). OS keyboard on Spotify; R1 supplies its own.
- Sources:
  - [Spotify's Premium app gets a big makeover (TechCrunch)](https://techcrunch.com/?p=1734533), published 2018-10-18, accessed 2026-11-02, read
  - [Search autocomplete (Spotify Support)](https://support.spotify.com/us/article/search-autocomplete/), published unavailable, accessed 2026-11-02, read
  - [Spotify audiobooks tips (PopSci)](https://www.popsci.com/diy/spotify-audiobooks-tips/), published 2023-09-12, accessed 2026-11-02, read
  - [Search (Spotify Support, HTTP 429 on fetch)](https://support.spotify.com/us/listen_everywhere/on_phone_tablet_desktop/search/), published unavailable, accessed 2026-11-02, snippet
- Observed appearance: Search tab: a search bar at top, 'Browse all' grid of coloured category tiles below. While typing, suggestions follow popular terms, history and recognised names; results grouped with filter chips (Songs, Artists, Albums, Playlists...) per 2021 reporting (snippet only).
- Observed behavior: Tapping the bar focuses it and raises the OS keyboard; suggestions appear after a few characters; results page offers type filters; search terms such as year:/genre: are supported.
- Evidence quality: documented
- Conflicts / confidence: Medium: placeholder text 'What do you want to listen to?' and recents list not confirmed by any read source.
- User requirement overrides: none recorded
- R1 adaptation: Field 48 px high, radius 8, white bar with black text (Spotify mobile style), search icon left, clear 44x44. Keyboard docks at bottom (QWERTY/T9/JP) hiding mini-player and nav; Search key submits; tapping outside or switching tab dismisses. Results: chips row (All, Songs, Albums, Artists, Playlists, Folders) with rows; 'See all' opens an in-shell results list using the same chips, not the legacy page. Browse tiles: Songs, Albums, Artists, Genres, Years, Favourites, Most played, Folders, Audiobooks (2-col, 96 px tall, rounded 8, coloured).
- Acceptance criteria:
  - Keyboard show/hide restores mini-player and nav exactly
  - 'See all results' and 'Folders' stay inside shell visuals
  - Back from results returns to Search with query intact
  - Candidates and key hit areas >=44 px

### Confirm dialogs, OK-only notices, low-battery modal, toast/snackbar feedback

- Audit IDs: SHELL-CONFIRM, SHELL-TOAST, SHELL-LOWBATTERY
- Research date 2026-11-02; context: iOS 2019-09-08 critique for feedback strings; Android community reports of the 'Added to Liked Songs' pop-up at the bottom. No official dialog spec read. Delete playlist: support lists the action but not a confirmation.
- Sources:
  - [Spotify interaction critique (IXD@Pratt)](https://ixd.prattsi.org/?p=9221), published 2019-09-08, accessed 2026-11-02, read
  - [Create and edit playlists (Spotify Support)](https://support.spotify.com/ls/article/create-playlists/), published unavailable, accessed 2026-11-02, read
  - [Queue confirmation message should refresh (Spotify Community, body empty)](https://community.spotify.com/t5/Live-Ideas/Queue-Confirmation-Message-Should-Refresh-for-Each-Added-Song/idi-p/7338371), published unavailable, accessed 2026-11-02, snippet
  - [Spotify Design (301 redirect to open.spotify.com, no content)](https://spotify.design), published unavailable, accessed 2026-11-02, snippet
- Observed appearance: Feedback is a brief message at the bottom of the screen above the nav ('Added to Liked Songs', 'Removed from Liked Songs', queue added). Exact dialog styling unverified (Spotify iOS dialogs are dark centred cards with white pill primary and text secondary; this is memory, not evidence).
- Observed behavior: Toast appears immediately after the action, auto-dismisses, and is not a blocking modal; a quick second action refreshes it. Destructive actions (delete playlist) are listed without a documented confirmation.
- Evidence quality: community report
- Conflicts / confidence: Medium for toast; low for dialog visuals and timing (no official numbers found).
- User requirement overrides: none recorded
- R1 adaptation: Toast: bottom snackbar 48 px high, radius 8, #282828 or white-on-black per message, font_ui_16, 16 px side margins, placed 8 px above the nav/mini-player, auto-dismiss ~3 s, no scrim, replaced (not stacked) on repeat. Keep a centred card with 60% scrim only for blocking busy states. Dialog: 320x(auto) card #282828 radius 16, title font_ui_20 bold white, body font_ui_16 #b3b3b3, buttons stacked or side-by-side 48 px high: primary white or green pill, secondary text button; destructive uses red label. OK-only notice is one pill. Low-battery countdown stays non-dismissable but uses the same card with a progress ring and a clear time label; plugging in cancels it. 5% notice is a dialog with single 'OK'.
- Acceptance criteria:
  - One dialog component powers all ~20 confirm sites
  - Toast never blocks input, appears above mini-player/nav, and does not stack
  - Low-battery modal remains non-dismissable and cancels on charger
  - Dialog buttons >=48 px high and title/body wrap without overlap

### Context-menu bottom sheet, sort sheet and anchored popover menus

- Audit IDs: SHELL-POPOVER, SHELL-BOTTOMSHEET
- Research date 2026-11-02; context: iOS+Android mobile; long-press on a song opens its context menu (Android Authority 2026-03-01); three-dot menu horizontal on iOS, vertical on Android (Now Playing support page).
- Sources:
  - [Spotify shortcuts gestures Android (Android Authority)](https://www.androidauthority.com/spotify-shortcuts-gestures-android-3644492/), published 2026-03-01, accessed 2026-11-02, read
  - [Now Playing view (Spotify Support)](https://support.spotify.com/us/article/now-playing/), published unavailable, accessed 2026-11-02, read
  - [Sort and filter (Spotify Support)](https://support.spotify.com/us/article/sort-and-filter/), published unavailable, accessed 2026-11-02, read
  - [Sleep timer how-to (How-To Geek)](https://www.howtogeek.com/749269/how-to-set-up-a-sleep-timer-in-spotify/), published 2021-09-28, accessed 2026-11-02, read
- Observed appearance: Context menu is a dark bottom sheet with the item's art/title header followed by text rows with icons (Add to playlist, Add to queue, Go to album...). Three-dot menu on Now Playing scrolls (Sleep Timer is near the bottom). Anchored popovers are not documented for mobile.
- Observed behavior: Long-press or three-dot opens the sheet; tap row acts and dismisses; swipe down dismisses; scrolling lists in the sheet are allowed. Sort uses a sheet with options and a check on the active one.
- Evidence quality: documented
- Conflicts / confidence: Medium: structure confirmed by text; no spec of heights, scrim opacity or corner radius from an official source (Spotify Design site redirects away).
- User requirement overrides: none recorded
- R1 adaptation: Replace anchored popovers with the shared bottom sheet everywhere (sorts, track menus, speed, keyboard layouts, BT/Wi-Fi row actions): sheet #202124, top radius 16, handle 36x4 #535353, optional header row (art 48 + title/artist), rows 56 px high with 24 px icon + font_ui_18 label, active choice shows green check, max height ~70% of 800 and internal scroll, scrim #000 at 60%, drag handle >70 px or X (44x44) or scrim tap dismisses; Back is handled consistently (close the sheet first). Rows beyond 7 scroll.
- Acceptance criteria:
  - No anchored popover remains reachable from the shell
  - Sheet dismisses by handle drag, X, scrim tap and Back, and calls the cleanup callback
  - Active option shows green check
  - Rows >=48 px high; header art loaded asynchronously

### Back navigation, swipe-back and navigation stack

- Audit IDs: SHELL-BACK
- Research date 2026-11-02; context: Android and iOS: Spotify relies on the OS back gesture/button; swipe up/down on the playing bar opens/hides Now Playing (Android Authority 2026-03-01). Community reports that swipe left/right on the bar can skip tracks.
- Sources:
  - [Spotify shortcuts gestures Android (Android Authority)](https://www.androidauthority.com/spotify-shortcuts-gestures-android-3644492/), published 2026-03-01, accessed 2026-11-02, read
  - [Swipe down current playing view (Spotify Community, snippet only)](https://community.spotify.com/t5/Live-Ideas/Android-swipe-down-current-playing-song-view-instead-of-tapping/idc-p/5271307/highlight/true), published unavailable, accessed 2026-11-02, snippet
  - [7 Spotify gestures (iGeeksBlog, snippet only)](https://www.igeeksblog.com/spotify-gestures-every-user-should-know/), published unavailable, accessed 2026-11-02, snippet
- Observed appearance: Page headers show a back chevron (iOS) or arrow (Android) at top-left; the Now Playing view closes with a downward chevron/swipe.
- Observed behavior: Back pops one level; Now Playing closes by swipe-down; swipes on the bar change tracks, so gesture conflicts are known.
- Evidence quality: community report
- Conflicts / confidence: Medium; Spotify Android predictive-back adoption unconfirmed.
- User requirement overrides: none recorded
- R1 adaptation: R1 has no hardware Back: provide a persistent 44x44 chevron at top-left of every pushed page on #121212 (never over content), plus rightward edge-swipe >=80 px started within 24 px of the left edge with a snapshot underlay. Disable swipe-back while a sheet/keyboard is open and inside horizontal scrollers. Now Playing closes by down chevron/swipe-down. First-run flows and non-dismissable modals expose no back. Return to the exact origin tab and scroll offset.
- Acceptance criteria:
  - Every pushed page shows a chevron with >=44 px hit area
  - Swipe-back works from the left edge and pops exactly one level
  - Back never leaves a half-unloaded footer/mini-player
  - Sheet and keyboard block swipe-back

### R1-only system chrome: status bar, quick panel, power menu, screensaver, physical keys

- Audit IDs: SHELL-TOPBAR, SHELL-QUICKPANEL, SHELL-POWERMENU, SHELL-SCREENSAVER, SHELL-PHYS-KEYS, SHELL-VOLUME-HUD
- Research date 2026-11-02; context: none (R1-only). Closest Spotify patterns: Connect volume slider, Now Playing device/Connect sheet, media-session lock-screen controls (iPhone lock screen widget page returned HTTP 429, unread). Android quick-settings media tile not researched.
- Sources:
  - [Spotify Connect (Spotify Support)](https://support.spotify.com/us/article/spotify-connect/), published unavailable, accessed 2026-11-02, read
  - [Spotify lock screen widget on iPhone (HTTP 429)](https://support.spotify.com/us/article/spotify-iphone-widget/), published unavailable, accessed 2026-11-02, snippet
- Observed appearance: No Spotify equivalent for pull-down control centre or power menu. Connect volume is a slider at the bottom of the device view.
- Observed behavior: Volume is changed through an in-app slider; hardware buttons are reported as unreliable for Connect devices on iOS.
- Evidence quality: unavailable
- Conflicts / confidence: Low; adaptation is design inference from the Spotify visual language, not evidence.
- User requirement overrides: none recorded
- R1 adaptation: Status bar 32 px high transparent over #121212 with 14-16 px icons (volume, BT, Wi-Fi, battery %, clock font_ui_16 white), no legacy accent. Quick panel (swipe down from the top 24 px): bottom-sheet-style #202124 card with 56x56 circle toggles (on = #1ed760 black icon, off = #282828), brightness slider 44 px tall, now-playing card using the mini-player layout; long-press opens the matching Settings page. Power menu: shared dark dialog with a slide-to-power-off track (56 px, green thumb) and Cancel text button. Volume HUD: pill 44 px high, #282828, 3 s fade, shown only over Now Playing/screensaver unless allowed. Screensaver: artwork + clock, swipe up to dismiss; on wake keep shell state. Physical keys keep existing remap.
- Acceptance criteria:
  - Quick panel toggles use the shared switch/circle style and do not overlap the status bar
  - Power menu and slide-to-confirm cannot be triggered by accidental taps (slide >=60%)
  - Volume HUD never blocks controls and respects the 'allowed' setting
  - Physical keys behave identically with the screen on shell pages and Now Playing

Failed attempts:

- https://spotify.design returned a 301 redirect to open.spotify.com; no Spotify Design / Encore articles could be read (WebSearch for Encore bottom sheet/snackbar/dialog specs returned only third-party secondary material).
- https://newsroom.spotify.com/2019-05-27/counting-sheep-sleep-timer/ (guessed URL) returned HTTP 404; the newsroom sleep-timer claim rests on a search snippet only (actually_read=false).
- https://support.spotify.com/us/article/audiobooks-listen/ (guessed URL) returned 'Page not found'; https://support.spotify.com/us/article/delete-playlists/ also not found.
- Several Spotify Community threads (How do I access settings, Queue Confirmation Message) fetched with empty bodies, so no dated first-hand text from them.
- Android Central (Your Library facelift) and Uxcel (Spotify onboarding teardown) fetched navigation/empty content only; TechHive returned HTTP 410; DigitalCitizen returned HTTP 403.
- Spotify Support pages Search, Content and display, Listen offline, Recent activity (profile menu entries) and the iPhone lock-screen widget returned HTTP 429 after rate limiting; their details come from search snippets only and are marked actually_read=false.
- No dated visual evidence (screenshots or YouTube video pages with dates) could be read; fetched pages gave text only. Visual details of mini-player, Create sheet, dialogs, snackbar timing and Settings row styles are therefore unverified and the R1 geometry is design inference.
- No source found for the Spotify exact search placeholder text, the playlist-name dialog title, destructive-action dialogs or Android/iOS dialog styling.
- Nothing found for Spotify analogues of the first-run Wi-Fi join, clock setting, quick panel, power menu, low-battery countdown or screensaver (R1-only); adaptations are recorded as 'none (R1-only)' with the closest Spotify pattern.

## settings

### Settings entry point and root list (SET-ENTRY-GEAR, SHELL-TOOLS, SET-ROOT)

- Audit IDs: n/a
- Research date 2026-11-02; context: Spotify Mobile, Android and iOS, current as of 2026-11 (support pages show (c) 2026 but carry no page-level version or date). SlashGear Android guide is undated in the fetched text. No experiment information found. Premium/Free layouts differ only by a 'Go Premium' button on Android (SlashGear).
- Sources:
  - [How To Open And Adjust Your Spotify Settings On Android (SlashGear)](https://www.slashgear.com/1965140/how-to-open-spotify-settings-on-android), published undated in fetched text, accessed 2026-11-02, read
  - [Internet and data usage (Spotify Support)](https://support.spotify.com/us/article/internet-and-data-usage/), published undated; (c) 2026, accessed 2026-11-02, read
  - [How to log out (Spotify Support)](https://support.spotify.com/sa-en/article/how-to-log-out/), published undated, accessed 2026-11-02, read
  - [Community thread on the profile side panel (What's New, Listening history, Settings and privacy)](https://community.spotify.com/t5/Content-Questions/Spotify-Bell-Icon-for-New-Releases-Gone/m-p/5653010/highlight/true), published unknown (about late 2023 per search summary), accessed 2026-11-02, snippet
- Observed appearance: Documented structure only; no screenshot was viewed, so pixel-level styling is not verified. Entry: tap the profile picture at the top (upper-left on Android per SlashGear) from Home or Your Library, then the 'Settings and privacy' item, described by Spotify as a gear-style icon. The Settings page is one vertical list of section entries, in this Android order: Account, Content and display, Playback, Privacy and social, Notifications, Apps and devices, Data-saving and offline, Media quality, Advertisements, About and support. A 'Go Premium' button sits at the top and Log out is at the very bottom. The sources do not state section header treatment, row height, or chevron styling.
- Observed behavior: The avatar opens a profile side panel (What's New, Listening history, Settings and privacy) and 'Settings and privacy' pushes a Settings page. Each section row pushes a sub-page (for example Playback has Crossfade and Equalizer). Back returns to the previous page. Spotify has NO tools or launcher grid: no equivalent of the R1 vertical-ellipsis Tools tile page. Music, wireless, audiobooks and more are Library or Search content in Spotify, and device features live under Apps and devices. Settings is one page reached from the profile, not from a tool grid.
- Evidence quality: documented
- Conflicts / confidence: Section names and order agree between Spotify support (Content and display, Data-saving and offline, Apps and devices, Playback) and SlashGear (full list). SlashGear's ordering is a third-party source, so medium confidence. Exact visuals unverified (low). The tab where the avatar appears differs: Spotify says 'top of the screen', SlashGear says Home or Your Library.
- User requirement overrides: none recorded
- R1 adaptation: Replace the legacy gear to settings_screen route and the ellipsis to tools_screen route with one profile/gear entry that pushes a Spotify-style Settings page, and delete the gridpage Tools launcher as a reachable fallback. The tools it held are re-homed as Settings rows or Library entries (music and audiobooks under Library, wireless under 'Connectivity' in Settings). Layout at 480x800: #121212 page, 56 px header (back chevron at 44x44 hit area at x=8, title font_ui_20 bold white, centred or left aligned consistently with other pushed pages), then a scrolling list. Section headings use font_ui_16 in #b3b3b3 with 24 px top margin. Rows are 64 px tall with 16 px side gutters: title font_ui_18 white, optional value or subtitle font_ui_16 #b3b3b3, and a trailing chevron (#b3b3b3) or control. Regroup the R1 root to Spotify-style names: Content and display (Appearance, Screen, Language), Playback (Keys, Control centre), Apps and devices (Keyboard, Bluetooth/Wi-Fi), Data and storage (Power, System), About, with Developer options appended only when unlocked. Keep the mini-player and bottom tab bar visible on the Settings root only if the shell does so for other pushed pages. Disclosure: this regrouping is an adaptation and is not a claim that Spotify has those section contents.
- Acceptance criteria:
  - From Home and Search the single header profile/gear control opens a Spotify-style Settings page whose bounds, background (#121212), header height 56 px and row height 64 px can be asserted live
  - No route from the Spotify shell reaches tools_screen or the legacy gridpage; grep of callbacks shows no switch_screen(tools_screen) from main_menu.c
  - Every settings row is at least 44 px tall and 64 px visible, with title font_ui_18 and secondary font_ui_16 #b3b3b3
  - Back from any sub-page returns to the Settings root with scroll position preserved and to the originating tab after leaving Settings
  - Developer options row is absent until unlocked and present afterwards, in the same row style

### Settings rows, toggles, sliders and pill groups (SET-APPEARANCE, SET-DISPLAY, SET-KEYS, SET-POWER, SET-KEYBOARD pills, SET-DEVOPTIONS)

- Audit IDs: n/a
- Research date 2026-11-02; context: Spotify Mobile Android and iOS, current as of 2026-11. Support pages say 'row with an on/off toggle' for Content and display and 'Playback toggles' and a crossfade 'slider'. No screenshots viewed; styling unverified.
- Sources:
  - [Video settings (Spotify Support): Videos and Canvas toggles](https://support.spotify.com/us/article/video-settings/), published undated, accessed 2026-11-02, read
  - [Transitions between tracks (Spotify Support): Playback screen layout](https://support.spotify.com/us/article/tracks-transitions/), published undated, accessed 2026-11-02, read
  - [Audio quality (Spotify Support): Wi-Fi and cellular streaming, Auto-adjust toggle](https://support.spotify.com/us/article/audio-quality/), published undated, accessed 2026-11-02, read
  - [Make Spotify use less mobile data (Guiding Tech)](https://www.guidingtech.com/make-spotify-use-less-mobile-data/), published 2024-08-24, accessed 2026-11-02, read
  - [Creating coherence: how Spotify's design system goes beyond platforms (Figma blog, Encore)](https://www.figma.com/blog/creating-coherence-how-spotifys-design-system-goes-beyond-platforms/), published 2023-08-31, accessed 2026-11-02, read
- Observed appearance: Documented: Content and display shows each setting as a row with an on/off toggle (Music videos, Other videos, Canvas), with a section heading 'Videos and Canvas'. Playback shows a Crossfade heading with a slider at the top and a 'Track transitions' section below with Automix and Gapless playback toggles. Data-saving and offline holds Data saver, audio-only downloads and audio-only streaming toggles. Audio quality has Wi-Fi streaming and Mobile streaming option rows plus an Auto-adjust toggle. The one-line row description text under toggles was not confirmed. Switch geometry, colours, and slider thumb size are NOT verified from any read source (Encore article covers buttons and tokens only, not switches).
- Observed behavior: Toggles apply immediately with no Save or Apply button. Section headings group rows. Sliders are used where a quantity has a range (Crossfade seconds). Option sets (audio quality) open a selection list rather than showing in-line pills. Some settings are disabled while using Spotify Connect (EQ, crossfade, transitions, from Spotify's own notes). The Encore system unifies buttons into primary, secondary and tertiary hierarchy with shared sizing, supporting one consistent control kit across screens.
- Evidence quality: documented
- Conflicts / confidence: Structure: high (Spotify support pages). Visual metrics: unavailable. Spotify rows use selection lists, not segmented pill groups, for choice settings, so the R1 pill groups (theme, clock position, text size) are a deliberate adaptation, medium confidence on equivalence.
- User requirement overrides: none recorded
- R1 adaptation: One shared row component: 64 px high, 16 px gutters, title font_ui_18 white, subtitle font_ui_16 #b3b3b3 below (row grows to 80 px when subtitle present). Toggle: 52x32 px track, off #535353 with white-ish thumb, on #1ed760 with #121212 thumb, hit target the whole row (min 44 px). Slider rows: label line plus value on the right (font_ui_16 #b3b3b3), 4 px track #535353 with #1ed760 fill and 20 px white thumb, drag area at least 44 px tall, row 88 px. Choice settings with 2 to 3 options (theme, clock position, text size, QWERTY/T9, Album/Pictures) stay as inline pill groups but restyled: 40 px high capsule, selected #1ed760 with #121212 label, unselected #282828 with white label, 8 px gap; for 4 or more options (accent circles, 38 time zones, languages) use a row that opens the shared bottom sheet. Section headings font_ui_16 #b3b3b3. Remove legacy accent colour usage except #1ed760 for selection. All rows commit immediately and show a snackbar only when the effect is not visible.
- Acceptance criteria:
  - Live bounds: every row on Appearance, Screen, Keys, Power, Keyboard and Developer options is >= 44 px and consistently 64 or 80 px, with 16 px side gutters
  - Toggles are 52x32 with specified on/off colours, sampled from the render
  - Sliders have a drag area >= 44 px and persist the value on release; value label updates during drag
  - No legacy topbar, theme_style default, or accent colours appear on any Settings sub-page render
  - Toggling every switch persists across restart (config round trip)

### Language selection (SET-LANGUAGE)

- Audit IDs: n/a
- Research date 2026-11-02; context: Spotify Mobile iOS and Android 13+, current 2026. Android 12 and older: OS language only. No version/date on page; (c) 2026.
- Sources:
  - [How to change the app language (Spotify Support)](https://support.spotify.com/article/change-language/), published undated; (c) 2026, accessed 2026-11-02, read
- Observed appearance: Not described by the source. Path: profile picture, Settings and privacy, Content and display, App language, then choose a language. On iOS the picker is reached from this path; the exact picker presentation (list or OS settings hand-off) is not described by the read source and is NOT verified.
- Observed behavior: Language lives under Content and display, not at the Settings root. Android 12 and older delegates to system settings, warning it changes the entire device. Web uses profile, Settings, Language, then log out and in.
- Evidence quality: documented
- Conflicts / confidence: Path: high. Picker visuals: unavailable. The prompt's claim that iOS opens OS settings was NOT confirmed by the page (the page lists the in-app path for iOS).
- User requirement overrides: none recorded
- R1 adaptation: Place Language under Content and display. Open it as a pushed page (not a sheet, since the list can exceed 10 entries) with the same header and rows: one 64 px row per .ini language, native language name in font_ui_18, trailing green check (#1ed760, 24 px) on the current one, whole row tappable. Tap applies immediately and rebuilds visible strings, with the check moving; no Confirm. Keep the list bounded and scroll-capable.
- Acceptance criteria:
  - Language opens as a Spotify-style pushed page with 64 px rows and exactly one check mark
  - Selecting a language updates the header title and Settings root labels on return without restart, and persists after restart
  - List with more than 8 languages scrolls and does not overlap the header

### First-boot welcome, language panel and setup (LANG-FIRSTBOOT)

- Audit IDs: n/a
- Research date 2026-11-02; context: Spotify Mobile sign-up/first run, Android and iOS. Only 2018 newsroom text and community reports surfaced; no current dated source was read.
- Sources:
  - [5 Things Free Users Need to Be Taking Advantage of on Spotify (Spotify Newsroom, taste onboarding)](https://newsroom.spotify.com/2018-08-27/5-things-free-users-need-to-be-taking-advantage-of-on-spotify/), published 2018-08-27, accessed 2026-11-02, snippet
  - [Spotify's New User Onboarding Feature product teardown (Uxcel)](https://app.uxcel.com/showcase/spotifys-new-user-onboarding-feature-product-teardown-222), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Unverified. Snippet-level only: after sign-in a full-screen picker asks to choose several artists (5 in 2018 newsroom text, 3 in newer unofficial references). The Uxcel page returned only a tagline, so no teardown was read. No visual evidence is recorded.
- Observed behavior: Spotify's first run is a full-screen, single-purpose step flow with a primary action and a skip path. This is a shape-level inference from snippets, not verified behaviour. Spotify does not have a Wi-Fi or clock setup step.
- Evidence quality: unavailable
- Conflicts / confidence: Artist-count threshold conflicts (5 in 2018 vs 3 in later references) and is not relevant to R1. Overall confidence low; do not cite as verified.
- User requirement overrides: none recorded
- R1 adaptation: R1-only flow, so use the shared visual language: #121212 full-screen steps, a centred step title in font_ui_28 bold, supporting text font_ui_16 #b3b3b3, and a bottom-anchored full-width pill primary button (56 px high, 24 px bottom margin, #1ed760 with #121212 label font_ui_18). Language step uses the same 64 px language rows as Settings with a check, and Continue is disabled until a language is selected (a default is preselected so it is enabled). Wi-Fi setup step uses the shared network rows and keyboard. Clock step reuses the Date and time controls restyled. Add a text 'Skip' tertiary button for Wi-Fi and clock steps. Step order and content preserved.
- Acceptance criteria:
  - Each first-boot step renders with #121212 background, a bottom pill primary button at 56 px, and no legacy full-screen language panel styling
  - Language preselected and applied live on the first step; flow can be completed with touch only
  - Skip on Wi-Fi and clock leaves defaults and still reaches Home

### Option choosers and bottom sheets: Remap action chooser, Double-click dialog, Time zone list, Keyboard layout (SET-REMAP-CHOOSER, SET-REMAP-DOUBLE, SET-DATETIME-TZ, SET-KBLAYOUT)

- Audit IDs: n/a
- Research date 2026-11-02; context: Spotify Mobile Android and iOS, sleep timer and device picker sheets, current as of 2025-2026. Sleep timer menu documented 2021 and 2022 and cross-device in 2025. Option screens were described by text only.
- Sources:
  - [How to set up a sleep timer in Spotify (How-To Geek)](https://www.howtogeek.com/749269/how-to-set-up-a-sleep-timer-in-spotify/), published 2021-09-28, accessed 2026-11-02, read
  - [Sleep timer cross-device update (Spotify Newsroom tag page; update dated 2025-08-29 on a 2022-08-02 post)](https://newsroom.spotify.com/tag/sleep), published 2025-08-29, accessed 2026-11-02, read
  - [How to set Spotify sleep timer (Guiding Tech)](https://www.guidingtech.com/how-to-set-spotify-sleep-timer/), published 2024-05-20, accessed 2026-11-02, read
  - [Spotify Connect (Spotify Support)](https://support.spotify.com/us/article/spotify-connect), published undated, accessed 2026-11-02, read
- Observed appearance: Documented by text: the sleep timer is a list titled 'Stop Audio In' with options 5, 10, 15, 30, 45 minutes, 1 hour and 'End of Track', opened from the three-dot menu on Now Playing (scroll to the bottom); the remaining time appears beside the Sleep Timer entry, and a 'Turn off timer' option cancels. A confirmation reads 'OK, your sleep timer is set.' The Connect device picker is opened by the device icon on the Now Playing bar and lists devices (Linn doc, older, says 'Devices Available'); picker layout is not described. Sheet height, corner radius, and drag handle are unverified from read sources.
- Observed behavior: Single-choice option list: tap an item to apply and dismiss; the current state is shown on the entry that opens the sheet; one confirmation snackbar or toast acknowledges the change. Choices are not committed with a separate Done. Time-zone style long lists (38 items) have no direct Spotify analogue; use the same single-choice list inside a scrolling sheet or page.
- Evidence quality: documented
- Conflicts / confidence: Durations agree between How-To Geek (2021) and Spotify newsroom (2022 original, 2025 update). Guiding Tech lacks the option list. Sheet visuals unverified (low). Counterpart for the Remap anchored popover is a bottom sheet, which is an intentional change from the legacy anchored popover.
- User requirement overrides: none recorded
- R1 adaptation: Use the shared gui/shell/bottomsheet.c for all choosers: #202124 sheet, 16 px top corner radius, 36x4 drag handle, title font_ui_20 bold, rows 56 px (title font_ui_18, trailing check #1ed760 for current), max height 640 px with an internal scroll for long lists (time zones: 38 rows, offset and places in font_ui_16 #b3b3b3 as a subtitle, so rows 72 px), scrim 60 percent black, tap outside or drag down to dismiss. Remap chooser: seven rows (six actions plus None), tap saves and dismisses. Double-click dialog becomes a sheet with two chip groups (button pills, action pills) each 40 px high capsules, saving on each tap with no Done, close by dismiss. Keyboard layout in-use/others reorder is an edit list (see Control centre record). Show a snackbar such as 'Action set' only if not visible in the underlying row.
- Acceptance criteria:
  - Remap chooser, Double-click, and Time zone render using bottomsheet.c with #202124 and a drag handle; no anchored popover or centred legacy dialog remains
  - Current choice shows a green check; tap saves, closes in about 220 ms (UI_SHEET_TRANSITION_MS), and the opener row reflects the new value
  - Time zone sheet scrolls 38 rows without exceeding 640 px or covering the status area; back or scrim dismiss without changes
  - Opening and closing 10 times while audio plays causes no audio gap and no widget leak (object count stable)

### Control centre and Keyboard layout drag-reorder edit lists (SET-CC, SET-KBLAYOUT)

- Audit IDs: n/a
- Research date 2026-11-02; context: Spotify Mobile playlist 'Edit' and Your Library reorder lists, Android and iOS. No source on drag-handle styling was found or read.
- Sources:
  - [Spotify Support home (searched; no article on edit-list reorder found)](https://support.spotify.com/), published n/a, accessed 2026-11-02, snippet
- Observed appearance: Unavailable. No dated mobile evidence of the drag-handle edit list was found. Described only as the commonly known pattern of rows with a trailing grip, which is inference.
- Observed behavior: Not verified. Inferred: edit mode lists rows with grips; dragging reorders; entering and leaving edit mode is explicit. R1's 'In use' versus 'Not in use' is R1-only.
- Evidence quality: inferred
- Conflicts / confidence: Low. Treat the Spotify counterpart as unresearched and record in the audit as a gap until a dated source is found.
- User requirement overrides: none recorded
- R1 adaptation: Keep the two-group model but restyle with the shared row: section headings 'In use (n/QP_SLOT_COUNT)' and 'Not in use' in font_ui_16 #b3b3b3; rows 64 px with a leading icon (24 px), title font_ui_18, and a trailing 3-line grip glyph in #b3b3b3 inside a 56x56 hit area. Dragging lifts the row (#282828 raised with shadow-less 1 px #3e3e3e border), auto-scrolls near the edges, and drops with a 150 ms settle. Add an accessible alternative: long-press-free tap on the row opens a sheet with 'Move up', 'Move down', 'Move to In use / Not in use'. First in-use keyboard layout shows a 'Default' subtitle in #1ed760 font_ui_16. Full list bounded (< 40 rows).
- Acceptance criteria:
  - Control centre and Keyboard layout pages use the shared 64 px row, 56x56 grip target and section headings
  - Drag across groups updates the counts n/QP_SLOT_COUNT and persists across restart; cannot exceed QP_SLOT_COUNT
  - Default layout marker follows the first in-use item after reorder
  - Page performs drag at a stable frame time with all rows present (host timing only; R1 timing unmeasured)

### Storage, cache and destructive confirmations: Factory reset, notices and dialogs (SET-FACTORY, SET-SYSTEM, SET-NOTICE-NO-PICTURES, SET-DATETIME-WIFI)

- Audit IDs: n/a
- Research date 2026-11-02; context: Spotify Mobile Android and iOS, current as of 2026. The support pages for Clear cache and Log out do NOT describe a confirmation dialog. Delete playlist confirmation is from third-party guides and community reports (low quality).
- Sources:
  - [Spotify storage information (Spotify Support)](https://support.spotify.com/article/storage-information/), published undated, accessed 2026-11-02, read
  - [How to log out (Spotify Support)](https://support.spotify.com/sa-en/article/how-to-log-out/), published undated, accessed 2026-11-02, read
  - [How to delete Spotify playlists (Business Insider NL)](https://businessinsider.nl/how-to-delete-spotify-playlists-and-clean-up-your-streaming-library-from-the-desktop-or-mobile-app/), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Documented: Storage lives under Settings and privacy, Data-saving and offline, Storage; a 'Clear' action sits next to 'Clear cache'; Android can move downloads to an SD card via a Storage location row and a Continue action. Log out is the last item of the Settings page. No dialog visuals were read. Delete confirmations are said (third-party, snippet only) to end in a second 'Delete' tap in a prompt, with a Cancel option not confirmed.
- Observed behavior: Clear cache and Log out are single-tap per Spotify's pages with no mentioned confirmation (low-risk operations). Destructive deletion of user content uses a confirmation prompt. Factory reset on R1 is irreversible config deletion plus reboot, which warrants a stronger pattern than Spotify's Clear cache. Spotify does not have a no-network clock sync flow; its offline banners are the only analogue. Dialog wording conventions are unverified.
- Evidence quality: documented
- Conflicts / confidence: Storage and log out paths: high. Dialog layout: low (not read). Spotify's single-tap behaviour for Clear cache is documented, but R1's Factory reset is far more destructive, so the contract is a deliberate adaptation, not a Spotify copy.
- User requirement overrides: none recorded
- R1 adaptation: Shared centred dialog (or bottom-anchored sheet for Factory reset) in #282828 on a 60 percent scrim: width 400 px, 24 px padding, title font_ui_20 bold, message font_ui_16 #b3b3b3 (max 4 lines), two full-width stacked pill buttons 52 px high: primary and Cancel text button. Factory reset keeps two steps, restyled: step 1 'Reset device?' with 'Continue' (white pill) and 'Cancel'; step 2 'Erase settings and reboot' with a red (#f3727f) destructive pill 'Reset' plus 'Cancel'; state that music files are NOT deleted. No-pictures notice: single OK pill and title 'No screensaver pictures', message naming the folder, source reverts to Album afterwards. Wi-Fi enable confirmation uses 'Turn on Wi-Fi?' with 'Turn on' and 'Not now'. Sync status uses an inline subtitle on the Time-zone/Automatic row plus a bottom snackbar (48 px high, #282828, 16 px side margin, above the mini-player) for synced or failed. Move Factory reset into a 'Danger zone' heading at the bottom of System with the label in #f3727f.
- Acceptance criteria:
  - All dialogs share one component: #282828, 400 px width, pill buttons >= 52 px; none uses legacy button styles
  - Factory reset needs two explicit confirmations and never deletes audio files (verified on a fixture card)
  - Cancel at either step leaves audio playing and config unchanged
  - Dialogs dismissed via back key or scrim return to the previous page with focus preserved
  - Snackbar never overlaps the mini-player or the bottom tab bar

### About page, version, build-number developer unlock (SET-ABOUT, SET-SYSTEM)

- Audit IDs: n/a
- Research date 2026-11-02; context: Spotify Mobile Android and iOS. 'Third-party software' wording is from legal text and community posts; the mobile About menu labels were not confirmed in a current source.
- Sources:
  - [SlashGear Android settings guide (About and support section listed)](https://www.slashgear.com/1965140/how-to-open-spotify-settings-on-android), published undated, accessed 2026-11-02, read
  - [Community thread on Open source licenses in the Android app](https://community.spotify.com/t5/Android/Open-source-licenses-not-available-in-Android-app/m-p/4657356/highlight/true), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Documented: an 'About and support' section exists on Android and shows app version, privacy policy, and terms (SlashGear). Third-party software or licenses page exists (community snippets). No visuals read.
- Observed behavior: About lists the app version and legal links as ordinary rows that push pages or open external pages. The build-number tap-to-unlock developer options is an Android OS convention and not a Spotify feature.
- Evidence quality: documented
- Conflicts / confidence: Existence of About: medium. Detail rows: low. The developer unlock is explicitly an R1 / Android system convention.
- User requirement overrides: none recorded
- R1 adaptation: About is a pushed page of 64 px rows: label font_ui_16 #b3b3b3 above value font_ui_18 white (Device, DAC, Serial, OS version, Build number). The SD usage bar is a card: #282828 radius 12, 6 px bar #1ed760 on #535353, caption 'x GB used of y GB' font_ui_16. Build number taps show a snackbar countdown ('3 more taps to enable developer options') rather than modal popups; unlock confirmed with 'You are now a developer'. Rows have no chevron except where they open pages. Easter egg on OS version stays but must not break layout.
- Acceptance criteria:
  - About rows follow label above value with 64 px height; no row truncates the serial or build text (ellipsis handled)
  - Five taps on Build number unlock Developer options with snackbar countdown that never stacks more than one snackbar
  - SD bar matches used/total from the storage API and never exceeds 100 percent

### Connectivity, Bluetooth and device-related settings reached from Date and time automatic sync (SET-DATETIME, SET-DATETIME-WIFI) and Apps and devices analogue

- Audit IDs: n/a
- Research date 2026-11-02; context: Spotify Mobile Apps and devices settings, Android and iOS, 2026. Local audio files toggle and Local device visibility are documented. Spotify has no date or time setting.
- Sources:
  - [Local files (Spotify Support)](https://support.spotify.com/us/article/Listen-to-local-files), published undated, accessed 2026-11-02, read
  - [Spotify Connect (Spotify Support): Local device visibility](https://support.spotify.com/us/article/spotify-connect), published undated, accessed 2026-11-02, read
- Observed appearance: Documented: Apps and devices holds a 'Local audio files' switch (profile, Settings and privacy, Apps and devices) which makes a 'Local Files' section appear in Your Library, and 'Local device visibility'. No panel styling read. Spotify has no clock, time zone or date roller.
- Observed behavior: Apps and devices rows are switches with immediate effect. Spotify does not own the date, so the R1 date/time panel is OS-pattern, R1-only. Local Files become a Library entry rather than a Settings page, which matters because R1 is local-only.
- Evidence quality: documented
- Conflicts / confidence: Local files toggle path: high on iOS and Android (same steps). Premium requirement for local files on mobile: unconfirmed by the page. Date/time: none (R1-only), adaptation is inferred.
- User requirement overrides: none recorded
- R1 adaptation: Replace the top-layer full-screen Date and time panel with a pushed settings page (not a modal layer): header 'Date and time', a two-pill segmented control Manual/Automatic (40 px capsules), then rows: Date (value right-aligned), Time, Time zone (opens the Time-zone sheet), Daylight saving (pills), 24-hour clock (switch). Date and Time rows open a bottom sheet with a roller (3 columns, 56 px items, font_ui_24 centred, selected row in white, others #b3b3b3) and a full-width 'Save' pill. No cancel chevron: back = discard. Automatic mode shows a status row ('Waiting', 'Connected', 'Synced', 'Sync failed' with the failed label in #f3727f) and a 'Sync now' text button. The Wi-Fi-off confirmation follows the dialog contract above.
- Acceptance criteria:
  - Date and time is a pushed page with header and back, not a top-layer panel
  - Manual save applies the time and persists across restart; Back discards edits
  - Automatic mode with Wi-Fi off shows the confirmation dialog and then the status row transitions waiting to connected to synced or failed
  - Roller sheet items are >= 44 px and scroll with snapping

### Equalizer and Playback-style audio settings as the analogue for Power, Screen and Keys sliders (SET-POWER, SET-DISPLAY, SET-KEYS)

- Audit IDs: n/a
- Research date 2026-11-02; context: Spotify Mobile Playback settings, Android: Equalizer under Listening controls; iOS: Equalizer toggle in Playback. EQ and transitions are unavailable while using Spotify Connect.
- Sources:
  - [Equalizer (Spotify Support)](https://support.spotify.com/us/article/equalizer/), published undated, accessed 2026-11-02, read
  - [Transitions between tracks (Spotify Support)](https://support.spotify.com/us/article/tracks-transitions/), published undated, accessed 2026-11-02, read
- Observed appearance: Documented: the equalizer has an on/off toggle, a preset list and a graph with draggable dots for bass and treble. Crossfade is a slider at the top of Playback, followed by Track transitions toggles. Colours and sizes unverified.
- Observed behavior: Toggle enables the feature; presets are chosen from a list; direct manipulation on a graph writes immediately. Dependent controls are disabled (not hidden) when a precondition is not met (Connect). Notes explain constraints.
- Evidence quality: documented
- Conflicts / confidence: High for structure, none for visuals. Spotify has no brightness, charge limit, LED or standby controls, so R1 settings are R1-only using the Playback visual language.
- User requirement overrides: none recorded
- R1 adaptation: Group the R1 Power page into section headings: 'Battery' (Charge limit, 80 percent / 500 mA toggles on R1 PMIC), 'Standby' (Standby memory toggle plus slider, LED on, LED off in standby, Lock keys when screen off), 'Auto power off' (toggle plus slider). A slider row appears only under its enabling toggle and is disabled and dimmed (50 percent opacity, still visible) when the toggle is off. Notes use font_ui_16 #b3b3b3 below the group, max 3 lines. Screen page: brightness slider with a sun glyph at each end, 'Screen off' slider with a value in seconds, Screensaver switch with an Album/Pictures pill row that is disabled when off, Rotate 180 switch (with a short confirm on change). Keys page: In-line remote switch, 'Remap buttons' row with chevron, 'Volume step' slider, 'Screenshot' switch with the key hint in the subtitle.
- Acceptance criteria:
  - Power, Screen and Keys pages contain only shared rows, switches, sliders and pills with documented sizes
  - Dependent sliders and pills are dimmed and non-interactive when their parent switch is off
  - All values persist across restart and take effect at once (brightness changes the real backlight on interaction)
  - Pages with more than 8 rows scroll and show the last row fully above the bottom edge or mini-player

### Boot logo, Remap device photo, Processes and Developer options (R1-only: SET-BOOTLOGO, SET-REMAP, SET-DEVOPTIONS, SET-PROCESSES)

- Audit IDs: n/a
- Research date 2026-11-02; context: None (R1-only). No Spotify Mobile equivalent exists. Closest pattern: a pushed Settings sub-page with rows, plus a Library-style grid for picture selection.
- Sources:
  - [SlashGear Android settings guide (closest pattern: list-of-rows sub-pages)](https://www.slashgear.com/1965140/how-to-open-spotify-settings-on-android), published undated, accessed 2026-11-02, read
- Observed appearance: Not applicable. Use the same pushed-page language as the other Settings pages. The Remap device photo is bespoke R1 hardware art, kept inside a #282828 card with leader lines.
- Observed behavior: Not applicable to Spotify. Long-running work (boot-logo conversion, process listing) must not block the UI thread and must show progress (R1 constraint, not Spotify behaviour). Developer options are hidden until unlocked.
- Evidence quality: unavailable
- Conflicts / confidence: Not applicable. Record explicitly as 'none (R1-only)'.
- User requirement overrides: none recorded
- R1 adaptation: Boot logo: Default as a 64 px row with a check; pictures in a 3-column grid of 144x144 px thumbnails (radius 8, 8 px gaps), up to 48 items loaded asynchronously with placeholders (bounded cache of about 24 thumbs); tapping shows a bottom sheet 'Use this boot logo?' with 'Use' and 'Cancel', then a progress snackbar while the worker thread converts, with 'Done' or 'Failed'. Empty state: centred icon, title 'No pictures found', message naming <card>/Bootlogo. Remap: device photo card at the top (max 360 px wide), touch zones at least 44x44 with leader lines, rows below each as 64 px rows with the current action as a trailing value; tapping opens the chooser sheet. Developer options: standard switches (ADB with a 700 ms poll showing a spinner until confirmed, Log to microSD with the path as subtitle, Database log, Bluetooth log) and a Processes row. Processes: a RAM summary card (used, free, player heap) with a 4 px bar, then up to 60 rows of 48 px (name font_ui_16, RSS right-aligned) in a recycled list with a refresh icon button (44x44) in the header.
- Acceptance criteria:
  - Each page uses the shared header, 64 px rows or documented grid; no legacy topbar or title
  - Boot-logo conversion keeps the UI responsive (spinner animates) and audio uninterrupted
  - Empty and error states exist for Boot logo and Processes and use the shared pattern
  - Processes list renders at most 60 bounded rows and refresh does not leak objects across 20 refreshes

### Screensaver overlay (SET-SCREENSAVER-OVERLAY)

- Audit IDs: n/a
- Research date 2026-11-02; context: Closest Spotify analogue: OS lock-screen media controls and the Now Playing full-screen artwork on Spotify Mobile. No Spotify source was read about the lock-screen presentation.
- Sources:
  - [Internet and data usage (Spotify Support): Canvas and Now Playing visuals under Content and display](https://support.spotify.com/us/article/internet-and-data-usage/), published undated, accessed 2026-11-02, read
- Observed appearance: Documented only that Now Playing visuals (Canvas looping visuals) are a Content and display option. Lock-screen layout: unavailable. The overlay remains R1-only.
- Observed behavior: Spotify itself does not implement a screensaver; the OS draws the lock screen. Therefore none (R1-only). Interaction on R1 is swipe up to dismiss.
- Evidence quality: unavailable
- Conflicts / confidence: Not applicable. No claim of Spotify parity.
- User requirement overrides: none recorded
- R1 adaptation: Full-bleed artwork (480x800, cover-fit with a 40 percent bottom gradient), centred bottom block: track title font_ui_24 bold white, artist font_ui_18 #b3b3b3, clock font_ui_28 white at the top centre, 24 px margins. A 4x48 px pill hint at the bottom edge signals swipe up. Use an opaque dark #121212 base behind letterboxed pictures (no real-time blur). Wake on any key or tap, dismiss on swipe up of at least 120 px. Hold the album artwork in a single bounded decoded buffer.
- Acceptance criteria:
  - Overlay shows artwork, track, artist and clock with legible contrast on both light and dark pictures (gradient scrim)
  - Swipe up dismisses and returns to the previous page without an audio gap
  - Pictures source with a missing folder falls back to Album after the notice dialog

Failed attempts:

- support.spotify.com/us/article/sleep-timer/, /playback-speed/, /clear-cache/ and /crossfade/ returned 404 pages; equivalent facts came from storage-information, tracks-transitions and third-party sleep timer guides (How-To Geek 2021-09-28, Spotify newsroom 2025-08-29)
- No dated screenshot-based review of the Spotify Android or iOS Settings page was found; all Spotify row, switch, slider and dialog visuals (heights, colours, radii) are UNVERIFIED and the R1 sizes are design choices
- Guiding Tech sleep timer article (2024-05-20) had empty screenshot placeholders and no option list
- Guiding Tech data saver article (2024-08-24) screenshots were placeholders, so layout was not confirmed
- Spotify Connect support page does not describe the picker layout and the Linn wiki only says 'Devices Available' bar; the Connect device sheet visuals are unverified
- Search for the Spotify mobile About page labels returned only legal pages and community snippets; Open source licenses wording unconfirmed, About page read from SlashGear only
- Delete playlist / destructive dialog wording: only snippets from third-party guides (Business Insider NL, others) were available and were not fetched, so dialog layout and button labels are unverified
- Onboarding: Uxcel teardown page returned only a tagline; 2018 newsroom page seen as a snippet only; current artist-count threshold (3 vs 5) unconfirmed
- No source was found on drag-handle edit lists (Control centre and Keyboard layout counterpart); recorded as inferred
- spotify.design / Encore pages for dialogs, switches and bottom sheets were not found; only the Figma blog (2023-08-31) on Encore buttons and tokens was read, which does not mention settings, dialogs, toggles or sheets
- YouTube video walkthroughs of Spotify mobile settings: the search returned no usable video pages, so no dated video evidence was recorded
- The routes' claim that iOS App language opens OS settings was not confirmed; Spotify's page lists the in-app path for iOS and OS-settings only for Android 12 and older
- Playback speed (podcasts and audiobooks): only community and blog snippets seen via search, not fetched; no official support page found, so no record was written beyond noting it is out of scope for this area's routes
- WebFetch cannot see images, so no visual appearance could be verified from any source

## musiclib

### Settings root, section groups and list rows (replaces the tools grid -> Music settings root, Library, Now Playing and DAC pages)

- Audit IDs: MUS-TOOLS-ENTRY, MSET-ROOT, MSET-LIBRARY, MSET-NOWPLAYING, MSET-EFFECTS-DAC
- Research date 2026-11-02; context: Android and iOS, Spotify mobile. Section order from SlashGear (Android, published 2025-09-14); support-page paths current in 2026 but undated. Free vs Premium: Go Premium banner at top of Settings; Crossfade, Normalize volume and Lossless are Premium-gated. No experiment or version number found. Accessed 2026-11-02.
- Sources:
  - [How To Open And Adjust Your Spotify Settings On Android (SlashGear)](https://www.slashgear.com/1965140/how-to-open-spotify-settings-on-android), published 2025-09-14, accessed 2026-11-02, read
  - [Equalizer - Spotify Support](https://support.spotify.com/us/article/equalizer/), published undated, accessed 2026-11-02, read
  - [Transitions between tracks - Spotify Support](https://support.spotify.com/us/article/tracks-transitions/), published undated, accessed 2026-11-02, read
  - [Audio quality - Spotify Support](https://support.spotify.com/us/article/audio-quality/), published undated, accessed 2026-11-02, read
  - [Spotify storage information - Spotify Support](https://support.spotify.com/us/article/storage-information/), published undated, accessed 2026-11-02, read
- Observed appearance: Settings is reached from the profile picture (upper-left on Home and Your Library) -> Settings and privacy; it is no longer a gear on Home (2019 Adweek guide described the old gear; community posts confirm the move). A single scrolling page starting with a Go Premium button, then ten section headings in this order: Account, Content and display, Playback, Privacy and social, Notifications, Apps and devices, Data-saving and offline, Media quality, Advertisements, About and support. Row types (toggle / value / chevron) are NOT described by any source I could read; the green switch, grey secondary value text and section-header styling are not documented by any text source I could read and would need a dated screenshot to confirm. Sub-grouping seen in text: Android Playback has a 'Listening controls' block (Equalizer switch) and a 'Volume controls' block (Volume normalization); the 'Track transitions' block holds Gapless playback and Automix; Crossfade has its own slider.
- Observed behavior: Settings are one flat vertical list with section headings, not a tile grid; there is no separate 'Music' hub. Playback settings (crossfade, EQ, normalization) are greyed out / unavailable while playing through Spotify Connect on another device. Navigation is push/pop with a back arrow. R1 consequence: the tools grid (Music, Wireless, Audiobooks, More, Settings) has no Spotify counterpart; Spotify routes everything through the profile menu -> Settings and privacy, and library content through the Your Library tab.
- Evidence quality: documented
- Conflicts / confidence: Section names and order: medium-high (one dated 2025 article plus support-page paths that agree: Playback, Apps and devices, Data-saving and offline, Media quality). Row-type visuals: LOW, no readable source. The Android 'Listening controls' vs iOS wording differ (iOS: 'Enable Audio Normalization', Android: 'Volume normalization'). Local files toggle label conflicts between sources, see the Local files record.
- User requirement overrides: None recorded. AGENTS.md requires the Spotify-style UI for every retained feature and forbids a legacy gridpage fallback; this supersedes keeping the tools grid.
- R1 adaptation: Replace the vertical-ellipsis tools grid and gear with ONE profile/gear entry in the Home and Library header that opens a single 'Settings' page (480x800, bg #121212). Header: 64 px tall, 48x48 back chevron at x=8, title font_ui_24 white centered-left. Body: scrolling list; section headers font_ui_16 #b3b3b3 bold, 40 px tall with 24 px top gap. Row: 64 px tall (72 px when it has a secondary line), 16 px side gutters, primary font_ui_18 white, secondary font_ui_16 #b3b3b3, whole row is the hit area (>=44). Toggle: 52x28 pill, #1ed760 on / #535353 off, knob white, placed right with the full row as hit area. Value rows show the value in #b3b3b3 + 20 px chevron. Pill-choice settings (Player layout, ReplayGain, Add-to-queue) become a row that opens a bottom sheet of radio rows (do not keep inline pills). Group the current Music settings into the Spotify section names: Playback (queue, sleep timer, crossfade, gapless, Remember track/volume), Audio (ReplayGain, EQ/PEQ/MSEB, bit-perfect, effects, volume, DAC), Library/Local files (scan options, album view, rearrange), Apps (Last.fm), Storage, About. DAC-only items sit under 'Audio' with a one-line explanatory secondary text because Spotify has no counterpart (document as R1-only adaptation). Dimmed rows (bit-perfect holds effects) use 40% opacity text plus a secondary line saying why.
- Acceptance criteria:
  - Home/Library header opens exactly one Settings page; no gridpage tile screen is reachable from the Spotify shell
  - Every music-settings page uses the shared #121212 page with 64 px header, back chevron >=48x48, section headers and 64/72 px rows
  - All toggles are 52x28 green switches whose entire row is tappable and keeps state across leaving and re-entering
  - Choice settings with 3+ options open a bottom sheet with radio rows instead of inline pills
  - No row text is clipped at 480 px width with font_ui_18 primary and font_ui_16 secondary; long values ellipsize
  - Disabled rows (bit-perfect, Connect-like unavailability) are dimmed and show a one-line reason

### Local files and library sources (scan folder picker, scan options, scan progress, empty or missing-card states)

- Audit IDs: SCAN-PICKER, SCAN-PROGRESS, MSET-SCANOPTS, MUS-CLASSIC-EMPTY-NOCARD, BRW-ROOT
- Research date 2026-11-02; context: Android and iOS Spotify mobile (support article, undated but current in 2026); desktop section is separate. Label conflicts across app versions. Community staff replies roughly 2021 to 2024 (snippets only). Accessed 2026-11-02.
- Sources:
  - [Local files - Spotify Support (global)](https://support.spotify.com/article/local-files), published undated, accessed 2026-11-02, read
  - [Local files - Spotify Support (US)](https://support.spotify.com/us/article/local-files/), published undated, accessed 2026-11-02, read
  - [Why are my local files not showing up? - Spotify Community](https://community.spotify.com/t5/Android/Why-are-my-local-files-not-showing-up/td-p/5448929), published c.2021-2022 (unconfirmed), accessed 2026-11-02, snippet
  - [Android Local Files - Spotify Community](https://community.spotify.com/t5/Android/Android-Local-Files/m-p/5851102/highlight/true), published unknown, accessed 2026-11-02, snippet
  - [Local files on iOS - Spotify Community FAQ](https://community.spotify.com/t5/FAQs/Local-File-Discovery-iOS/ta-p/5384985/highlight/true), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Mobile has no folder picker. The setting is a single switch: profile picture -> Settings and privacy -> Apps and devices -> 'Local audio files' (support article, both OS). Search results and community staff replies also show the label 'Show audio files from this device' under a 'Local Files' settings section (Android), and older 'Show local audio files' under Import. Desktop (for contrast) has 'Show Local Files' plus a 'Show songs from' list of folder toggles and an 'Add a source' button. Result in the app: a 'Local Files' entry appears in Your Library (reached via the Playlists filter on some versions), shown as a normal playlist row.
- Observed behavior: Toggle on -> the app indexes playable audio automatically; moderators say mobile scans the whole device storage (SNIPPET ONLY, not verified). Files must be playable on the device and (per community) over about 20 seconds. Local files play only on the device holding them (not over Connect). Crossfade between local files works only when sample rate and channel count match (read on the support page). The support page gives NO scan progress UI, NO empty state wording and NO folder picker for mobile, and community advice for missing files is 'turn it off and on again' (snippet only). Therefore progress, empty and no-card states have no Spotify visual contract; they must be built from generic Spotify patterns (empty-state illustration/headline/CTA, snackbars).
- Evidence quality: documented
- Conflicts / confidence: Toggle label conflict: support page says 'Local audio files' (Apps and devices); search/snippet sources say 'Show audio files from this device' (Local Files section). Treat both as app-version variants; confidence high that it is one toggle plus automatic indexing, LOW on scan scope and empty-state text.
- User requirement overrides: Product is local-only; the Local files area is the primary content source on R1, not a secondary setting. Folder selection and scan options are R1 adaptations beyond Spotify mobile.
- R1 adaptation: Create a 'Local files' settings section (under Settings, and reachable from the empty Library state): first row 'Show local audio files' with the 52x28 green switch (turning it off hides the Local files collection but never deletes audio files; say so in the secondary line). Second row 'Music folders' (font_ui_18, secondary line '3 of 5 selected' font_ui_16 #b3b3b3, chevron) opens the folder picker as a FULL-HEIGHT bottom sheet (#202124, 20 px top radius, 4x40 grabber, header 'Choose music folders' font_ui_20) with 64 px checkbox rows (24 px green checkbox at left, folder name, track-count secondary) and a sticky footer holding a full-width 56 px #1ed760 pill 'Scan' (black text font_ui_18) and a text 'Cancel'. Replace the centred card over the 60% veil. Scan progress becomes a full page: 64 px header 'Scanning', a large font_ui_28 live count '1,248 tracks', a 4 px green linear bar (indeterminate unless total known), a secondary line with the current folder, and a 56 px outlined 'Cancel' pill (red text only in the confirm dialog, not as a bright red button). Empty Library (no index): centred 64 px icon, font_ui_24 headline 'Add your music', font_ui_16 #b3b3b3 sentence, 56 px green pill 'Scan music'. Missing or USB-shared card: same layout with headline 'SD card not available' and a secondary pill 'Open settings'; USB-shared copy says the card is in use by the computer. The filesystem browser is surfaced as the 'Local files' folder view (back chevron, breadcrumb title, 56 px rows) because Spotify's Local Files list is the closest analogue.
- Acceptance criteria:
  - Scan folder selection opens as a bottom sheet with 64 px checkbox rows; tapping the veil or Cancel dismisses without scanning
  - Scan page shows a monotonically increasing track count updated at most 5x per second and a Cancel that asks for confirmation before aborting
  - Empty index and missing-card states each show icon, headline, one explanatory sentence and exactly one primary green pill
  - Disabling local files never deletes audio files and the secondary line states this
  - Returning from the scan page lands on the tab the user started from

### Your Library filters, sorting and view options (Music grid with reorderable tiles, Rearrange main page, album view)

- Audit IDs: MUS-CLASSIC-GRID, MSET-TILES, MSET-LIBRARY
- Research date 2026-11-02; context: Android and iOS, redesign rolled out from 2021-04-29. Sort labels reported by community volunteers for that era (snippet only). Pinning up to four items; Downloaded filter is Premium. No 2025/2026 official description was found, so current chip order is unverified. Accessed 2026-11-02.
- Sources:
  - [Spotify launches redesigned Your Library on iOS and Android (9to5Mac)](https://9to5mac.com/2021/04/29/spotify-launches-redesigned-your-library-ios-android/), published 2021-04-29, accessed 2026-11-02, read
  - [Spotify Redesigns 'Your Library' App Tab (iPhone in Canada)](https://www.iphoneincanada.ca/news/spotify-your-library/), published 2021-04-29, accessed 2026-11-02, read
  - [Spotify introduces mobile library redesign (XDA)](https://www.xda-developers.com/spotify-library-redesign-improved-navigation/), published 2021-04-29, accessed 2026-11-02, read
  - [Your Library - Spotify Support](https://support.spotify.com/us/article/your-library/), published undated, accessed 2026-11-02, read
  - [New Your Library view (iOS & Android) - Spotify Community](https://community.spotify.com/t5/Your-Library/New-Your-Library-view-iOS-amp-Android/m-p/5200807/highlight/true), published 2021, accessed 2026-11-02, snippet
  - [Design Critique: Spotify (iOS app) - Pratt IxD](https://ixd.prattsi.org/2019/09/spotify-design-critique/), published 2019-09-08, accessed 2026-11-02, read
- Observed appearance: Your Library header: profile picture, a search icon (one search covering the whole library) and a create (+) action. Under it a horizontal row of filter chips: albums, artists, playlists, podcasts, and a Downloaded chip (Premium); selecting a chip hides the others and shows a clear (x). Items are listed as 72-ish px rows with artwork thumbnail, title and secondary line (type and creator), or as a grid of large cover tiles. A sort control (text button with arrows) shows the current sort. Pinned items sit first with a pin glyph. Exact chip colours/heights are not documented by any text source I read.
- Observed behavior: Tapping a chip filters in place and keeps scroll at the top; a second tap clears it. Sort options (community, snippet only): Recents, Recently added, Alphabetical, Creator; Playlists also offers Custom order. The squares icon toggles list <-> grid. Swipe right on an item reveals pin (max four). 9to5Mac reports there is NO manual reorder of the whole library (XDA: 'no way to manually reorder'); custom order exists only for playlists. The 2019 Pratt critique notes the library search/filter bar hid above the list when scrolled (poor discoverability), which later versions fixed by a visible search icon.
- Evidence quality: documented
- Conflicts / confidence: Chips, pinning, grid, sort: high for the 2021 launch; labels 'Recents/Recently added/Alphabetical/Creator' are community snippets (medium). Current 2026 chip order and styling UNVERIFIED. Spotify has no tiles like Years, Genres, Most played, so those are R1 additions.
- User requirement overrides: Latest requirement: the Spotify-style shell is canonical, so the legacy reorderable tile grid is replaced, not styled. The persistent tile order is migrated into chip order.
- R1 adaptation: Replace the 10-tile grid with a Library tab list. Top: 56 px header (title 'Your Library' font_ui_24, search and + icons 48x48 right). Below a horizontally scrolling chip row, 40 px tall chips with 16 px gutters and 8 px gaps: Playlists, Albums, Artists, Tracks, Genres, Years, Favourites, Most played, Folders (keep rest under a trailing 'More' chip that opens a sheet). Chip: unselected #282828 with white font_ui_16 text; selected #1ed760 with black text; tapping the selected chip clears it. Row height 72 px (56 px artwork, 8 px radius for albums, circular for artists, 16 px gap, title font_ui_18 white, secondary font_ui_16 #b3b3b3). Sort chip/button 'Recents' with up-down glyph opens a bottom sheet of radio rows (Recents, Recently added, A-Z, Creator); a 48x48 grid toggle swaps to a 2-column grid (cell 216x~270, artwork 216 px). The Rearrange main page screen becomes 'Customize chips': a list with drag grips and show toggles, at least one chip must remain on; auto edge scroll kept. Drop the bottom-edge Cover Flow pull from the main page (offer it as an optional view in album view settings) because Spotify has no equivalent gesture and it competes with the mini-player drag.
- Acceptance criteria:
  - The Library tab shows a chip row and a bounded paginated list; no 3x4 tile grid is reachable from it
  - Chip tap filters within 200 ms of data being ready, selecting again clears, selected state is green with black text
  - Sort and view mode persist across navigation and restart
  - Each row hit area is >=64 px tall; artwork loads asynchronously and the list stays responsive during playback
  - Customize chips refuses to hide the last remaining chip with an inline note

### Track context actions, long-press multi-select and Add to playlist/queue (filesystem browser, Play, Shuffle)

- Audit IDs: BRW-PLAY, BRW-SHUFFLE, BRW-SELECT, FILES-MENU
- Research date 2026-11-02; context: Android and iOS. Gesture list from iGeeksBlog updated 2026-05-06; 2019 Pratt iOS critique; 2020 newsroom iOS refresh. Spotify mobile has NO native multi-select in 2025 (guide dated Sept 2025, snippet). Accessed 2026-11-02.
- Sources:
  - [7 Spotify gestures every user should know (iGeeksBlog)](https://www.igeeksblog.com/spotify-gestures-every-user-should-know/), published 2026-05-06 (updated), accessed 2026-11-02, read
  - [3 Icons to Know in Spotify Mobile's Refreshing New Look](https://newsroom.spotify.com/2020-02-27/3-icons-to-know-in-spotify-mobiles-refreshing-new-look/), published 2020-02-27, accessed 2026-11-02, read
  - [Create and edit playlists - Spotify Support](https://support.spotify.com/us/article/create-playlists/), published undated, accessed 2026-11-02, read
  - [Design Critique: Spotify (iOS app)](https://ixd.prattsi.org/2019/09/spotify-design-critique/), published 2019-09-08, accessed 2026-11-02, read
  - [Mobile: Add/Delete multiple Songs at once - Spotify Community Idea](https://community.spotify.com/t5/Live-Ideas/Mobile-Add-Delete-multiple-Songs-at-once/idi-p/1001523/highlight/true/page/25), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Track row: cover art (all views except Album view), title white, artist grey, three-dot overflow button at right (vertical dots on Android, horizontal on iOS). Playlist header: large green circular Play button and a Shuffle toggle; the 2020 refresh added a green 'Shuffle Play' button that combines playback and shuffle in one tap, plus an action row (like, download, add, more). The overflow opens a bottom sheet with the track header (art, title, artist) and rows including Add to playlist, Add to queue, Like. Add to playlist opens a full-height sheet/page listing playlists with checkmarks and a 'Done' button (support page: choose the playlist, then tap Done).
- Observed behavior: Tap a row -> plays the context and the mini-player appears; Now Playing is NOT auto-expanded (user taps the mini-player). Swipe right on a row = add to queue (green queue icon confirm); swipe left = like (snackbar 'Added to Liked Songs' with Change). Edit mode on a playlist: remove control per row and drag handle, then Save/Done. There is NO long-press multi-select on mobile (community idea threads, snippet; Sept 2025 guide), so multi-select is an R1 adaptation modelled on Gmail/Photos-style selection with a contextual top bar. Auto-expanding Now Playing on file tap is a legacy Sonix behaviour not found in Spotify.
- Evidence quality: documented
- Conflicts / confidence: Gestures and track-row anatomy: medium-high (dated sources agree). Multi-select non-existence: medium (snippet from a Sept 2025 guide and community ideas). Add-to-playlist sheet visuals: medium-low (support text only).
- User requirement overrides: None. Multi-select is kept as an R1 feature (explicitly user-reachable today) and adapted into the Spotify visual language.
- R1 adaptation: Tap an audio file row: start playback and show the mini-player (64 px tall above the 56 px tab bar); do NOT auto-expand Now Playing (or make it a setting default off). Folder header shows a 56x56 green circle Play and a 40 px shuffle toggle at the right of the 'Local files' header, replacing the corner Shuffle; shuffle starts a random track and keeps the mini-player collapsed. Row overflow: 48x48 three-dot button opens a bottom sheet (#202124): 72 px header with 48 px art, title font_ui_18, artist font_ui_16 #b3b3b3, then 56 px action rows with 24 px icons: Add to queue, Play next, Add to playlist, Add to Favourites, Go to album, Go to artist. Long-press a row starts select mode: top bar swaps to '3 selected' (font_ui_20) with an x on the left and a text button 'Select all'; checkbox circles appear at the row left; a docked 56 px action bar shows Queue, Favourite, Add to playlist; the physical Back key exits selection. Add to playlist opens a full-height sheet (title 'Add to playlist', search field, 'New playlist' row first, 64 px rows with green check when contained, sticky 'Done' pill). Confirm with a bottom snackbar.
- Acceptance criteria:
  - Long-press enters select mode without starting playback; Back key and x exit it and clear the selection
  - Action bar shows the live selected count and disables actions at 0
  - Add to playlist sheet marks playlists already containing the track and writes on Done; failure leaves the collection unchanged
  - Tapping a file starts playback and shows the mini-player with the browsing position unchanged after returning
  - Shuffle from a folder with no tracks is disabled and says why

### Rename, new folder, create playlist and Last.fm sign-in text entry (shared keyboard and name layers)

- Audit IDs: FILES-NAME, MSET-ARTISTEXC, LASTFM-LOGIN
- Research date 2026-11-02; context: Android and iOS. Rename path from Android Authority style guides (updated 2025-01-11, getdroidtips); create flow from Spotify support. Keyboard is the OS keyboard on Spotify, so no keyboard visual spec exists. Accessed 2026-11-02.
- Sources:
  - [How to rename a Spotify playlist (GetDroidTips)](https://www.getdroidtips.com/rename-spotify-playlist/), published 2025-01-11 (updated), accessed 2026-11-02, read
  - [Create and edit playlists - Spotify Support](https://support.spotify.com/us/article/create-playlists/), published undated, accessed 2026-11-02, read
  - [Change playlist name on Spotify (Android Authority)](https://androidauthority.com/change-playlist-name-spotify-3081864), published unknown, accessed 2026-11-02, snippet
  - [Design Critique: Spotify (iOS app)](https://ixd.prattsi.org/2019/09/spotify-design-critique/), published 2019-09-08, accessed 2026-11-02, read
- Observed appearance: Create playlist: tap the + Create button -> Playlist -> a name entry screen (large centred name field, keyboard raised) with a green 'Create' pill. Rename: overflow -> 'Edit Playlist' (Android) / 'Edit' (iOS) -> an edit page with cover art and a text field holding the current name, Save (Android) or Done (iOS) at the top. The text field is one centred or left aligned large font with an underline; no extra chrome. Login on Spotify mobile is a separate email/password form (not researched, no read source).
- Observed behavior: The keyboard opens immediately focused with the existing name selected; Save/Done commits and returns to the previous page; leaving without saving discards. Empty names are blocked by a disabled Save/Create. Rename requires owning the playlist. No confirmation dialog is used for the edit. Spotify Android offers a shortcut: press and hold the name to open the edit option.
- Evidence quality: documented
- Conflicts / confidence: Labels (Save vs Done, Edit Playlist vs Edit) are platform differences, high confidence. Keyboard look: unavailable (OS keyboard). Login form: no evidence read.
- User requirement overrides: None. Long-press-name shortcut is not adopted (R1 uses overflow menu + Rename row).
- R1 adaptation: Use one shared 'Text entry' page for rename, new folder, Unsplit artists and Last.fm username/password: 56 px header with x at left (cancels, asks to discard only if text changed), title font_ui_20 ('Rename', 'New folder'), and a right-aligned green text button 'Save' (disabled grey when empty or unchanged). Field: 56 px tall at y=96, 16 px gutters, #282828 fill, 8 px radius, font_ui_20 white, 2 px #1ed760 bottom/focus outline, helper or error line font_ui_16 #b3b3b3 / #f3727f beneath. The shared keyboard docks at the bottom (rows >=56 px tall keys on 480 px width, key font_ui_20, #121212/#282828 keys, green return key labelled Done/Save). Password field has a 48x48 eye toggle inside the field and the form shows a 56 px green pill 'Log in'. The field text stays visible above the keyboard (page scrolls so the field is above y=360). Do not use the old full-screen name layer with legacy OK.
- Acceptance criteria:
  - The focused text field and its error line are never covered by the keyboard
  - Save is disabled for empty or unchanged text; invalid characters show an inline error without closing the page
  - Back key and x close the page; discarding a modified entry asks for confirmation
  - Return key on the keyboard commits like Save
  - Password characters are masked by default and the eye toggle is >=48x48

### Confirmation dialogs and destructive actions (delete file/folder, sign out of Last.fm, MSEB reset, cancel scan)

- Audit IDs: FILES-DELETE, LASTFM-PAGE, MSET-SOUND, SCAN-PROGRESS
- Research date 2026-11-02; context: iOS (iPhone Life guide updated 2023-03-20); Android not independently confirmed. Spotify's support pages do not mention confirmation dialogs. Accessed 2026-11-02.
- Sources:
  - [How to Delete a Playlist from Your iPhone (iPhone Life)](https://iphonelife.com/node/272534), published 2023-03-20 (updated), accessed 2026-11-02, read
  - [Save and recover playlists - Spotify Support](https://support.spotify.com/us/article/save-recover-playlists/), published undated, accessed 2026-11-02, read
  - [Create and edit playlists - Spotify Support](https://support.spotify.com/us/article/create-playlists/), published undated, accessed 2026-11-02, read
  - [Delete Playlists - Spotify Community](https://community.spotify.com/t5/iOS-iPhone-iPad/Delete-Playlists/m-p/5041179/highlight/true), published unknown, accessed 2026-11-02, snippet
- Observed appearance: The deletion confirm is a centred modal pop-up with a destructive 'Delete' button (iPhone Life); exact title and body wording are NOT quoted by any source I could read. Playlist deletion is recoverable for 90 days from the web 'Recover playlists' page, which is why the dialog is short. Removing a followed playlist from the library shows a 'Removed from Your Library' snackbar rather than a dialog (search-result snippet, not verified).
- Observed behavior: Flow: overflow -> 'Delete Playlist' -> pop-up with Delete / Cancel -> returns to Your Library with the item gone. No undo toast is documented on mobile. Filesystem delete on R1 is NOT recoverable, so it needs stronger wording than Spotify's.
- Evidence quality: community report
- Conflicts / confidence: Existence of a Delete/Cancel pop-up: medium (one dated iPhone Life guide); exact copy: unknown; Android: unverified. Support pages explicitly state no confirmation steps.
- User requirement overrides: AGENTS.md: collection changes must never delete audio files; only the explicit File explorer delete may, and it must say so.
- R1 adaptation: Shared confirm dialog: centred card 320 px wide, #282828, 16 px radius, 24 px padding, scrim rgba(0,0,0,0.6). Title font_ui_20 white ('Delete 3 items?'), body font_ui_16 #b3b3b3 (names the item and says 'This cannot be undone.' for files; 'Your audio files are not deleted' for library-only removals). Two 48 px-tall buttons right aligned: text 'Cancel' (white) and 'Delete' (pill #ff5252 text on transparent, or red-filled for irreversible file deletion). Sign out of Last.fm uses 'Sign out' with a neutral button; MSEB reset uses 'Reset'; stopping a scan uses 'Stop scan' with the count preserved. Back key = Cancel; tapping the scrim cancels. During recursive delete show the progress sheet (see Snackbars) with the current name and block taps.
- Acceptance criteria:
  - Every destructive action goes through the shared dialog; Back key and scrim tap cancel
  - The dialog title names the object and count; file deletion copy states irreversibility, library removal copy states files are kept
  - Buttons are >=48 px tall with 16 px gap and do not wrap at 480 px
  - Confirming deletes only the selection, and deleting open or playing files is blocked with an error snackbar

### Snackbars, toasts and progress modals (background scan, reorganize, move/copy progress)

- Audit IDs: SCAN-BACKGROUND, FILES-PICK, MSET-SCANOPTS
- Research date 2026-11-02; context: Android and iOS mobile. Evidence from 2019 Pratt iOS critique, 2023 9to5Mac plus-button flow (snippet; fetch returned 401), Spotify community complaints about a toast blocking the Done button. Duration unofficial. Accessed 2026-11-02.
- Sources:
  - [Design Critique: Spotify (iOS app)](https://ixd.prattsi.org/2019/09/spotify-design-critique/), published 2019-09-08, accessed 2026-11-02, read
  - [7 Spotify gestures every user should know](https://www.igeeksblog.com/spotify-gestures-every-user-should-know/), published 2026-05-06 (updated), accessed 2026-11-02, read
  - [Spotify heart button turns to plus button (9to5Mac)](https://viptest.9to5mac.com/2023/02/27/spotify-heart-button-turns-to-plus-button/), published 2023-02-27, accessed 2026-11-02, snippet
  - [Mobile: Prevent/fix 'Added to' message blocking Done button UI - Spotify Community](https://community.spotify.com/t5/Live-Ideas/Mobile-Prevent-fix-Added-to-message-blocking-Done-button-UI/idi-p/7460074), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Transient pill/bar near the bottom, above the mini-player and tab bar, with a short message ('Added to Liked Songs', 'Removed from Liked Songs', 'Removed from Your Library') and sometimes a green action text ('Change'). Users say it can sit over the playlist Done button for about five seconds (community, snippet).
- Observed behavior: Appears immediately on the action, auto-dismisses after roughly 5 seconds (unofficial), can be swiped away, and does not take focus or block the UI. The Pratt critique credits these confirmations with teaching users hidden gestures. There is no progress modal for long operations in the sources found; indexing is silent.
- Evidence quality: community report
- Conflicts / confidence: Presence and message style: medium. Duration, exact geometry, colours: low (inferred from community posts). Progress modals: no Spotify evidence, so the R1 design is an adaptation.
- User requirement overrides: Keep expensive scanning and copying off the UI thread per AGENTS.md.
- R1 adaptation: Snackbar: 48 px tall, 16 px side margin, 12 px radius, #282828 fill, font_ui_16 white, optional green action text (48 px hit), anchored 8 px above the mini-player (y ~ 680 when mini-player and tab bar are visible), 4 s auto-dismiss, swipe-down dismiss, a new message replaces the old one. Silent Detect changes runs with a small indeterminate status glyph in the header of Library (not a toast) and finishes with 'Library updated' snackbar only if tracks changed. 'Reorganize' shows a persistent non-modal banner row at the top of Library ('Reorganizing library...' with 2 px green progress line) and a final snackbar 'Library reorganized'; it must be cancellable. Move/Copy progress: bottom sheet (#202124) with title 'Copying 12 items', current name (ellipsised), 4 px green bar, and a 'Cancel' text button; result is a snackbar ('Copied 12 items' / 'Couldn't copy 2 items' + 'Details'). Avoid full-screen blocking busy popups.
- Acceptance criteria:
  - Snackbars never cover the mini-player controls and dismiss after 4 s or by swipe
  - Background indexing never blocks input; only the Library header status glyph changes
  - Move/Copy progress sheet shows current item and can be cancelled, leaving already-copied items intact and reporting counts
  - Only one snackbar is visible at a time, queued messages replace in order
  - Error text names the cause (card removed, no space, permission)

### Sleep timer (inside Playback and queue settings, and Now Playing menu)

- Audit IDs: MSET-PLAYBACK
- Research date 2026-11-02; context: Android (introduced 2019-05-27) and iOS (2019-12-03; iMore). Song-timer durations 5 to 60 minutes, plus 'end of track'. Newer 2026 podcast variant uses a moon icon and 'end of episode' (snippet only). Accessed 2026-11-02.
- Sources:
  - [Spotify sleep timer Android (9to5Google)](https://9to5google.com/2019/05/27/spotify-sleep-timer-android/), published 2019-05-27, accessed 2026-11-02, read
  - [Spotify for iOS gains Sleep Timer (iMore)](https://www.imore.com/spotify-ios-gains-same-sleep-timer-feature-androids-had-months), published 2019-12-03, accessed 2026-11-02, read
  - [How to set Spotify sleep timer (FreeYourMusic)](https://freeyourmusic.com/en/blog/how-to-set-spotify-sleep-timer), published Updated Sep 2026 (per search result), accessed 2026-11-02, snippet
  - [Unlock Audiobooks - Spotify Support (mentions Sleep timer control in Now Playing)](https://support.spotify.com/br-en/article/audiobooks-unlock/), published undated, accessed 2026-11-02, snippet
- Observed appearance: A list in a bottom sheet: Sleep Timer entries 5, 10, 15, 30, 45 minutes, 1 hour, End of track (list reported as 5 min to 1 hour, plus end of current track), and 'Turn off timer' appears once active. An active timer is shown in Now Playing by a moon icon/countdown (snippet). Exact row styling unavailable.
- Observed behavior: Open Now Playing -> overflow (three dots, top right) -> scroll to 'Sleep Timer' -> choose a duration; it starts immediately, no confirm; reopen the menu to see 'Turn off timer'. When it expires playback stops. Spotify offers fixed presets and no wheel picker. R1's toggle + duration wheels is therefore a non-Spotify pattern.
- Evidence quality: documented
- Conflicts / confidence: Android 2019 article lists 5 min to 1 hour and end of track (read). iMore says 5 min to 1 hour, no presets listed (read). Preset values 5/10/15/30/45/60 come from a 2019 MobileSyrup snippet (not read). End-of-track for music is confirmed only by the 9to5Google Android article. 2026 UI unverified.
- User requirement overrides: None. Custom duration is an R1 addition beyond Spotify's fixed list.
- R1 adaptation: Replace the toggle + wheels with a 'Sleep timer' row (value shows 'Off' or '23 min left') that opens a bottom sheet titled 'Sleep timer' (font_ui_20): 56 px radio rows 5 minutes, 10 minutes, 15 minutes, 30 minutes, 45 minutes, 1 hour, End of track, and a 'Custom...' row opening a 2-wheel hour/minute picker only for custom (wheels use font_ui_24, row 48 px, selected row in green). When active, a first row 'Turn off timer' in red-neutral text. Also expose the same sheet from the Now Playing overflow so the user does not need to go into Settings. On expiry fade the volume over 5 s, pause, and show a snackbar 'Sleep timer ended'. Timer survives screen-off but not reboot.
- Acceptance criteria:
  - Selecting a preset starts the timer immediately and the settings row updates to the remaining time
  - Turn off timer is visible only while active and cancels without confirmation
  - End of track stops after the current track finishes, including with gapless/crossfade on
  - Now Playing overflow and Settings open the identical sheet
  - Timer expiry pauses playback and does not corrupt the queue position

### Crossfade, gapless and Automix (Crossfade and gapless settings, Playback and queue toggles)

- Audit IDs: MSET-FADE, MSET-PLAYBACK
- Research date 2026-11-02; context: Android and iOS, support article (undated, current). Crossfade is Premium. Settings are unavailable while casting over Spotify Connect. Accessed 2026-11-02.
- Sources:
  - [Transitions between tracks - Spotify Support](https://support.spotify.com/us/article/tracks-transitions/), published undated, accessed 2026-11-02, read
  - [Local files - Spotify Support (crossfade note for local files)](https://support.spotify.com/us/article/local-files/), published undated, accessed 2026-11-02, read
  - [How To Turn On Crossfade On Spotify (WORKING 2026!) - YouTube](https://www.youtube.com/watch?v=9Dir8nFlw1A), published 2026 (title only), accessed 2026-11-02, snippet
- Observed appearance: Settings and privacy -> Playback. 'Crossfade' appears as its own labelled item with a slider (range not stated on the page; commonly reported 0 to 12 s, UNVERIFIED). Under the heading 'Track transitions': switch 'Gapless playback', switch 'Automix'. Slider value label and tick styling not documented.
- Observed behavior: Crossfade overlaps and fades two tracks; slider sets length; gapless removes pauses between tracks; Automix only on select playlists. Crossfade between local files works only when sample rate and channel count match, a real constraint relevant to R1 local playback. Not settable while using Connect.
- Evidence quality: documented
- Conflicts / confidence: Paths and labels high. Slider range 12 s: not on the support page (YouTube evidence unreadable), so treat as unverified.
- User requirement overrides: None.
- R1 adaptation: One page 'Playback' sections: 'Track transitions' with a 64 px 'Gapless playback' toggle row and a 'Crossfade' row with toggle; when on, an adjacent full-width slider row (96 px tall: label 'Crossfade' font_ui_18 left, value '6 s' font_ui_18 #b3b3b3 right, 4 px track #535353 with #1ed760 fill, 20 px white thumb inside a 48 px touch zone, range 1 to 12 s in 1 s steps). Secondary line under crossfade: 'Only tracks with the same sample rate and channels are blended' (derived from the Spotify local-files note). When Bit-perfect is on, both rows are dimmed to 40% with a secondary line explaining why. Slider changes apply live without a Save.
- Acceptance criteria:
  - Gapless and crossfade toggles keep their state across restart and take effect on the next track boundary
  - The crossfade slider is operable with a finger drag and shows the value while dragging
  - Mixed-sample-rate tracks fall back to gapless without audible glitches and the note explains the rule
  - Dimmed rows are non-interactive under bit-perfect and say why

### Equalizer, normalization, audio quality and sound/DAC settings (EQ, PEQ, MSEB, ReplayGain, bit-perfect, effects, volume limit, DAC options)

- Audit IDs: MSET-SOUND, MSET-EFFECTS-DAC
- Research date 2026-11-02; context: Android and iOS support pages (undated). Android EQ switch is under 'Listening controls' and may drive the system equalizer. Normalization labels differ by OS. Quality tiers (Low 24 kbit/s ... Lossless 24-bit/44.1 kHz FLAC). Mono audio and balance are NOT covered by the pages I read. Accessed 2026-11-02.
- Sources:
  - [Equalizer - Spotify Support](https://support.spotify.com/us/article/equalizer/), published undated, accessed 2026-11-02, read
  - [Volume normalization - Spotify Support](https://support.spotify.com/us/article/volume-normalization/), published undated, accessed 2026-11-02, read
  - [Audio quality - Spotify Support](https://support.spotify.com/us/article/audio-quality/), published undated, accessed 2026-11-02, read
  - [Spotify for iOS scores an equalizer (PhoneArena)](https://www.phonearena.com/news/Spotify-for-iOS-scores-an-equalizer-with-a-lot-of-presets-in-tow-at-long-last_id58733), published c.2015 (unconfirmed), accessed 2026-11-02, snippet
  - [Best Spotify Equalizer Settings (Headphonesty)](https://www.headphonesty.com/2022/03/best-spotify-equalizer-settings/), published 2022-03, accessed 2026-11-02, snippet
- Observed appearance: Equalizer: a switch in Settings -> Playback, then a screen with a preset dropdown (Flat is default) and a multi-band graph with draggable dots (iOS 6 bands, Android 5 bands, per snippet-only sources). Normalization: switch ('Enable Audio Normalization' on iOS, 'Volume normalization' on Android) plus a 'Volume level' choice Loud / Normal (default) / Quiet. Media quality: levels Automatic, Low, Normal, High, Very high, Lossless as a choice list, with 'Auto adjust quality' switch.
- Observed behavior: All audio settings are disabled when playing over Connect. Lossless exposes the output sample format. Spotify has no DAC filter, NOS, DRE, DSD gain, crossfeed, soundfield, bit-perfect or per-output volume limit, so these are R1-only. The closest Spotify patterns are Playback rows, the 'Volume level' radio list, and the EQ screen graph.
- Evidence quality: documented
- Conflicts / confidence: Normalization and quality labels: high. EQ visuals: LOW (text only; descriptions disagree on band counts and presets). No source for DAC-like settings, as expected.
- User requirement overrides: None. DAC controls remain R1-only, documented under the Spotify-looking 'Audio' section.
- R1 adaptation: Settings > Audio: rows 'Equalizer' (value: preset name or Off, chevron), 'Parametric EQ', 'Normalize volume' (toggle, ReplayGain mode as a 'Volume level' sheet with Track / Album / Off radio rows), 'Bit-perfect output' (toggle with secondary line 'Disables effects and EQ'), 'Effects' (Soundfield, Crossfeed, Balance and mono), 'Volume limit', 'DAC' (High gain, Filter, NOS, DRE, DSD gain). EQ page: header with an on/off toggle at right, preset chip row (40 px chips), and a band graph filling 480x360: draggable dots 32 px visual with 48 px hit, vertical -12..+12 dB range, band labels font_ui_16 #b3b3b3, 'Reset' text button; MSEB uses the same page with its own bands and a confirm dialog for reset. Slider rows (crossfeed, balance, volume limit): 96 px tall as in the Playback record. DRE row is hidden (not just disabled) on CS43131 hardware; DAC rows carry a one-line description since users have no Spotify reference. Disabled-by-bit-perfect rows dim to 40% and read 'Turned off by Bit-perfect'.
- Acceptance criteria:
  - Every audio control is reachable from the Audio settings page in two taps or fewer and opens Spotify-styled pages
  - EQ graph dots can be dragged with a finger and the value readout stays visible while dragging
  - Bit-perfect dims and disables dependent rows, restoring their previous values when turned off
  - DRE is hidden on unsupported hardware instead of showing a dead control
  - All slider and toggle changes persist and apply without leaving the page

### Last.fm and connected apps (scrobble page, account row, sign-in)

- Audit IDs: LASTFM-PAGE, LASTFM-LOGIN
- Research date 2026-11-02; context: Spotify no longer offers an in-app Last.fm setting; it is connected from Last.fm's website (community FAQ snippets, Last.fm applications page). Spotify's own 'Apps and devices' section lists connected apps such as Google Maps and Waze. Accessed 2026-11-02.
- Sources:
  - [Spotify on other apps - Spotify Support](https://support.spotify.com/us/using_spotify/app_integrations/scrobble-to-last-fm/), published undated, accessed 2026-11-02, read
  - [How To Open And Adjust Your Spotify Settings On Android](https://www.slashgear.com/1965140/how-to-open-spotify-settings-on-android), published 2025-09-14, accessed 2026-11-02, read
  - [How can I connect Spotify to Last.fm? - Spotify Community FAQ](https://community.spotify.com/t5/FAQs/How-can-I-connect-Spotify-to-Last-fm/ta-p/4795301), published unknown, accessed 2026-11-02, snippet
  - [last.fm scrobbling no longer possible - Spotify Community](https://community.spotify.com/t5/Your-Library/last-fm-scrobbling-no-longer-possible/td-p/4996229), published unknown, accessed 2026-11-02, snippet
- Observed appearance: No in-app Last.fm screen exists on Spotify mobile. The nearest pattern is the Apps and devices list: rows for each connected app with a chevron or a Remove access action on the web Apps page.
- Observed behavior: Connection is granted explicitly ('Your Spotify account will never link to a third-party app without your explicit permission' - read), and removed via Remove Access. Last.fm's own flow is Settings > Applications > Connect next to Spotify Scrobbling (snippet only).
- Evidence quality: inferred
- Conflicts / confidence: Existence of a native Spotify Last.fm screen: refuted (medium confidence). The R1 design is therefore an adaptation of the connected-apps pattern and the Spotify email/password login, whose visuals were not found.
- User requirement overrides: Product is local-only; Last.fm is a retained local feature (scrobbling to an external web service) and must not reintroduce online-music services.
- R1 adaptation: Settings > Apps > 'Last.fm' row with a secondary line 'Connected as <user>' / 'Not connected' / '12 scrobbles queued'. The Last.fm page: header + a 72 px account block (48 px avatar circle with initial, user name font_ui_20, status font_ui_16 #b3b3b3), toggle row 'Scrobble plays', info rows 'Status' and 'Queued', and a full-width 56 px pill at the bottom: 'Log in' (green) or 'Log out' (outlined, opens the confirm dialog). Sign-in uses the shared text-entry page: username and password fields, 56 px green 'Log in' pill disabled until both are filled, busy state as a button spinner (not a blocking toast), error as an inline red line under the password field.
- Acceptance criteria:
  - Log in is disabled until username and password are non-empty and shows a spinner on the button while waiting
  - Failed login stays on the page with an inline error and keeps the username
  - Sign-out always asks for confirmation and clears stored credentials but keeps queued scrobbles for the next login (or states otherwise)
  - Toggling scrobbling off persists and leaves the account connected

### Mini-player and Now Playing expansion after tapping a file (browser, shuffle)

- Audit IDs: BRW-PLAY, BRW-SHUFFLE
- Research date 2026-11-02; context: Android mini-player redesign test 2021-07-28 (Spotify 8.6 beta); iOS swipe-up/down per community; gestures 2026 list. Accessed 2026-11-02.
- Sources:
  - [Spotify tests floating mini player (9to5Google)](https://9to5google.com/2021/07/28/spotify-miniplayer-android-test/), published 2021-07-28, accessed 2026-11-02, read
  - [7 Spotify gestures every user should know](https://www.igeeksblog.com/spotify-gestures-every-user-should-know/), published 2026-05-06 (updated), accessed 2026-11-02, read
  - [Spotify upgrades its mini player (Android Headlines)](https://www.androidheadlines.com/2021/07/spotify-upgrades-its-mini-player-ui-with-modern-look.html), published 2021-07, accessed 2026-11-02, snippet
- Observed appearance: The mini-player floats above the tab bar with rounded corners, takes a colour from the current artwork, shows art, title, artist, a like/add button, Connect icon and play/pause, with a thin progress bar at the bottom edge (2021 test).
- Observed behavior: Tapping a track starts playback and shows the mini-player; tapping the mini-player expands Now Playing; swiping down on the expanded view returns to the previous page; swipe left/right on the mini-player skips tracks (read).
- Evidence quality: documented
- Conflicts / confidence: Medium-high for behaviour; 2021 layout may differ from 2026. Auto-expansion on track tap is not a Spotify behaviour (inferred from the sources).
- User requirement overrides: Auto-expanding Now Playing after file tap is a legacy Sonix behaviour retained only if the user setting 'Now Playing at boot/open' says so.
- R1 adaptation: Mini-player 64 px tall, 8 px side margins, 12 px radius, tinted from the artwork (darkened to keep text contrast >= 4.5:1), 48 px art, title font_ui_18, artist font_ui_16 #b3b3b3, play/pause 48x48 at right, 2 px green progress at the bottom edge, swipe left/right skips, tap or swipe up expands. Filesystem browser keeps its position when playback starts; a 'Now playing' sheet opens only on explicit tap. Bottom padding of lists increases by 64 px while the mini-player is visible so the last row stays reachable.
- Acceptance criteria:
  - Tapping a file shows the mini-player without moving the browser's scroll position
  - Tap or swipe up expands Now Playing and swipe down collapses it to the same page
  - Last list row is fully visible above the mini-player and tab bar
  - Mini-player text meets 4.5:1 contrast on artwork tint

### File explorer, text reader and image viewer (R1-only utilities)

- Audit IDs: FILES-ROOT, FILES-MENU, FILES-PICK, TEXT-VIEW, IMAGE-VIEW
- Research date 2026-11-02; context: No Spotify counterpart. Closest patterns: Spotify's Local Files folder view, the track overflow bottom sheet, Settings > Storage ('Clear cache' / download location), and the artwork enlarge view. Accessed 2026-11-02.
- Sources:
  - [Spotify storage information - Spotify Support](https://support.spotify.com/us/article/storage-information/), published undated, accessed 2026-11-02, read
  - [Create and edit playlists - Spotify Support](https://support.spotify.com/us/article/create-playlists/), published undated, accessed 2026-11-02, read
  - [Local files - Spotify Support](https://support.spotify.com/us/article/local-files/), published undated, accessed 2026-11-02, read
- Observed appearance: Storage in Spotify is a section 'Storage' in Data-saving and offline with a 'Clear cache' row and a 'Clear' button; Android shows a download-location choice with Continue. Spotify has no file manager, text viewer or image viewer; lyrics is the closest full-screen text view and artwork the closest image view, but no source was read about either.
- Observed behavior: Storage actions are single-tap rows with no extra dialog (not documented). All Spotify mobile list items use the three-dot overflow menu and bottom sheets. No evidence on file manipulation. Therefore the file explorer must follow the generic Library/Local files list pattern and the shared bottom sheet.
- Evidence quality: inferred
- Conflicts / confidence: No direct evidence; adaptation inferred from the shared Spotify list, sheet and settings patterns. Lyrics/artwork reference not verified.
- User requirement overrides: R1-only utilities are kept (AGENTS.md: do not silently remove a feature) but must adopt the Spotify visual language and navigation.
- R1 adaptation: File explorer: Settings > Storage > 'File explorer' (or Library > Local files > Folders) uses the shared folder list: 56 px header with back chevron and the folder name (font_ui_20, ellipsised, breadcrumb sub-line font_ui_16 #b3b3b3), 64 px rows (24 px folder/file icon, name font_ui_18, size/date font_ui_16 #b3b3b3, 48x48 three-dot at right). Row menu = shared bottom sheet with Rename, Move to, Copy to, Delete (Delete in red-neutral); New folder is a 56 px circular green + button above the mini-player. Move/Copy destination picker: full-height sheet 'Choose destination' listing folders only, sticky pill 'Move here' / 'Copy here'. Text reader: black #000 body with font_ui_18 text and a 56 px header with the file name; page scrolls with fast flick, large files open in chunks; unreadable shows empty-state panel with icon and 'Can't open this file' + 'Close'. Image viewer: edge-to-edge image on #121212, a centred spinner while decoding, tap toggles the 56 px header, double-tap zoom, 'Can't show this image' state. Hidden/dotfiles shown only after a 'Show hidden files' toggle in the sheet.
- Acceptance criteria:
  - File explorer rows, row menu, destination picker and viewers all use the Spotify-style header, rows and sheet, with no legacy popovers
  - Destination picker lists folders only, hides the source folder subtree and disables the action when destination equals source
  - Text and image viewers handle unreadable or oversized files with an empty-state panel, not a silent blank screen
  - All hit areas >= 48 px; Back key returns one level and preserves the scroll position
  - Deleting never targets paths outside the selected item

Failed attempts:

- support.spotify.com/us/article/sleep-timer/ returned a Spotify 'Page not found'; no official sleep-timer support article was found (search only surfaced audiobook and DJ pages that mention it).
- support.spotify.com/us/article/crossfade-and-gapless-playback/ and /settings-and-privacy/ returned 'Page not found'; the equivalent facts were obtained from /tracks-transitions/ and from SlashGear's settings walkthrough.
- freeyourmusic.com sleep-timer article (both URLs) timed out; its content is search-result snippet only (actually_read=false).
- 9to5mac.com heart-to-plus article (viptest mirror) returned HTTP 401; snackbar 'Change' behaviour rests on the iGeeksBlog gestures page (read) and search snippets.
- Spotify Community threads (Local Files, New Your Library view, Last.fm FAQ, Local File Discovery iOS, Added-to toast) returned only the page title through WebFetch; their content is snippet-only evidence and marked actually_read=false.
- androidauthority.com/change-playlist-name-spotify-3081864 returned HTTP 403; rename flow taken from GetDroidTips (updated 2025-01-11).
- phonearena.com Spotify iOS equalizer article returned HTTP 403; no readable description of the Spotify equalizer screen layout, so EQ visuals are low confidence.
- newsroom.spotify.com/2021-04-29/introducing-a-new-way-to-browse-and-search-your-library/ returned 404 (guessed URL); official library-redesign evidence is taken from 9to5Mac, iPhone in Canada and XDA (all read).
- YouTube pages (sleep timer iPhone, equalizer 2026, crossfade 2026) returned only page chrome through WebFetch: no upload date, channel, description or transcript, so no video evidence was verified.
- No spotify.design page was reachable through search (results only gave engineering.atspotify.com and newsroom); the Encore accessibility post that was read does not discuss touch targets. No official Spotify source for touch-target size, row height or mobile typography was found, so R1 heights/fonts are design decisions, not measured Spotify values.
- No readable source describes Spotify's Settings row visuals (toggle style, header typography) or the Connect device picker layout; both remain unverified and need a dated screenshot or device capture.
- Spotify Connect page was read, but no Connect picker sheet appearance or its current-device indicator was documented; the Devices/Connect pattern is only used indirectly (settings greyed out while casting).
- No source found for Spotify mobile login form, lyrics view or artwork enlarge view, so LASTFM-LOGIN, TEXT-VIEW and IMAGE-VIEW adaptations are inferred.

## audio

### Equalizer (EQ page, EQ presets list, EQ reset) - covers AUD-EQ-PAGE, AUD-EQ-PRESETS, AUD-EQ-RESET-CONFIRM

- Audit IDs: AUD-EQ-PAGE, AUD-EQ-PRESETS, AUD-EQ-RESET-CONFIRM
- Research date 2026-11-02; context: Spotify iOS: in-app equalizer (Settings and privacy > Playback > Equalizer). Spotify Android: path is the same in the 2025-26 support article under a 'Listening controls' heading, but 2021-2023 third-party reports say tapping Equalizer hands off to the device/manufacturer EQ, which changes all apps' audio. Support page undated; community FAQ and Tom's Guide not readable. Experiments and app versions unknown. No evidence of custom user-saved presets on iOS (a community feature request exists; its body was not readable).
- Sources:
  - [Spotify Support - Equalizer](https://support.spotify.com/us/article/equalizer/), published undated (current), accessed 2026-11-02, read
  - [TechCrunch - Spotify Adds An Equalizer So You Can Turn Up The Bass](https://techcrunch.com/2014/07/29/spotify-adds-an-equalizer-so-you-can-turn-up-the-bass), published 2014-07-29, accessed 2026-11-02, read
  - [iOS Hacker - Change Equalizer Settings on Spotify for iPhone](https://ioshacker.com/how-to/change-equalizer-settings-on-spotify-for-iphone), published 2026-08-20 (as reported by the fetch), accessed 2026-11-02, read
  - [Audio Solace - How To Use Spotify Equalizer](https://audiosolace.com/how-to-use-spotify-equalizer/), published 2021-12-01 (page also claims 'Updated November 3, 2026', suspicious), accessed 2026-11-02, read
  - [SlashGear - How To Find Spotify's Equalizer On Android](https://www.slashgear.com/1342840/how-to-find-spotifys-equalizer-android), published 2023-07-23, accessed 2026-11-02, read
  - [SoundGuys - Best equalizer settings for Spotify](https://www.soundguys.com/best-equalizer-settings-spotify-101232/), published 2025-09-11 (updated 2025-03-25 per editor note), accessed 2026-11-02, read
  - [Headphonesty - Best Spotify Equalizer Settings (iOS 6 bands vs Android 5)](https://www.headphonesty.com/2022/03/best-spotify-equalizer-settings/), published 2022-03, accessed 2026-11-02, snippet
  - [Spotify Community - More equalizer bands and custom EQ presets for iOS](https://community.spotify.com/t5/iOS-iPhone-iPad/More-equalizer-bands-and-custom-EQ-presets-for-iOS/m-p/5857684/highlight/true), published unknown, accessed 2026-11-02, snippet
- Observed appearance: iOS Equalizer screen (text-described; no readable screenshot obtained): a top 'Equalizer' switch labelled On/Off, a graph with draggable dots (a 'simplified parametric EQ', 6 control points on iOS per Headphonesty snippet - unverified, vs 5 bands on Android native EQ), and a vertical list of named presets under the graph. Preset names from 2014 TechCrunch (22 names: Acoustic, Bass Booster, Bass Reducer, Classical, Dance, Deep, Electronic, Flat, Hip-Hop, Jazz, Latin, Loudness, Lounge, Piano, Pop, R&B, Rock, Small Speakers, Spoken Word, Treble Booster, Treble Reducer, Vocal Booster); current list may differ. Android with manufacturer EQ: screen is whatever the OEM draws, with a toggle top right (SoundGuys). Spotify shows a warning that EQ can cause sudden volume changes on Android (SoundGuys).
- Observed behavior: Switch turns EQ on/off. Choosing a preset applies immediately and the graph updates; dragging a dot afterwards customises the curve live. Per the support article: audio settings (including EQ) cannot be changed while playing through Spotify Connect on another device. No evidence of an on-page reset-all, per-band +/- stepper, or a width/Q control in Spotify. No evidence of a confirm dialog for EQ reset. Android: on devices with a built-in EQ, changes affect other apps too.
- Evidence quality: documented
- Conflicts / confidence: Medium. Official path/toggle/presets/graph is documented (support page) and corroborated by 2014 TechCrunch + 2026 iOS Hacker. Unresolved: exact current iOS band count (6 is from a search snippet only), whether the preset list is still 22 long, and whether Android in 2026 still redirects to system EQ (2021 and 2023 sources say yes; 2025-26 support text implies the same screen for both). Third-party sources disagree on EQ location (Your Library gear vs profile menu), reflecting the 2023-25 move from gear to profile/Settings and privacy. No visual capture was obtained, so geometry is inferred.
- User requirement overrides: None recorded. AGENTS.md requires R1-only EQ features (10 bands, width pills, headroom note) to be preserved and styled in the Spotify language, not removed.
- R1 adaptation: Keep the R1 10-band drag board (R1-specific) but dress it as the Spotify EQ page: 56 px header (back chevron 48x48 hit area, centred font_ui_20 'Equalizer'), full-width 'Equalizer' row 64 px high with font_ui_18 label and a 52x30 green (#1ed760) switch, graph card on #202124 radius 12 with #1ed760 curve and 20 px knobs (44 px invisible hit area), then 'Presets' heading (font_ui_16 #b3b3b3) and preset rows 56 px with a green check on the active one. Move the band card (-, +, reset band) and width pills into a compact row under the graph using Spotify chip styling (36 px high pills, #282828 fill, selected = #1ed760 fill with black text). Replace the corner gear/reset glyphs with a header trailing '...' that opens the shared bottom sheet (Presets, Reset to flat). Preset tap should apply and stay on the page showing the check (as Spotify does), with an optional toast; do not auto-navigate back. Disable the whole page visually (dim to 40 percent) when On is off. When playing over DAC/BT, no Connect-like lockout is needed.
- Acceptance criteria:
  - EQ page uses #121212 background, 56 px header with 48x48 back target, no legacy topbar or corner glyph buttons.
  - On switch row is 64 px high, entire row toggles, and thumb/track colours match the shared switch used in other Spotify-style settings.
  - Selecting a preset applies the curve, moves the green check, keeps the user on the EQ page, and survives restart.
  - Graph knob hit area is at least 44x44 px and does not overlap neighbouring knobs at 10 bands on 480 px width.
  - Reset (flat) is reachable from the '...' sheet and goes through the shared destructive confirm.
  - Live bounds check: no row or control is under 44 px tall and nothing is clipped at 480x800.

### Settings root and list rows/toggles/single-choice lists (Music settings root, DAC filters, Now Playing overflow 'Settings' handoff, PEQ settings, MSEB settings range pills) - AUD-ENTRY-TOOLS-MUSIC-GEAR, AUD-MUS-ROOT, AUD-NP-TRACKMENU-SETTINGS, AUD-DAC-FILTERS, AUD-PEQ-SETTINGS, AUD-MSEB-SETTINGS

- Audit IDs: AUD-ENTRY-TOOLS-MUSIC-GEAR, AUD-MUS-ROOT, AUD-NP-TRACKMENU-SETTINGS, AUD-DAC-FILTERS, AUD-PEQ-SETTINGS, AUD-MSEB-SETTINGS
- Research date 2026-11-02; context: Spotify Android (SlashGear, 2025-09-14) and iOS (iOS Hacker, 2026-08-20; SoundGuys 2025-09-11): Settings is reached from the profile picture (top left) > 'Settings and privacy', replacing the older Home gear. Layout of the individual rows and the toggle style was not captured visually. Android and iOS differ in labels ('Enable Audio Normalization' on iOS vs 'Volume normalization' on Android per search snippets).
- Sources:
  - [SlashGear - How To Open And Adjust Your Spotify Settings On Android](https://www.slashgear.com/1965140/how-to-open-spotify-settings-on-android), published 2025-09-14, accessed 2026-11-02, read
  - [Spotify Support - Equalizer (shows 'Settings and privacy' > 'Playback' > 'Listening controls')](https://support.spotify.com/us/article/equalizer/), published undated (current), accessed 2026-11-02, read
  - [Spotify Support - Lossless Audio Quality (Media quality section)](https://support.spotify.com/au/article/lossless-audio-quality/), published undated (min app 9.0.58), accessed 2026-11-02, read
  - [Spotify Support - Internet and data usage (Data-saving and offline, Media quality)](https://support.spotify.com/article/internet-and-data-usage), published undated, accessed 2026-11-02, snippet
  - [TechPP - turn off normalization on iPhone/Android](https://techpp.com/?p=239756), published 2025-01 (per search snippet), accessed 2026-11-02, snippet
- Observed appearance: Settings and privacy is a single scrolling list on #121212 with a 'Go Premium' button at the very top (SlashGear) followed by section groups: Account, Content and display, Playback, Privacy and social, Notifications, Apps and devices, Data-saving and offline, Media quality, Advertisements, About and support. Playback contains Crossfade and Equalizer (plus Gapless, Normalize volume, Mono audio, per community snippets). Support text refers to a 'Listening controls' heading on Android's Playback screen. Rows are label (and sometimes secondary grey value/subtitle) with a trailing switch or chevron; the visual spec (heights, font sizes) was not obtained and is inferred.
- Observed behavior: Settings is a push-navigation hierarchy: each section is a page, back chevron returns, no tile grid. Toggles flip inline with no confirm. Choice rows (Media quality: Automatic/Low/Normal/High/Very high/Lossless) are single-select with a check; some are sub-pages. The Now Playing '...' sheet in Spotify does not contain app Settings or EQ; Settings is reachable only through the profile menu (confirmed by SlashGear + support articles). Spotify Android moved the entry from a Home gear to the profile picture in the 2023-25 redesign; older guides still say gear (SoundGuys 2025 reports the gear at top right on Android).
- Evidence quality: documented
- Conflicts / confidence: Medium. Section names are documented (SlashGear 2025-09-14) but the visual row spec is inferred. SoundGuys (gear, Android) and SlashGear (profile picture) disagree on the entry affordance, probably app version or experiment. iOS row ordering and Android 'Listening controls' heading are only partly corroborated.
- User requirement overrides: AGENTS.md: every retained feature must use the Spotify-style system; legacy Tools grid must not be a fallback reachable from the Spotify-style experience.
- R1 adaptation: Treat 'Music settings' as a Spotify Playback page: header 'Playback' (or 'Music settings') with 48 px back, sections as font_ui_16 #b3b3b3 uppercase-free headings ('Sound', 'Effects', 'Volume', 'DAC') with 24 px top padding, rows 64 px high, font_ui_18 white label and font_ui_16 #b3b3b3 sub-label, trailing 52x30 green switch or 24 px chevron / current value. Equaliser, PEQ and MSEB rows are navigation rows with a value summary (On/Off). DAC filter rows become a single-select list with a 24 px green check on the right and the explanatory note as grey caption under the list. Range pills in MSEB settings become the shared chip row. The shell should provide an entry that skips the legacy Tools grid and Music tile grid: put a gear/'Settings' item in the Home header (profile-style) opening this page directly, and keep the Now Playing overflow 'Settings' route, but have its back return to the player. Preserve R1-only hardware rows (High gain, NOS, DRE hidden on CS43131, DSD) with the same row component; hide unsupported ones rather than showing disabled legacy rows.
- Acceptance criteria:
  - From Home, the audio settings are reachable in at most 2 taps without any legacy gridpage tile screen.
  - Every row in Music settings, DAC filters, PEQ settings and MSEB settings uses the same row component: 64 px height, 16 px side gutter, 44 px minimum hit area, #121212 background.
  - Toggle rows toggle on a tap anywhere on the row; value rows show a grey current value; single-choice rows show exactly one green check.
  - Back from Music settings returns to the originating screen (Home header or Now Playing), preserving scroll position and playback.
  - No theme_style_* default colours or legacy topbar are visible; footer/mini-player behaviour matches other Spotify-style pages.

### Audio quality / Lossless / bit-perfect, DAC output options (Bit-perfect, ReplayGain, High gain, Filters, NOS, DRE, DSD) - AUD-MUS-ROOT sound and DAC sections

- Audit IDs: AUD-MUS-ROOT, AUD-DAC-FILTERS
- Research date 2026-11-02; context: Spotify mobile (iOS and Android), Lossless launched 2025-09-10 for Premium, more than 50 markets by 2025-10-06; requires mobile app 9.0.58 or later. Media quality moved to its own Settings section. Not applicable to podcasts/audiobooks/videos.
- Sources:
  - [Spotify Newsroom - Lossless listening arrives on Spotify Premium](https://newsroom.spotify.com/2025-09-10/lossless-listening-arrives-on-spotify-premium-with-a-richer-more-detailed-listening-experience/), published 2025-09-10 (updated 2025-10-06), accessed 2026-11-02, read
  - [Spotify Support - Audio quality](https://support.spotify.com/us/article/audio-quality/), published undated (current), accessed 2026-11-02, read
  - [Spotify Support - Lossless Audio Quality](https://support.spotify.com/au/article/lossless-audio-quality/), published undated, accessed 2026-11-02, read
  - [Spotify Support - Spotify Connect ('Change quality settings')](https://support.spotify.com/us/article/spotify-connect), published undated, accessed 2026-11-02, read
- Observed appearance: Settings and privacy > Media quality section lists Wi-Fi streaming, Cellular streaming and Downloads, each opening a level choice: Automatic, Low (~24 kbit/s), Normal (~96), High (~160), Very high (~320, Premium), Lossless (up to 24-bit/44.1 kHz FLAC, Premium), with data usage shown per level (newsroom). An 'Auto-adjust quality' toggle (on by default) sits below. A Lossless indicator appears in the Now Playing view/bar and in the Connect picker when active.
- Observed behavior: Each level is a single-select row; changing it takes effect on next playback. Lossless must be enabled manually per device and is disabled over Bluetooth/AUX where unsupported (support page). For third-party Connect devices, quality is set from the phone via 'Change quality settings'. Spotify has no bit-perfect, DAC filter, NOS, DSD or ReplayGain controls: loudness is a 'Normalize volume' toggle and EQ is separate. Spotify shows no per-output hardware options.
- Evidence quality: documented
- Conflicts / confidence: High for the quality ladder and Lossless indicator (primary Spotify sources). Low applicability to R1-only DAC controls: the Spotify counterpart is only the list/check pattern and the Lossless badge.
- User requirement overrides: None recorded.
- R1 adaptation: Adopt Spotify's Media quality list pattern for DAC options: a 'Sound' group of toggle rows (Bit-perfect, ReplayGain with value row 'Track/Album/Off', High gain) and a 'DAC' group where Filters opens a single-select page (4 rows, 64 px, check on the right). Provide subtitle copy such as 'Bypasses volume/EQ for exact output' in font_ui_16 #b3b3b3 under the label (max 2 lines). When bit-perfect is on, show EQ/PEQ/MSEB rows dimmed with a 'Off while Bit-perfect is on' hint rather than silently not applying. Add a small Lossless/Hi-Res style pill (font_ui_16, #1ed760 outline, 24 px high) in the Now Playing format line; reuse the shared pill. NOS/DRE/DSD stay as rows, hidden by capability (not disabled).
- Acceptance criteria:
  - Filters page shows exactly 4 single-select rows with one green check, and the explanatory note visible without scrolling at 480x800.
  - Bit-perfect on visibly dims or marks the dependent rows (EQ, PEQ, MSEB, ReplayGain) with a hint; toggling it off restores them.
  - Capability-hidden rows (DRE on CS43131) are absent, not disabled, and the layout has no gap.
  - Selected filter persists across restart and across track changes during playback without audible dropout beyond the existing DAC reconfigure.

### Name-entry sheet with keyboard (EQ preset save, PEQ preset save, MSEB preset save) and numeric keypad (PEQ gain/Q) - AUD-EQ-PRESET-SAVE, AUD-PEQ-PRESET-SAVE, AUD-MSEB-PRESET-SAVE, AUD-PEQ-KEYPAD

- Audit IDs: AUD-EQ-PRESET-SAVE, AUD-PEQ-PRESET-SAVE, AUD-MSEB-PRESET-SAVE, AUD-PEQ-KEYPAD
- Research date 2026-11-02; context: Spotify mobile (iOS and Android), playlist create/rename. Create: 'Create' (+) > Playlist > name > Create (support article). Rename: three-dot menu > Edit playlist (or tap the title) > edit name > Save (community/tech press; Android Authority and Headphonesty pages blocked). Rename Blend playlists reported unavailable (2022). Exact sheet styling (full screen vs bottom sheet) not verified visually.
- Sources:
  - [Spotify Support - Create playlists](https://support.spotify.com/us/article/create-playlists/), published undated (current), accessed 2026-11-02, read
  - [Android Authority - Change playlist name on Spotify](https://androidauthority.com/change-playlist-name-spotify-3081864), published unknown, accessed 2026-11-02, snippet
  - [Headphonesty - Change Spotify playlist names](https://www.headphonesty.com/2023/11/change-spotify-playlist-names/), published 2023-11, accessed 2026-11-02, snippet
  - [Spotify Community - Rename playlists on mobile](https://community.spotify.com/t5/Live-Ideas/Rename-playlists-on-mobile/idi-p/812727), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Spotify's name prompt is a dedicated full-height screen/sheet on a dark background with a heading ('Give your playlist a name' per the task's reference; not confirmed by a readable source), a large centred text field with the name selected, the platform keyboard, and a pill action (green Create or Save). Official support text confirms only the Create button after naming. No numeric-keypad counterpart exists in Spotify.
- Observed behavior: Open > type > primary pill commits and returns to the originating page; Cancel/back/swipe-down discards. Empty names are not accepted (the commit control is inert or the name defaults). On commit the new item appears at the top of the list. Edit-in-place shows the existing name pre-filled. Spotify does not show a separate success toast for naming.
- Evidence quality: community report
- Conflicts / confidence: Low-medium. The official source covers only the Create button; the Edit/Save flow is from secondary descriptions I could not open; the exact heading text and layout are unverified.
- User requirement overrides: AGENTS.md: features share the keyboard and bottom sheets; the legacy full-screen layers with the mismatched X are not acceptable.
- R1 adaptation: Make the three save flows use one shared 'name sheet': full-screen #121212 layer, 56 px header with a text-only 'Cancel' left and nothing right, centred font_ui_28 white title ('Name your preset'), a text field 56 px high, 24 px side margins, bottom border #1ed760 when focused, font_ui_24 input, and the shared on-screen keyboard docked at the bottom (keyboard.c). A 48 px-high pill 'Save' (green, black text, radius 24) sits above the keyboard; disabled (40 percent) when empty or when the name collides (show 'Name already used' in #e91429? use font_ui_16 red under the field). PEQ writes <sd>/PEQ/<name>.txt, MSEB <sd>/.local/MSEB/<name>.ini: sanitise path characters at input time and cap length at 32. Replace the legacy close X with the Cancel text button (or keep X as a 44 px target). Numeric keypad for PEQ gain/Q: rebuild as a bottom sheet (#202124, radius 16 top corners, 3x4 keys each at least 64x56, key fill #282828, OK pill green) over the band editor, with a value readout in font_ui_28 at the top, +/- toggle for gain, long-press repeat on delete preserved.
- Acceptance criteria:
  - All three save flows open the same sheet with the same layout; only the title and target path differ.
  - Keyboard does not cover the field or Save pill; both stay visible at 480x800.
  - Empty and duplicate names are rejected with inline copy; the Save pill state matches.
  - Back or Cancel discards without writing a file; Save writes exactly one file and shows a toast after returning.
  - Numeric keypad keys are at least 56 px high, delete repeats on hold, and out-of-range values clamp to the slider range before commit.

### Destructive confirmations and item '...' menus (EQ/PEQ/MSEB resets, preset delete, PEQ/MSEB preset lists, DAC leave confirm) - AUD-EQ-RESET-CONFIRM, AUD-EQ-PRESET-DELETE, AUD-PEQ-RESET-CONFIRM, AUD-PEQ-PRESET-LIST, AUD-MSEB-PRESET-LIST, AUD-DAC-LEAVE-CONFIRM

- Audit IDs: AUD-EQ-RESET-CONFIRM, AUD-EQ-PRESET-DELETE, AUD-PEQ-RESET-CONFIRM, AUD-PEQ-PRESET-LIST, AUD-MSEB-PRESET-LIST, AUD-DAC-LEAVE-CONFIRM
- Research date 2026-11-02; context: Spotify mobile iOS and Android, 2025. Playlist delete: Your Library > three-dot on the playlist > 'Delete playlist' > pop-up with Delete (SoundGuys, 2025-08-22). No readable screenshot of the dialog was obtained.
- Sources:
  - [SoundGuys - How to delete a playlist on Spotify](https://soundguys.com/how-to-delete-playlist-on-spotify-101556), published 2025-08-22, accessed 2026-11-02, read
  - [FreeYourMusic - How to delete a playlist on Spotify](https://freeyourmusic.com/blog/how-to-delete-a-playlist-on-spotify), published unknown, accessed 2026-11-02, snippet
  - [Dummies - How to delete playlists](https://dummies.com/article/technology/digital-audio-radio/spotify/how-to-delete-playlists-playlist-tracks-and-playlist-folders-179691), published older, undated, accessed 2026-11-02, snippet
- Observed appearance: Row-level '...' (three horizontal dots on Spotify mobile; the R1 uses a vertical ellipsis) opens a bottom sheet listing actions, each with a leading 24 px icon, white font_ui_18 label, destructive actions in the same list (Spotify does not colour them red as far as sources show). Delete then raises a centred modal dialog with a short question and two buttons; Delete is the confirming button. Exact wording (e.g. 'Delete from Your Library?' / 'Remove') and button colours are unverified.
- Observed behavior: The sheet dismisses on selection; the dialog needs an explicit second tap, tap-outside cancels. After deletion the list updates immediately and a snackbar-style confirmation may appear. Spotify's confirmation text is brief and names the item. Spotify has no reset-to-default confirmations in the EQ; they are R1-only.
- Evidence quality: community report
- Conflicts / confidence: Low. Only the flow (menu > Delete playlist > Delete in pop-up) is read from a dated 2025 article; the dialog copy and styling are not documented in an official source. An older Dummies description cites 'Delete'/'Keep' buttons (unread).
- User requirement overrides: AGENTS.md: shared bottom sheets; collection changes must never delete audio files.
- R1 adaptation: One shared destructive dialog: centred on #000 60 percent scrim, card #282828 radius 16, 24 px padding, width 400 px, title font_ui_20 white (e.g. 'Delete "Warm" preset?'), body font_ui_16 #b3b3b3 one or two lines ('This removes the file from the card. This cannot be undone.'), two 48 px pills side by side: 'Cancel' (outlined white) and the action (white fill, black text; use #1ed760 only for non-destructive confirmations). DAC leave confirm uses the same component. Row '...' (44x44 hit area) opens the shared bottom sheet (#202124, 16 px top radius, drag handle 4x32, item rows 56 px with 24 px icon, a header row showing the preset name and file kind) with Load / Rename (optional) / Delete. Replace the legacy popover. Reset confirmations keep the item counts but sit in the same dialog. Deleting a preset removes only the preset file, never audio files, as required by AGENTS.md.
- Acceptance criteria:
  - All destructive confirms in the audio area use the one dialog component: same width, buttons, scrim and tap-outside-cancels.
  - Preset row '...' opens the shared bottom sheet; no legacy popover remains.
  - Deleting a preset removes only that file, refreshes the list preserving scroll position, and shows an empty-state when none remain.
  - DAC leave confirm cancels on back/scrim and 'Leave' stops the DAC then navigates back exactly once.
  - Dialog buttons are at least 48 px high and 44 px apart at 480x800.

### Connect device picker / device states for USB DAC page - AUD-DAC-PAGE, AUD-DAC-STATES, AUD-DAC-LEAVE-CONFIRM

- Audit IDs: AUD-DAC-PAGE, AUD-DAC-STATES, AUD-DAC-LEAVE-CONFIRM
- Research date 2026-11-02; context: Spotify mobile (iOS and Android) Connect device picker. Official support text only; no readable capture of the sheet. iOS needs local network access; the 'Local device visibility' toggle is under Settings > Apps and devices.
- Sources:
  - [Spotify Support - Spotify Connect](https://support.spotify.com/us/article/spotify-connect), published undated (current), accessed 2026-11-02, read
  - [Spotify Newsroom - Lossless (Lossless indicator also in the Connect picker)](https://newsroom.spotify.com/2025-09-10/lossless-listening-arrives-on-spotify-premium-with-a-richer-more-detailed-listening-experience/), published 2025-09-10, accessed 2026-11-02, read
- Observed appearance: A 'Connect device' icon sits at the bottom of the Now Playing screen and in the Now Playing bar; it opens a bottom-anchored picker listing 'This iPhone/phone' and nearby devices, with the active device highlighted (green) and a Lossless badge when applicable. The support page does not describe a 'connecting' state, a green icon or other indicators, so those cannot be cited from it.
- Observed behavior: Tap icon > pick device > playback transfers. Hardware volume buttons show a volume slider on iOS. Devices not supporting current Spotify show 'No longer works with Spotify'. After more than 10 minutes paused the user may need to reconnect. Audio settings cannot be changed while the Connect target is another device. No explicit 'leave device' confirm exists in Spotify.
- Evidence quality: documented
- Conflicts / confidence: Medium for flow, low for visuals. R1 DAC mode is a hardware USB gadget mode, not a network picker, so only the state vocabulary and active-device highlight transfer.
- User requirement overrides: None recorded.
- R1 adaptation: Rebuild the DAC page as a Spotify page: header 'USB DAC' with back; a hero card (#202124, radius 16, 24 px padding) with a 64 px icon (green when streaming, grey otherwise), title font_ui_24 'DAC mode' and the format line (e.g. '44.1 kHz / 32-bit' or 'DSD64') in font_ui_16 #b3b3b3; below it a 64 px full-row 'DAC mode' switch row. Status text uses a fixed 2-line area with explicit copy and colours: off (grey, dac_note), turning on (grey with spinner), waiting for host (#b3b3b3 plus pulsing dot), streaming (#1ed760 'Playing' with the rate), error (#e91429 with Retry link). Corner charge toggle becomes a second switch row 'Charge device' (hidden if unsupported). Leaving while on uses the shared confirm dialog. Entry from Tools > More > DAC tile should be replaced by a row in the Spotify-style Playback/Music settings page (a 'USB DAC' row with On/Off summary).
- Acceptance criteria:
  - All five states render distinct copy and colour at 480x800 without overlap.
  - Whole 'DAC mode' row is the hit target (at least 64 px high).
  - Back or swipe while DAC is on opens the shared confirm; 'Leave' stops DAC then navigates once; cancel keeps the state.
  - Hero icon and status update within one frame of the DAC state callback with no UI-thread blocking.
  - The page is reachable without passing through the legacy Tools grid.

### Quick-panel effect buttons (MSEB/EQ/PEQ round toggles in Control Centre) and Now Playing sound shortcuts - AUD-QP-EFFECT-BUTTONS

- Audit IDs: AUD-QP-EFFECT-BUTTONS
- Research date 2026-11-02; context: No Spotify counterpart: Spotify mobile has no quick-settings tile for EQ and Spotify's EQ cannot be changed while Connect targets another device. Closest analogous patterns are Spotify toggle chips/pills and the Now Playing '...' sheet. Research on the quick-panel pattern itself: unavailable.
- Sources:
  - [Spotify Support - Equalizer (EQ reachable only through Settings and privacy > Playback)](https://support.spotify.com/us/article/equalizer/), published undated (current), accessed 2026-11-02, read
- Observed appearance: None for quick toggles. Spotify uses small pill chips (rounded, #282828 unselected, #1ed760 selected with black label) for filters in Home, Search, and Library. Round icon buttons exist for Shuffle/Repeat in Now Playing with a green tint plus dot under the icon when active.
- Observed behavior: Active state is indicated by green tint and a small dot (Shuffle/Repeat pattern); single tap toggles. Long-press is not a documented Spotify pattern for entering settings.
- Evidence quality: inferred
- Conflicts / confidence: Low. The pattern (green tint plus dot for active toggles) is from general knowledge of the app, not a fetched source. No dated visual evidence recorded.
- User requirement overrides: None recorded; R1-only feature documented as an adaptation per AGENTS.md.
- R1 adaptation: Keep the Control Centre but restyle the three buttons as 64x64 circles on #282828, icon 28 px white; when On the circle is #1ed760 with black icon (and a 6 px dot below for accessibility). Add a font_ui_16 caption under each ('EQ', 'PEQ', 'MSEB'). Single tap toggles; since long-press is hidden behaviour, also add a visible chevron or 'Edit' affordance, or open the page by tapping the caption; the page route uses the Spotify-style effect page and returns to the Control Centre. Toggle during playback must be glitch-free and persist; PEQ and EQ both on should show the shared headroom warning in a snackbar.
- Acceptance criteria:
  - Each round button hit area is at least 64x64 and has a visible on/off state without relying on colour alone.
  - Opening an effect page from the Control Centre and pressing back returns to the Control Centre with the pull-down state intact.
  - Toggling while audio plays does not interrupt playback.
  - State matches the one in the effect page switch after returning.

### R1-only parametric tools: PEQ main/band editor, MSEB page and settings, preset load lists - AUD-PEQ-MAIN, AUD-PEQ-BAND, AUD-PEQ-SETTINGS, AUD-PEQ-PRESET-LIST, AUD-MSEB-PAGE, AUD-MSEB-SETTINGS, AUD-MSEB-PRESET-LIST

- Audit IDs: AUD-PEQ-MAIN, AUD-PEQ-BAND, AUD-PEQ-SETTINGS, AUD-PEQ-PRESET-LIST, AUD-MSEB-PAGE, AUD-MSEB-SETTINGS, AUD-MSEB-PRESET-LIST
- Research date 2026-11-02; context: None (R1-only). Spotify's iOS EQ is a simplified graph with preset list; it has no parametric bands, filter shapes, Q, preamp or bipolar character sliders. Analogous patterns taken from Spotify's EQ page, Your Library rows and Media quality lists. Spotify's slider visual (Now Playing seek and volume: thin 4 px track, white thumb, green fill) was not re-verified in this session.
- Sources:
  - [Spotify Support - Equalizer](https://support.spotify.com/us/article/equalizer/), published undated (current), accessed 2026-11-02, read
  - [Audio Solace - How To Use Spotify Equalizer ('simplified parametric EQ')](https://audiosolace.com/how-to-use-spotify-equalizer/), published 2021-12-01, accessed 2026-11-02, read
  - [Figma blog - Creating coherence: Spotify's design system (Encore Mobile)](https://www.figma.com/ja-jp/blog/creating-coherence-how-spotifys-design-system-goes-beyond-platforms/), published unknown, accessed 2026-11-02, read
- Observed appearance: Not applicable to Spotify. The Spotify design system (Encore) is token-based and shares colour/typography/spacing across mobile and web, with a unified primary/secondary/tertiary button hierarchy (Figma blog). That supports using one shared set of tokens, pills and buttons rather than per-screen colours.
- Observed behavior: Not applicable. Analogous: preset rows apply on tap; value editing by dragging; compact segmented choices as chips.
- Evidence quality: inferred
- Conflicts / confidence: Low. Nothing in Spotify defines these controls; the adaptation relies on shared tokens and component hierarchy only. The Figma blog does not describe bottom sheets, dialogs or sliders.
- User requirement overrides: AGENTS.md: R1-only functions must use the same visual language and be documented as adaptations; bounded rows required.
- R1 adaptation: PEQ main: header 'Parametric EQ' with '...' (Presets, Reset), On switch row, response chart card (#202124, radius 12, 160 px high, #1ed760 curve, grid #282828), Preamp slider row (label plus value chip, 4 px track, 20 px thumb with 44 px hit area), auto-headroom as a caption, then ten 64 px 'Band N' navigation rows with a one-line summary (e.g. 'Peak 1.0 kHz +3.0 dB Q 1.0') in #b3b3b3 and a chevron. Band editor: title 'Band N', On switch row, Shape as a horizontally scrollable chip row (36 px chips: Peak, Low shelf, High shelf, Low pass, High pass), Frequency/Gain/Q slider rows each with a tappable value chip opening the shared keypad sheet; Gain row hidden for pass filters with no gap. MSEB: On row plus 10 bipolar sliders as 56 px rows (label left, centre-zero track, value right) with knob-only hit-test replaced by a full 44 px-tall row hit area; range choice (-20..+20, -40..+40, -100..+100) uses the chip row. Preset Load lists use 64 px rows (name plus path caption) with the shared '...' sheet; empty state centred with a 48 px icon, font_ui_20 title and font_ui_16 hint plus a Save-preset pill. Partly-loaded notice becomes a snackbar with 'Details' rather than a popup.
- Acceptance criteria:
  - PEQ band rows (10) fit as a scrolling list with no per-row custom widgets beyond label, summary and chevron; scrolling stays smooth with audio playing.
  - Band editor shows Gain row only for filter types that use it and the layout reflows without leaving empty space.
  - Sliders have 44 px minimum vertical hit area and a visible numeric value; keypad edit clamps to the allowed range.
  - Preset list shows at most 512 PEQ entries via a bounded/paginated list and shows the empty state when none exist.
  - All pages use shared header, row, chip and sheet components with no legacy topbar.

### Metronome and time-signature picker - AUD-METRONOME, AUD-METRONOME-BEATS

- Audit IDs: AUD-METRONOME, AUD-METRONOME-BEATS
- Research date 2026-11-02; context: None (R1-only). Spotify has no metronome. Closest patterns: Spotify's sleep timer options list (music: 5 to 60 min or End of track, opened from the Now Playing '...' menu, per 2025 coverage) and the selected-option bottom sheet.
- Sources:
  - [Spotify Newsroom - sleep tag (sleep timer controls, 2025-08-29 update)](https://newsroom.spotify.com/tag/sleep), published 2025-08-29, accessed 2026-11-02, snippet
  - [FreeYourMusic - How to set a Spotify sleep timer](https://freeyourmusic.com/blog/how-to-set-spotify-sleep-timer), published updated 2026-09, accessed 2026-11-02, snippet
- Observed appearance: Search snippets only: the sleep timer is a bottom sheet listing durations with the active one checked and a 'Turn off timer' row. Visual details not verified.
- Observed behavior: Tap an option, sheet closes, selection stored; Spotify has no continuous tempo control to compare against.
- Evidence quality: unavailable
- Conflicts / confidence: Very low. Only search snippets seen; the bottom sheet appearance is not backed by a read page. Treat as an adaptation, not Spotify fidelity.
- User requirement overrides: None recorded.
- R1 adaptation: Metronome page in Spotify language: header 'Metronome' with back; large tempo hero (font_ui_28 white BPM, tempo name in #b3b3b3 font_ui_16) over the pendulum drawing on #202124; beat dots 12 px (current beat #1ed760, accent beat larger); round -/+ buttons 56 px with hold-repeat (existing); BPM slider with 44 px hit area; 'Tap tempo' as a 44 px outlined pill; play/pause as a 64 px green circle with black glyph (same as Spotify's Now Playing play button); time signature pill opens the shared bottom sheet (#202124) with options 1/4 to 7/8 as 56 px rows and a green check on the current one, replacing the legacy popover. Pausing audio is not required; document that the metronome click shares the audio output.
- Acceptance criteria:
  - Time-signature picker is the shared bottom sheet, 7 rows, one green check, tap picks and dismisses.
  - -/+ buttons repeat on hold and the BPM readout updates within a frame; BPM clamps to the min/max.
  - Play/pause is a 64 px circle matching the mini-player play button style.
  - Page has no legacy topbar; back returns to its entry route.

### Toasts and transient feedback (preset applied, saved, deleted, partly loaded, errors)

- Audit IDs: AUD-EQ-PRESETS, AUD-EQ-PRESET-SAVE, AUD-PEQ-PRESET-LIST, AUD-MSEB-PRESET-LIST
- Research date 2026-11-02; context: Spotify mobile snackbars/toasts. No dedicated Spotify source on appearance was found or read in this session.
- Sources:
  - [Spotify Support - Create playlists (no snackbar described)](https://support.spotify.com/us/article/create-playlists/), published undated (current), accessed 2026-11-02, read
- Observed appearance: Unverified. From general knowledge the confirmations are short dark-grey bars near the bottom above the mini-player (for example 'Added to Liked Songs') with optional action text. No fetched evidence.
- Observed behavior: Unverified from sources.
- Evidence quality: unavailable
- Conflicts / confidence: No evidence. Do not claim fidelity for toast styling from this research.
- User requirement overrides: None recorded.
- R1 adaptation: Use the shared toast: #282828 pill, radius 8, font_ui_16 white, 48 px high, 16 px side margins, positioned 12 px above the mini-player/footer, auto-dismiss 2.5 s, optional green text action ('Undo' for delete, 'Details' for partial load). Preset apply from EQ/PEQ/MSEB lists should toast and remain on the page (see Equalizer record) rather than silently navigate back.
- Acceptance criteria:
  - All audio preset operations use the shared toast component, never the legacy popup.
  - Toast never overlaps the mini-player or keyboard.
  - A repeated action replaces the active toast instead of stacking.

Failed attempts:

- WebFetch https://www.techradar.com/news/software/applications/spotify-for-iphone-and-ipad-revives-the-boombox-glory-days-with-new-equaliser-1259535 returned only the site header (body truncated); no EQ UI details obtained.
- WebFetch https://www.tomsguide.com/how-to-use-the-spotify-equalizer-on-ios-and-android returned only the title; content not available.
- WebFetch https://community.spotify.com/t5/FAQs/Equalizer-Overview/ta-p/4666978 returned only the title; the official FAQ body could not be read, so band count/preset list remain from a search snippet.
- WebFetch https://community.spotify.com/t5/iOS-iPhone-iPad/More-equalizer-bands-and-custom-EQ-presets-for-iOS/m-p/5857684/highlight/true returned only the title.
- WebFetch https://www.phonearena.com/news/Spotify-for-iOS-scores-an-equalizer-with-a-lot-of-presets-in-tow-at-long-last_id58733 returned HTTP 403.
- WebFetch https://www.headphonesty.com/2022/03/best-spotify-equalizer-settings/ and /2023/11/change-spotify-playlist-names/ returned HTTP 403; claims (iOS 6 bands vs Android 5, rename flow) are from search snippets only.
- WebFetch https://androidauthority.com/change-playlist-name-spotify-3081864 returned HTTP 403.
- WebFetch https://www.digitalcitizen.life/spotify-settings-tricks/ and https://techpp.com/?p=239756 returned HTTP 403.
- WebFetch https://support.spotify.com/us/article/playback-settings/ and /lossless/ were 404 'Page not found' (wrong URL guesses); a Playback settings support article listing Crossfade/Gapless/Normalize volume/Mono audio was not found.
- WebFetch https://newsroom.spotify.com/2025-09-10/spotify-music-pro-lossless/ returned 404 (guessed URL); the correct newsroom URL was found via search and read.
- No dated mobile screenshot or YouTube video page of the Spotify iOS/Android Equalizer screen, Settings rows, Connect picker, delete confirmation dialog, name-entry screen or snackbars could be retrieved; YouTube search returned no video pages. All geometry in the adaptations is therefore inferred, not measured from Spotify.
- spotify.design and Encore documentation for mobile bottom sheets, dialogs, sliders and toggles could not be found (search returned third-party summaries; the Figma blog post read does not cover them).
- No Spotify source exists for a metronome or quick-settings effect toggles; the sleep timer sheet pattern was seen only in search snippets (not read).
- I did not read repository files (task scope was web research); R1 metrics such as the 64 px row and 56 px header were not checked against gui/shell/ui_metrics.h and are proposals.

## connectivity

### Devices / Connect picker (entry, device list, current device, section structure) - covers SHELL-TOOLS-WIRELESS, SHELL-QP-RADIOS, BT-ROOT, SONIXLINK-PAGE, SONIXLINK-REMOTE

- Audit IDs: n/a
- Research date 2026-11-02; context: Android and iOS mobile. Spotify support text is undated in the body; Connect article and Jam article appear to be current 2025-26 revisions (search tool reported the Connect article as about four years old, so label text may have drifted). Premium needed for hosting Jam. No experiment info available. Exact app version unknown.
- Sources:
  - [Spotify Connect - Spotify Support](https://support.spotify.com/us/article/spotify-connect/), published undated (support article, current revision), accessed 2026-11-02, read
  - [Start or join a Jam - Spotify Support](https://support.spotify.com/us/article/jam/), published undated, accessed 2026-11-02, read
  - [How to Use Spotify Connect, Your Ultimate Listening Remote](https://newsroom.spotify.com/2020-06-18/spotify-connect-your-ultimate-listening-remote/), published 2020-06-18, accessed 2026-11-02, read
  - [Spotify's latest shortcut should help streamline casting - Android Police](https://www.androidpolice.com/2020/09/28/spotifys-latest-shortcut-should-help-streamline-casting/), published 2020-09-28, accessed 2026-11-02, read
  - [How to use Spotify on multiple devices (third party)](https://www.theory7.net/support/en-us/tips-tricks/how-to-use-spotify-on-multiple-devices), published updated 2026-09-24, accessed 2026-11-02, read
  - [Spotify Testing New Nearby Device Feature - Thurrott](https://www.thurrott.com/?p=212087), published 2019-08-13, accessed 2026-11-02, read
  - [Select a (Connect) device on Android - Spotify Community](https://community.spotify.com/t5/Android/Select-a-Connect-device-on-Android/m-p/5073979/highlight/true), published unknown, accessed 2026-11-02, snippet
  - [New Jam screen hides wifi speakers suddenly after last app upgrade - Spotify Community](https://community.spotify.com/t5/Android/New-Jam-screen-hides-wifi-speakers-suddenly-after-last-app/m-p/6335786/highlight/true), published unknown, accessed 2026-11-02, snippet
  - [Put the old Connect to device layout back - Spotify Community](https://community.spotify.com/t5/Other-Podcasts-Partners-etc/Put-the-old-Connect-to-device-layout-back/td-p/6179201), published unknown, accessed 2026-11-02, snippet
- Observed appearance: No source I could read gives pixel-level or dated screenshot evidence of the picker. Documented facts only: the picker opens from a device (speaker/screen) icon in the bottom playback bar / Now Playing screen (support alt text: 'Connect device'); Android Police 2020-09-28 confirms the icon sits left of the heart in the Now Playing bar and that the previous non-tappable 'devices available' second line was replaced by it. Support says 'Pick the device you want to play on'. A tappable sheet lists devices (this phone, Wi-Fi Connect speakers/TVs, other logged-in devices); the three-dot menu next to a device offers 'Log out of this device'. In the picker, Jam rows appear: 'Start a Jam', 'Invite', a switch 'Let others Jam on this speaker', and a Bluetooth-proximity-invitation toggle (label not documented). The 'Change quality settings' action appears after selecting a playing device (Lossless article). Green highlight on the active device and 'Listening on' wording are NOT confirmed by any source read (newsroom 2020 explicitly lacks them). Section headings in the support article are 'Use Spotify Connect', 'Don't see your device?', 'Connect not working?', 'Don't recognize a device?'.
- Observed behavior: Tap Connect icon -> choose a device -> playback transfers and the player becomes a remote ('Your display will automatically change to a remote', 2024 third-party guide via search snippet only). All devices must share the same Wi-Fi for first connect. Settings > Apps and devices has a 'Local device visibility' toggle (support text contradicts itself on on/off semantics; community thread label is 'Show local devices only'). iOS requires Settings > Spotify > Local Network permission. Bluetooth is NOT a Connect transport: Spotify's own Bluetooth help says pair in the OS Bluetooth settings first, then play; the app does not manage pairing. Duplicate-named devices happen; guidance is 'try connecting to each in turn'. Unsupported devices show 'No longer works with Spotify'. A 2019 test (Thurrott, 2019-08-13) added a favourite-device nearby prompt; not confirmed shipped.
- Evidence quality: documented
- Conflicts / confidence: Medium for entry point and flow (multiple consistent support sources); LOW for visual layout (no dated screenshot read; community threads returned titles only). Conflicts: support says Local device visibility off => see devices off-Wi-Fi in one section and on => local only in another; community label 'Show local devices only' under Settings > Devices versus support 'Settings > Apps and devices'. Third-party guide places the device icon in the full Now Playing screen, support says bottom of screen. Presented as unverified: any 'Listening on' header, any Bluetooth section inside the picker.
- User requirement overrides: none recorded
- R1 adaptation: Replace the legacy Wireless grid with one 'Devices' page in the Spotify settings shell (480x800, bg #121212). Header: back chevron 48x48 + title 'Devices' (font_ui_24). Rows 72 px tall, 16 px side gutter, font_ui_18 primary (#fff) / font_ui_16 secondary (#b3b3b3), trailing 44x44 hit area for status/toggle. Rows: Wi-Fi (value = SSID or 'Off'), Bluetooth (value = connected device or 'Off'), AirPlay, SonixLink, Wi-Fi Transfer, each with a leading 40 px glyph and trailing chevron; active/connected row text and glyph #1ed760, otherwise #fff. Because Spotify has no Bluetooth-in-picker, keep the device list as a Settings-style page rather than a fake Connect sheet; add a short secondary hint line (font_ui_16 #b3b3b3) under each name. Mini-player gets a speaker icon (44x44) that opens a bottom sheet (#202124, top corners 16 px, scrim 60%) titled 'Connect to a device' with the currently active output (green), then Bluetooth/AirPlay/SonixLink outputs, and a 'Manage devices' row that navigates to the Devices page. Pull-down control centre circles (SHELL-QP-RADIOS) must be restyled as a #282828 raised sheet with 56 px circles, green when on, and long-press must open the new pages, not legacy ones. SonixLink should read 'Listening on <phone>' only if that wording is introduced as an R1 adaptation and documented.
- Acceptance criteria:
  - No route from Home/Search ellipsis to Wi-Fi, Bluetooth, AirPlay, SonixLink or Transfer displays a gridpage tile screen; all use the shared settings-row page with #121212 background.
  - Devices page rows are >=64 px tall (target 72), have >=44x44 touch targets, and show live status values that match the radio state.
  - The active/connected row text or glyph uses #1ed760 and inactive uses #fff or #b3b3b3 with contrast >=4.5:1 on #121212.
  - Control-centre circles and long-press targets open the Spotify-style pages and the mini-player stays visible on every Devices subpage where playback is active.
  - Rendering at 480x800 shows no clipped or overlapped text for the longest SSID/device name (ellipsis applied).
  - SonixLink remote commands continue to work with the Devices page closed (SONIXLINK-REMOTE regression).

### Settings root and list rows with toggles and value rows (Settings > Apps and devices style) - covers SHELL-OPEN-WIFI-DEAD, WIFI-ROOT toggle card, BTAUDIO-PAGE, TRANSFER-PAGE toggle, AIRPLAY-PAGE toggle, AIRPODS-HOLD

- Audit IDs: n/a
- Research date 2026-11-02; context: Android and iOS mobile; SlashGear article dated 2025-09-14 (Android, mirrors the profile -> Settings and privacy route). Support articles undated. Labels shown are English US.
- Sources:
  - [How To Open And Adjust Your Spotify Settings On Android - SlashGear](https://www.slashgear.com/1965140/how-to-open-spotify-settings-on-android), published 2025-09-14, accessed 2026-11-02, read
  - [Internet and data usage - Spotify Support](https://support.spotify.com/us/article/internet-and-data-usage/), published undated, accessed 2026-11-02, read
  - [Local files - Spotify Support](https://support.spotify.com/us/article/local-files/), published undated, accessed 2026-11-02, read
  - [Audio quality - Spotify Support](https://support.spotify.com/us/article/audio-quality/), published undated, accessed 2026-11-02, read
- Observed appearance: Settings is reached by tapping the profile picture at top-left of Home or Your Library, then 'Settings and privacy' (SlashGear 2025-09-14). Top-level section list: Account, Content and display, Playback, Privacy and social, Notifications, Apps and devices, Data-saving and offline, Media quality, Advertisements, About and support, with a 'Go Premium' button at the very top. Sub-pages (Data-saving and offline) contain labelled on/off switches: 'Data saver', 'Audio-only streaming', 'Downloads over cellular'. Apps and devices contains a 'Local audio files' toggle (local-files support article). Row visual specifics (height, divider, chevron, secondary text) are not described in any source read; support text implies a title + switch rows and an action 'Clear' next to 'Clear cache'.
- Observed behavior: Sections are drill-in pages; toggles apply immediately (no Save). Spotify toggles under Apps and devices include Local audio files and local device visibility. No source documents exact transitions or haptics.
- Evidence quality: documented
- Conflicts / confidence: High for the route and section names (SlashGear 2025 matches support); Low for row geometry (not described by any source read, treat from-memory visuals as unverified). The label 'Data Saver' became 'Data-saving and offline' per a community snippet and support page.
- User requirement overrides: none recorded
- R1 adaptation: Use the shared settings page contract: title bar 56 px (back 48x48, title font_ui_24, left aligned), then optional section label (font_ui_16, #b3b3b3, 24 px top gap), then rows 64-72 px: title font_ui_18 #fff, secondary line font_ui_16 #b3b3b3, trailing LVGL switch 52x32 with #1ed760 on-state and #535353 off track, or value text + chevron. Toggle 'cards' on Wi-Fi, AirPlay, Transfer, SonixLink should become the first row of the page (title 'Wi-Fi', switch right) instead of a custom card. Keep a single dividerless layout, rows on #121212 (not #282828) with pressed state #282828. Wire the unused open_wifi_settings action to the 'Wi-Fi settings' secondary row on AirPlay/Transfer/SonixLink so the dead code becomes a real link, and document the move in the Decisions log.
- Acceptance criteria:
  - Every connectivity page uses settingsrow-based rows with the same height, font sizes and switch style as the Settings root.
  - Switch toggles change state immediately and persist across page re-entry; the toggle reflects hardware state if changed from the control centre.
  - The 'Wi-Fi settings' link rows on AirPlay, Transfer and SonixLink navigate to the Spotify-style Wi-Fi page and return with Back to the originating page.
  - Dead action SHELL-OPEN-WIFI-DEAD is either wired to a reachable control or removed with a log entry.

### Single-choice quality list (Bluetooth codec choice) - covers BTAUDIO-CODEC and quality-related rows on BTAUDIO-PAGE

- Audit IDs: n/a
- Research date 2026-11-02; context: Android and iOS mobile; Spotify support article undated; Lossless article states a 'Change quality settings' action on the Connect picker. iOS label for the section is 'Audio Quality', Android 'Media quality' per search snippet only.
- Sources:
  - [Audio quality - Spotify Support](https://support.spotify.com/us/article/audio-quality/), published undated, accessed 2026-11-02, read
  - [Lossless Audio Quality - Spotify Support](https://support.spotify.com/us/article/lossless-audio-quality/), published undated, accessed 2026-11-02, read
- Observed appearance: Steps: profile picture -> 'Settings and privacy' -> 'Media Quality'. Option names: Automatic, Low (~24 kbit/s), Normal (~96), High (~160), Very high (~320, Premium), Lossless (up to 24-bit/44.1 kHz FLAC, Premium). Separate selections exist for Wi-Fi streaming, cellular streaming and downloads, plus an 'Auto-adjust' switch. Visual treatment (radio ring vs check mark) is not documented by any source read.
- Observed behavior: Quality is chosen per network type; Lossless can also be switched from the Connect picker via 'Change quality settings' > 'Lossless' (mobile only). The connection method (Bluetooth, Connect, AUX) can limit lossless; the article does not say which methods work.
- Evidence quality: documented
- Conflicts / confidence: High for option names; Low for the selection indicator. Whether the choice returns immediately to the previous page is not documented, so the R1 'choose then pop' behavior is an adaptation, not Spotify fidelity.
- User requirement overrides: none recorded
- R1 adaptation: Codec page as a settings sub-page: title 'Codec', rows 72 px with codec name (font_ui_18) and short description (font_ui_16 #b3b3b3, e.g. 'Highest quality, needs strong signal'), trailing radio/check 24 px green #1ed760 on the in-use codec, label 'In use' removed in favour of the check plus secondary text. Order by quality: LDAC 990, LDAC 660, LDAC ABR, aptX HD, aptX, AAC, SBC. Selecting a row applies it, updates the check, shows a bottom snackbar 'Codec set to <name>' and then pops (document the pop as an R1 adaptation). Empty state (no device connected / no codecs): centered icon 48 px, title font_ui_20 'No device connected', body font_ui_16 #b3b3b3.
- Acceptance criteria:
  - Exactly one codec row shows the green check at a time and it matches the currently negotiated codec.
  - Disabled codecs (unsupported by the connected device) are dimmed to #535353 and not selectable, or omitted with the reason in the secondary line.
  - Empty state renders at 480x800 with no legacy empty label and has a visible Back control.
  - Selecting a codec persists across reconnect and is reflected in the BT Audio row value.

### Empty, loading and scanning states (Wi-Fi scanning, Bluetooth scanning, pairing 'Connecting...') - covers WIFI-STATES, BT-SCAN, BT-PAIRING-MODAL

- Audit IDs: n/a
- Research date 2026-11-02; context: Android and iOS. Spotify has no Wi-Fi or Bluetooth scan screen; closest evidence is Connect troubleshooting text and Bluetooth help. 'No devices found' wording is NOT confirmed by any source read.
- Sources:
  - [Spotify Connect - Spotify Support](https://support.spotify.com/us/article/spotify-connect/), published undated, accessed 2026-11-02, read
  - [Bluetooth help for Spotify - Spotify Support](https://support.spotify.com/us/article/bluetooth/), published undated, accessed 2026-11-02, read
  - [Spotify is offline - Spotify Support](https://support.spotify.com/us/article/spotify-is-offline/), published unknown (HTTP 429 on fetch), accessed 2026-11-02, snippet
- Observed appearance: No source describes a Spotify scan spinner or an empty devices list. Troubleshooting guidance reads as headings and body: 'Don't see your device?', 'Connect not working?'. Bluetooth article: pair in OS settings, keep devices ~1 m apart, check media sharing, re-pair via 'Forget'/'Unpair' in the phone settings.
- Observed behavior: Spotify relies on the OS for Bluetooth and Wi-Fi states; the app itself only shows missing-device guidance and an 'offline' notion. The tool cannot confirm the in-app offline banner or retry control wording.
- Evidence quality: inferred
- Conflicts / confidence: Low. The task assumption that Spotify shows 'No devices found' / 'Looking for devices' is not supported by any readable source; treat as R1 copy. Offline-state wording could not be fetched (429).
- User requirement overrides: none recorded
- R1 adaptation: Common state components for Wi-Fi/Bluetooth lists: (1) Loading: 24 px green spinner (#1ed760) inline in a 56 px section header row 'Searching for networks...' (font_ui_16 #b3b3b3), no modal. (2) Empty: centered column, 48 px glyph #535353, title font_ui_20 #fff ('No networks found'), body font_ui_16 #b3b3b3, optional 44 px pill button 'Scan again' (#fff text on #282828, radius 22). (3) Radio off: first row shows the switch Off and the list collapses to a single secondary line 'Turn on Wi-Fi to see networks'. (4) Bluetooth pairing in progress: replace the blocking modal with a bottom sheet (#202124) showing a spinner, 'Connecting to <name>...' and a 'Cancel' text button, with the same 90 s timeout; on failure show snackbar 'Couldn't connect to <name>' and keep the row. Never use a full-screen scrim without Cancel.
- Acceptance criteria:
  - Each of the 6 Wi-Fi states and 4 Bluetooth states in the inventory renders using the shared empty/loading components with no legacy labels.
  - Pairing sheet always shows a working Cancel and dismisses on connect, failure, radio-off and the 90 s timeout.
  - No state leaves the screen with only a spinner and no Back or Cancel path.
  - Scan restarts only on user action or page entry and does not run on the UI thread.

### Overflow ('...') bottom sheet for list items - covers WIFI-ROW-MENU and BT-DEVICE-MENU

- Audit IDs: n/a
- Research date 2026-11-02; context: Android and iOS. Support: three-dot menu is vertical on Android and horizontal on iOS. Device row '...' offers 'Log out of this device' in the Connect picker. No dated screenshot of the sheet was read.
- Sources:
  - [Create and edit playlists - Spotify Support](https://support.spotify.com/us/article/create-playlists/), published undated, accessed 2026-11-02, read
  - [Spotify Connect - Spotify Support](https://support.spotify.com/us/article/spotify-connect/), published undated, accessed 2026-11-02, read
- Observed appearance: Menus are opened via a three-dot icon (vertical Android, horizontal iOS) and in the Connect picker a three-dot next to a device. Sheet visual treatment (header with art/name, row height, icons) is not described by the sources read. Menu item labels from support: 'Add to Playlist', 'Delete Playlist', 'Log out of this device'.
- Observed behavior: Tapping the dots opens a context menu for that item; actions execute and dismiss. Destructive actions in Spotify mobile are generally immediate with a toast (community discussion) rather than confirm dialogs for library removals; delete playlist confirmation not documented.
- Evidence quality: inferred
- Conflicts / confidence: Low-medium. The inventory's popover (anchored menu) is not Spotify-like; Spotify uses a modal bottom sheet. Confidence that actions are immediate comes from community discussion only.
- User requirement overrides: none recorded
- R1 adaptation: Use shared gui/shell/bottomsheet: #202124, top radius 16, 4x32 grab handle #535353, header row (glyph 40 px + item name font_ui_18 + subline font_ui_16 #b3b3b3, e.g. 'Connected - WPA2' or 'Paired'), then action rows 56 px high with leading 24 px icon and font_ui_18 label. Wi-Fi current network: Details, Disconnect, Forget (Forget in #ff5c5c or #fff per decision log). Saved network: Connect, Forget. Bluetooth device: Connect/Disconnect, Rename, Forget. The 3-dot trailing control must be a 44x44 hit area with vertical-dots glyph (Android convention is the project reference). Long-press must open the same sheet. Sheet dismisses on scrim tap, swipe-down, or action. Forget asks no extra dialog but shows snackbar 'Forgot <name>' with no undo (undo is not possible once credentials are erased), or optionally a confirm sheet if the decisions log requires it.
- Acceptance criteria:
  - Wi-Fi and Bluetooth row menus use the shared bottom sheet and never the legacy popover.
  - Tapping the three-dot control, the connected row, or long-press opens the same sheet for the same item.
  - Sheet dismisses by scrim tap, swipe-down and hardware Back without side effects.
  - Action rows are >=56 px high with >=44 px hit area and the sheet respects bottom safe area above the mini-player.

### Text entry (Wi-Fi password, Bluetooth device rename, player name) with keyboard and field - covers WIFI-PASSWORD, BT-RENAME-DEVICE, BTAUDIO-NAME

- Audit IDs: n/a
- Research date 2026-11-02; context: Android and iOS mobile. Playlist create flow: 'Create' (plus) -> 'Playlist' -> enter a name -> 'Create'. Rename flow is not documented in the mobile support text I read.
- Sources:
  - [Create and edit playlists - Spotify Support](https://support.spotify.com/us/article/create-playlists/), published undated, accessed 2026-11-02, read
- Observed appearance: Only text-entry evidence read: 'Name your playlist, and tap Create' on mobile. The support article does not describe the field, Save/Done button, header or keyboard placement, and says nothing about a mask/eye toggle (Spotify's own password fields in login are not covered by this area). Spotify's pattern is a full-screen or sheet with a large heading, an underlined/filled field and a prominent pill primary button.
- Observed behavior: Name is submitted via an explicit 'Create' button; renaming and the confirm dialogs for deletion are not covered by the source. Cancel/back dismisses without saving.
- Evidence quality: inferred
- Conflicts / confidence: Low. Visual fidelity cannot be asserted from sources; the R1 keyboard OK action is an acceptable adaptation. Needs a dated screenshot of Spotify's playlist-name screen (not obtained).
- User requirement overrides: none recorded
- R1 adaptation: Single shared full-screen 'text entry' page on #121212: top bar with X close (48x48, left) and title font_ui_20 (e.g. 'Network password' / 'Rename'); subline font_ui_16 #b3b3b3 with SSID or device address; field 56 px high, fill #282828, radius 8, font_ui_20, caret #1ed760; password field adds a 44x44 eye toggle inside the field's trailing edge. Primary action: pill button 48 px high, #1ed760 bg, #000 text font_ui_18 'Connect' / 'Save' placed above the keyboard; disabled until the field is non-empty (but for rename empty input restores advertised name - show helper 'Leave empty to use default name'). Keyboard (gui/shell/keyboard.c) occupies the bottom 300 px; the layout must not shrink the field below 56 px. Wrong password: inline error text #ff5c5c under field 'Wrong password. Try again.' instead of silently re-opening. No native-title bar.
- Acceptance criteria:
  - All three text-entry journeys share one layout with identical field, button, close control and keyboard behavior.
  - Keyboard OK and the primary button perform the same action; Back/X discards without side effects.
  - Eye toggle changes masking and has >=44x44 hit area; masked text is never logged or exposed.
  - With the keyboard visible at 480x800 the field, helper/error text and primary button are all visible and non-overlapping.
  - A wrong password shows inline error and keeps the user on the page.

### Confirm dialogs (leave transfer server running, leave receiver mode)

- Audit IDs: n/a
- Research date 2026-11-02; context: Android and iOS. No Spotify support or newsroom source read documents a confirm dialog for any action; delete-playlist confirmation is not covered by create-playlists article. Community ideas thread requests confirm/undo for removing liked songs, implying none exists.
- Sources:
  - [Create and edit playlists - Spotify Support](https://support.spotify.com/us/article/create-playlists/), published undated, accessed 2026-11-02, read
  - [Your Library: Confirm to Delete/dislike or an Undo button - Spotify Community idea](https://community.spotify.com/t5/Live-Ideas/Your-Library-Confirm-to-Delete-dislike-or-an-Undo-button/idi-p/4868440), published unknown, accessed 2026-11-02, snippet
  - [Spicetify react components (unofficial; mentions Spotify desktop confirm dialog)](https://spicetify.app/docs/development/api-wrapper/properties/react-components), published unknown, accessed 2026-11-02, snippet
- Observed appearance: No readable mobile evidence. Search snippets only: Spotify desktop has a confirmation dialog for playlist/track removal (unofficial Spicetify docs). Mobile appearance of a centered dialog is therefore unverified.
- Observed behavior: Spotify mobile mostly performs library changes immediately with a toast (per community requests for undo). Confirm dialogs exist for some destructive account/playlist actions but not documented here.
- Evidence quality: unavailable
- Conflicts / confidence: Very low. Do not claim Spotify fidelity for dialog layout; record as R1 adaptation with no Spotify citation. A leave-confirm is needed because leaving would stop a service (transfer server, receiver) - a real side effect that Spotify has no counterpart for.
- User requirement overrides: none recorded
- R1 adaptation: Use a modal centered dialog sheet from the shared component: card #282828, radius 12, width 400 of 480 (40 px margins), padding 24; title font_ui_20 #fff ('Stop transferring?' / 'Stop receiving?'), body font_ui_16 #b3b3b3 ('Turning off stops the web server and disconnects your computer.'), two buttons stacked or right-aligned: secondary text button 'Cancel' (#fff) and primary pill 'Turn off' / 'Leave' (#1ed760 bg, #000 text), both 48 px high. Scrim black 60%. Hardware Back selects Cancel. Do not use red for the primary because the action is a service stop, not a data deletion.
- Acceptance criteria:
  - Dialog appears on back button, edge swipe and header Back while the service is active, and does not appear when inactive.
  - Cancel keeps the service and page state; Turn off/Leave stops the service then navigates back.
  - Dialog buttons are >=48 px high and the dialog is fully visible at 480x800 and uses the shared dialog component.
  - No orphan service remains after confirming (thttpd stopped / receiver released).

### Snackbars / toasts for result messages (connected, forgotten, disconnected, failed, codec set)

- Audit IDs: n/a
- Research date 2026-11-02; context: Android and iOS. Community reports (undated search snippets) describe an 'Added to ...' toast with duration around 5 s that can block UI beneath it; users request Undo. Official Spotify/Encore snackbar spec could not be found (spotify.design redirects to open.spotify.com).
- Sources:
  - [Mobile: Prevent/fix 'Added to' message blocking Done button UI - Spotify Community idea](https://community.spotify.com/t5/Live-Ideas/Mobile-Prevent-fix-Added-to-message-blocking-Done-button-UI/idi-p/7460074), published unknown, accessed 2026-11-02, snippet
  - [Material Design Snackbars and toasts (general reference only)](https://m1.material.io/components/snackbars-toasts.html), published unknown, accessed 2026-11-02, snippet
  - [Spotify Design (redirected to open.spotify.com)](https://spotify.design/), published n/a, accessed 2026-11-02, read
- Observed appearance: Not directly verified. Snippet-only: short bottom message, e.g. 'Added to Liked Songs' / 'Added to <playlist>', can overlap bottom controls for ~5 s.
- Observed behavior: Transient, non-blocking, auto-dismissing; users ask for Undo on library actions (suggesting it is not currently universal).
- Evidence quality: community report
- Conflicts / confidence: Low. Only community snippets with unknown dates; do not cite as Spotify spec. Specific duration (5 s) is a user's observation.
- User requirement overrides: none recorded
- R1 adaptation: Shared snackbar: a pill/card #282828 (or #3a3a3a) at the bottom, 16 px side margin, 48 px high, radius 8, font_ui_16 #fff, one line, ellipsis; positioned 12 px ABOVE the mini-player (not covering it or the keyboard action bar), 3 s auto-dismiss (4 s if it has an action), one at a time with new replacing old. Optional right action text in #1ed760 ('Undo', only where reversible; Forget network and Disconnect have no Undo). Use for: Connected to <SSID>, Disconnected, Forgot <name>, Couldn't connect, Codec set. Failure messages use the same container with a leading 20 px warning glyph and secondary text color #b3b3b3 for detail, never a legacy popup.
- Acceptance criteria:
  - Every connectivity result in WIFI-STATES, BT-DEVICE-MENU, BTAUDIO-CODEC and BTAUDIO-NAME triggers the shared snackbar and no legacy message box.
  - Snackbar never overlaps the mini-player, keyboard, or primary page action; it is auto-dismissed after 3-4 s.
  - A new snackbar replaces the previous one without stacking and does not block touch input to underlying controls.

### Local files / transfer / import (Wi-Fi Transfer page, TRANSFER-PAGE) - closest Spotify counterpart for R1-only function

- Audit IDs: n/a
- Research date 2026-11-02; context: Android and iOS mobile. Spotify has no web-server transfer; the nearest thing is Settings > Apps and devices > 'Local audio files' toggle and manual copy instructions. Support text undated. Community search snippet gives an older label 'Show audio files from this device' (unconfirmed).
- Sources:
  - [Local files - Spotify Support](https://support.spotify.com/us/article/local-files/), published undated, accessed 2026-11-02, read
- Observed appearance: Route: profile picture -> 'Settings and privacy' -> 'Apps and devices' -> toggle 'Local audio files'; files then appear in Your Library under 'Local Files'. Instructions for adding files list platform-specific steps (iOS Files app 'On My iPhone' Spotify folder; Android via Google Drive or USB). No instructions page layout or notes styling is described.
- Observed behavior: Toggle on enables playback of local audio; permission may need to be granted in device settings; the page does not describe what turning off does or any server-style status.
- Evidence quality: documented
- Conflicts / confidence: High for the label and route; none for transfer-server behavior (R1-only). Treat the HTTP server as an R1 adaptation.
- User requirement overrides: none recorded
- R1 adaptation: Transfer page as a settings page: first row 'Wi-Fi Transfer' with green switch (starting the server pauses playback via a bottom sheet notice, not popup: 'Playback paused while transferring', Resume on exit). Below, a #282828 info card, radius 12, 16 px padding: label font_ui_16 #b3b3b3 'Open on your computer', address in font_ui_20 #fff ('http://<name>.local' and IP, two lines, long-press copies not required), then helper text. If Wi-Fi is off: replace the card with 'Turn on Wi-Fi first' text and an adjacent 'Wi-Fi settings' row (chevron) that opens the Wi-Fi page. Leaving the page triggers the shared leave-confirm dialog. Because Spotify has no equivalent, document in Decisions that this is R1-only.
- Acceptance criteria:
  - Transfer page uses settings rows and info card styles; toggling on shows the address and notes within one screen at 480x800.
  - With no Wi-Fi the page shows the settings-style 'Turn on Wi-Fi first' state and the Wi-Fi row works.
  - Server is stopped on any exit path (confirm, system back, power button) and playback state is preserved and resumable.
  - Playback mini-player is visible on the page whenever audio was playing before entry.

### R1-only receivers and detail pages (AirPlay receiver, Bluetooth receiver, AirPods detail and Press and hold) - closest Spotify pattern 'Listening on / Now playing' and settings sub-pages

- Audit IDs: n/a
- Research date 2026-11-02; context: No Spotify counterpart. Closest evidence is Connect picker flow and Now Playing; no dated screenshots read for Now Playing 'Listening on' state. Lossless/Connect article names 'Change quality settings'.
- Sources:
  - [Spotify Connect - Spotify Support](https://support.spotify.com/us/article/spotify-connect/), published undated, accessed 2026-11-02, read
  - [Lossless Audio Quality - Spotify Support](https://support.spotify.com/us/article/lossless-audio-quality/), published undated, accessed 2026-11-02, read
- Observed appearance: No source documents a receiver screen or battery ring UI. Spotify player pages use large centered artwork, title/artist below, and a bottom control row; Connect shows a remote-style view when playing on another device (third-party guide). AirPods/receiver pages are therefore R1 adaptations.
- Observed behavior: Not applicable to Spotify. For the R1: receivers show state only (waiting, connecting, playing, error), and codec choices are one tap away; battery rings and ANC pills are device-specific.
- Evidence quality: inferred
- Conflicts / confidence: Low. These pages have no Spotify analogue; fidelity means consistent shell visuals, not copying a Spotify screen. Confidence high that settings-row and now-playing language is the correct fallback.
- User requirement overrides: none recorded
- R1 adaptation: Receiver pages (AirPlay, Bluetooth): header with back and title; then a now-playing-style block: 160x160 artwork or glyph tile (#282828, radius 8), title font_ui_24 (1 line, ellipsis), artist font_ui_18 #b3b3b3, source device line font_ui_16 ('From <iPhone>'); green status dot + text for state (green for playing, #b3b3b3 for waiting, #ff5c5c for error). Codec/rate chip: pill 32 px high #282828 font_ui_16; tapping opens the shared bottom sheet. No progress bar unless the stream supplies it. AirPods page: title = device name, three battery rings 72 px with % text (font_ui_20) on #121212, noise mode as a 3-segment control 48 px high (selected #1ed760 bg/#000 text, others #282828/#fff), 'Press and hold' as a standard settings row with chevron; hold page uses three switch rows plus helper font_ui_16 #b3b3b3. Hardware-only caveat: AirPods battery and noise-control behavior unverified on R1.
- Acceptance criteria:
  - Receiver and AirPods pages use only shared tokens, fonts font_ui_16/18/20/24 and the settings-row/segmented/sheet components, with no legacy topbars.
  - Leave-confirm appears only while a receiver session is active and uses the shared dialog.
  - Each status (no source, connecting, waiting, playing, error) renders with distinct text and no overlap at 480x800.
  - AirPods battery states (single Max, L/R/case, reading battery) are visible without clipping; segmented control hit areas >=44 px.

Failed attempts:

- spotify.design returned a 301 redirect to open.spotify.com; no Spotify Design / Encore article on bottom sheets, dialogs or snackbars could be read.
- community.spotify.com threads fetched through WebFetch (Select a Connect device on Android; New Jam screen hides wifi speakers; Put the old Connect to device layout back; Show only local devices does not work; 'Added to' message blocking Done) returned only titles, so they are marked actually_read=false and unusable for layout evidence.
- YouTube pages (watch?v=3oKjvjtVGas, Clc1K3NZcqw) returned only title/navigation; no channel, date, description or transcript, so no dated visual evidence of the Connect picker was obtained.
- crutchfield.com/learn/spotify-connect.html returned HTTP 403.
- support.spotify.com 'Spotify is offline' (us and au) and 'Spotify and Bluetooth' (us and au) returned HTTP 429; the Bluetooth article was read once via /us/article/bluetooth/ but offline-state wording was not obtained.
- No dated Android/iOS screenshots of the Spotify Connect picker sheet, Settings rows, snackbars or confirm dialogs were obtained, so appearance claims for them (green active device, 'Listening on', row heights, switch style) are unverified and were recorded as R1 adaptation rather than fidelity.
- No source confirms 'No devices found' or 'Looking for devices' strings in Spotify's picker; do not cite them as Spotify copy.
- Repo files (main_menu.c, settingsrow.c, ui_metrics.h) were not inspected by this research task, so concrete pixel values in r1_adaptation are proposals consistent with the stated design tokens and must be reconciled with ui_metrics.h before implementation.
- Presentation labels (legacy/mixed/unreachable) in the route inventory were taken from the brief and were not re-traced against source in this research pass.

## audiobooks

### Audiobooks entry on Home (category chip) and Home audiobook feed (covers AB-HOME-CHIP, AB-HOME-FEED)

- Audit IDs: AB-HOME-CHIP, AB-HOME-FEED
- Research date 2026-11-02; context: Spotify mobile (iOS + Android), Premium audiobook launch US Nov 8 2023, expanded to more markets through Nov 2025. Popular Science article dated Sep 12 2023 describes the original entry point. Current (2026) app version, experiments and tier variations are UNKNOWN: no dated screenshots of the 2026 Home screen were read. Audiobook recommendations are in the Home feed (Newsroom Nov 18 2025).
- Sources:
  - [How to listen to Spotify's huge library of audiobooks (Popular Science)](https://www.popsci.com/diy/spotify-audiobooks-tips/), published 2023-09-12, accessed 2026-11-02, read
  - [Spotify Newsroom: audiobooks in Premium, 5 more European countries](https://newsroom.spotify.com/2025-11-18/audiobooks-in-premium-sweden-denmark-finland-iceland-monaco/), published 2025-11-18, accessed 2026-11-02, read
  - [TechCrunch: Spotify brings 15 hours of monthly audiobook listening to US Premium (screens caption only; images not seen)](https://techcrunch.com/?p=2625792), published 2023-11-08, accessed 2026-11-02, read
  - [9to5Google: Spotify audiobooks US Premium](https://9to5google.com/2023/11/08/spotify-audiobooks-us-premium/), published 2023-11-08, accessed 2026-11-02, read
- Observed appearance: Documented only in text: on the Home tab an 'Audiobooks' link/pill sits beside Music and Podcasts & Shows at the top (Popsci, 2023); the Search tab also had an 'Audiobooks' thumbnail above the search box. Titles surface in the Home feed with an 'Included in Premium' label (9to5Google 2023). No source read describes shelf titles, card size, or a 'Continue listening' shelf with progress bars for audiobooks. Visual specifics (pill height, shelf layout, card aspect) are NOT verified from pixels.
- Observed behavior: Tapping the Audiobooks entry filters/navigates to audiobook content; recommendations also appear in the general Home feed and an 'Audiobooks hub' with editorial picks (Newsroom 2025). Automatic bookmarking saves the place so you can resume (Newsroom 2025). The sources do not state whether a chip choice persists across launches or whether the audiobook feed has a 'Continue listening' shelf; both are INFERRED from general Spotify Home behavior.
- Evidence quality: documented
- Conflicts / confidence: MEDIUM for 'audiobook entry exists at top of Home' (2023 text source, may have changed); LOW for any shelf names, card layout, 2-column grid, paging controls. Spotify Home does not use Previous/Next pagination (infinite scroll); that part of the R1 feed is a deliberate offline adaptation. Conflict: Newsroom 2025 says 'Home feed' + 'hub' while Popsci 2023 says top link; both can be true. Search-tab thumbnail may be retired.
- User requirement overrides: none recorded
- R1 adaptation: Keep the existing Audiobooks chip (pill height 44-48 px, font_ui_16, active = #1ed760 fill with black text, inactive #282828 with white text, 8 px gap, horizontally scrollable if it overflows 480 px). Feed shelves: header font_ui_20 bold, cards 2x per row at ~212 px wide (220 cover per UI_COLLECTION_COVER is too wide for a 2-column 480 px grid with 16 px gutters: use (480-16*2-12)/2 = 218 px cover square), title font_ui_16 white 1 line, author font_ui_16 #b3b3b3 1 line. Replace Previous/Page x of y/Next with an 'Show all' / 'See more' row of >=48 px height only when the list exceeds 12; keep paging because the bounded-row rule forbids unbounded widgets, but render the pager as a single #282828 pill row styled as a Spotify 'Show more' button. Persist the chip choice (already done) and restore scroll offset on return from Now Playing.
- Acceptance criteria:
  - With the Audiobooks chip active the header chip row stays pinned, active chip is #1ed760 background with #121212 text and has a hit area of at least 44x44 px.
  - Chip choice survives restart and Home to Now Playing to Home round trips without resetting scroll offset more than once.
  - No more than 6 + 6 + 12 book cards exist as widgets at any time (bounded), with cover loading asynchronous.
  - The pager buttons and 'Page x of y' label do not overlap the mini-player (mini-player height reserved) at 480x800 and are >=44 px high.

### Continue listening / resume progress display and Resume vs Start action (covers AB-HOME-FEED shelf, AB-HOME-PLAY, AB-HOME-DETAILS button)

- Audit IDs: AB-HOME-FEED, AB-HOME-PLAY, AB-HOME-DETAILS
- Research date 2026-11-02; context: Spotify mobile, 2023-2026. Official sources confirm progress is saved and playback resumes; no official source read documents a 'Continue listening' shelf, time-left text or a distinct 'Resume' label. Community reports (Android/iOS, undated or 2023-2025) describe finished chapters with green check marks and recurring resume-position bugs.
- Sources:
  - [Audiobooks in Premium plans (Spotify Support)](https://support.spotify.com/us/article/audiobooks-premium-plans/), published undated, accessed 2026-11-02, read
  - [Spotify Newsroom 2025-11-18](https://newsroom.spotify.com/2025-11-18/audiobooks-in-premium-sweden-denmark-finland-iceland-monaco/), published 2025-11-18, accessed 2026-11-02, read
  - [Spotify Newsroom: Audiobook Recaps beta (JUMP BACK IN)](https://newsroom.spotify.com/2025-11-13/audiobook-recaps-beta-test/), published 2025-11-13, accessed 2026-11-02, read
  - [9to5Mac: Spotify tests AI audiobook Recaps (iOS)](https://9to5mac.com/2025/11/13/spotify-tests-ai-powered-audiobook-recaps-on-ios-expands-playlist-shuffle-controls/), published 2025-11-13, accessed 2026-11-02, read
  - [Spotify Community: Resume Audiobooks (Android) - search snippet only](https://community.spotify.com/t5/Android/Resume-Audiobooks/m-p/4442749/highlight/true), published undated, accessed 2026-11-02, snippet
  - [Idea: Allow users to mark last played chapter - login redirect, snippet only](https://community.spotify.com/t5/Live-Ideas/Allow-Users-to-Mark-the-Last-Played-Chapter-or-Title-in/idi-p/7043160), published undated, accessed 2026-11-02, snippet
- Observed appearance: Documented: on the iOS audiobook page a 'Recap' button appears 'at the top of an audiobook's page' directly above the Chapters list (Newsroom + 9to5Mac, Nov 13 2025, iOS beta, English titles, after 15-20 minutes listened). Chapters list implies per-chapter state; community reports say fully played chapters show a green check mark. No source gives progress-bar thickness, time-left copy ('X h Y min left') or a green Resume pill. NOT verified visually.
- Observed behavior: Progress is saved automatically and the audiobook resumes where left off (Support + Newsroom). Playback speed does not change allowance accounting. Spotify has no documented user-facing 'mark as finished' control; completion appears automatic (community report, LOW confidence); a Community idea asks for manual 'mark last played' which suggests it does not exist. Tapping a finished audiobook behavior is undocumented.
- Evidence quality: documented
- Conflicts / confidence: MEDIUM that Spotify resumes automatically; LOW for any shelf/progress-bar visuals; the R1 AB-HOME-DETAILS 'Resume/Start listening' single green button is an adaptation consistent with Spotify's green primary Play on a collection page but the label text is INFERRED. The requirement text mentioning 'Mark as finished' is NOT corroborated for Spotify mobile; do not claim parity. Community reports of resume bugs support keeping an explicit saved file + second.
- User requirement overrides: none recorded
- R1 adaptation: Continue listening shelf cards: cover 218 px square, 4 px progress track (#4d4d4d track, #1ed760 fill, 218 px wide, 8 px below cover), below it font_ui_16 #b3b3b3 'Xh Ym left' (or 'Not started'); green 56x56 circular Play button (UI_COLLECTION_PLAY is 64; keep 56 on cards, 64 only on the sheet) overlaid bottom-right of cover with 8 px inset. Details sheet button: full-width 56 px pill, #1ed760, font_ui_20 bold black text 'Resume' when position>0 else 'Play'; add a secondary text row 'Mark as finished' only if product keeps that feature, styled as a #282828 sheet row 56 px high (label as R1-only). Finished books show a green check glyph left of the title instead of a progress bar.
- Acceptance criteria:
  - A book with a saved position shows a progress bar whose fill width equals position/duration within 1 px and a 'left' time that matches the book index.
  - Tapping the card Play or the sheet button resumes at the exact saved file and second (+/- rewind-after-pause setting) and does not expand Now Playing.
  - A finished book shows a check glyph and 'Finished' text and no progress bar; replaying it starts at 0.
  - Resume position persists across restart for at least three different books.

### Audiobook details: book page / bottom sheet with cover, author, description, chapters, primary Play (covers AB-HOME-DETAILS, AB-SUMMARY, the 'Summary' item of AB-NP-MENU)

- Audit IDs: AB-HOME-DETAILS, AB-SUMMARY, AB-NP-MENU
- Research date 2026-11-02; context: Spotify mobile iOS/Android. Book page elements per Popsci (Sep 12 2023): price/running time/star rating/narrator, sample clip, plus-to-library, Get/Buy (store flows not applicable), download arrow, pin via long-press. iOS audiobook page has Recap button above the Chapters list (Nov 2025). 'Extras' section for Follow-Along visuals (Nov 2024). Author pages are pilot (Nov 2024). Platform parity Android vs iOS for Recap: iOS only.
- Sources:
  - [Popular Science: Spotify audiobooks tips](https://www.popsci.com/diy/spotify-audiobooks-tips/), published 2023-09-12, accessed 2026-11-02, read
  - [9to5Mac: Recap button above Chapters list (iOS)](https://9to5mac.com/2025/11/13/spotify-tests-ai-powered-audiobook-recaps-on-ios-expands-playlist-shuffle-controls/), published 2025-11-13, accessed 2026-11-02, read
  - [Engadget: Spotify drops new audiobook tools](https://www.engadget.com/entertainment/streaming/spotify-drops-a-bunch-of-new-tools-for-audiobooks-including-a-sleep-timer-173327861.html), published 2024-11-21, accessed 2026-11-02, read
  - [Thurrott: Spotify adds sleep timer, playlists and immersive features for audiobooks](https://www.thurrott.com/?p=313485), published 2024-11-21, accessed 2026-11-02, read
  - [AlternativeTo: Page Match + book purchases](https://www.alternativeto.net/news/2026/2/spotify-adds-in-app-real-book-purchases-and-a-new-tool-to-sync-progress-with-audiobooks/), published 2026-02-06, accessed 2026-11-02, read
- Observed appearance: Documented in text only: cover, title, narrator/author, running time, rating, download arrow, chapter list, optional Recap button above chapters, 'extras' section. Exact layout (sticky header, scrolled title fade, gradient from cover color, description 'See more' expansion) is NOT verified; the Spotify description 'See more' behavior is INFERRED from other Spotify pages. Page Match (Feb 2026) adds a button on the book page for iOS/Android.
- Observed behavior: Book page is a full page, not a sheet, in Spotify; it shares structure with album/podcast pages (cover, title, metadata, action row, then list). Long-press on a library item gives Pin audiobook. Chapters are tapped to play; entire audiobooks cannot be queued, individual chapters can via 'Add to queue' (Popsci 2023).
- Evidence quality: documented
- Conflicts / confidence: MEDIUM for element list, LOW for layout. The R1 'one shared bottom sheet with excerpt' deviates from Spotify, which uses a full collection page for audiobooks; AGENTS.md says albums/playlists share the collection page, so the audiobook detail should likely reuse the shared collection page (collectionview) rather than a custom sheet, with the sheet kept only as the long-press/overflow quick menu. This is a recommendation, not a documented Spotify fact.
- User requirement overrides: none recorded
- R1 adaptation: Route a book tap to the shared collection page: header cover 220 px (UI_COLLECTION_COVER) centered, title font_ui_28 bold white (2 lines max), author font_ui_16 #b3b3b3, a metadata line 'Audiobook - Xh Ym' font_ui_16 #b3b3b3, action row 68 px (UI_COLLECTION_ACTION_HEIGHT) with download-less R1 icons (shuffle hidden for books), and a 64 px green Play/Resume circle (UI_COLLECTION_PLAY) on the right. Description block: 'About' font_ui_20, summary max 4 lines font_ui_16 #b3b3b3 with 'See more' link that expands in place (no separate Summary screen). Chapters list below as 76 px rows (UI_COLLECTION_TRACK_HEIGHT) with duration right-aligned and green title for the current chapter. The long-press/'...' sheet stays as the quick-action sheet (#202124, 16 px top radius, drag handle, rows >=56 px).
- Acceptance criteria:
  - Tapping a book in any surface opens the same collection-style page; there is no separate full-screen 'Summary' page with legacy topbar.
  - Summary text expands in place via 'See more' and collapses via 'See less' without scrolling jumps.
  - Chapters rows are 76 px with at least 44 px hit area and the playing chapter title uses #1ed760.
  - Back from the book page returns to the exact Home feed state (chip, shelf, scroll offset).

### Audiobooks Now Playing mode: controls, 15 s skips, transport (covers AB-NP-BOOK, AB-NP-SKIP, AB-LEG-CONTROLS)

- Audit IDs: AB-NP-BOOK, AB-NP-SKIP, AB-LEG-CONTROLS
- Research date 2026-11-02; context: Spotify mobile Now Playing for audiobooks. Official Support article 'Unlock Audiobooks' (page undated; text read via us and ao-en regional variants): Now Playing has playback speed control '1x', Skip forward/back 15 seconds, Sleep timer icon, download icon (whole book only) and three-dot menu (horizontal on iOS, vertical on Android) with Rate (after at least one chapter) and Share. Popsci Sep 12 2023: skip buttons jump between chapters in the queue context, 15 s skips, speed 0.5x to 3.5x.
- Sources:
  - [Unlock Audiobooks (Spotify Support, US)](https://support.spotify.com/us/article/audiobooks-unlock/), published undated, accessed 2026-11-02, read
  - [Unlock Audiobooks (Spotify Support, regional variant)](https://support.spotify.com/ao-en/article/audiobooks-unlock/), published undated, accessed 2026-11-02, read
  - [Popular Science: Spotify audiobooks tips](https://www.popsci.com/diy/spotify-audiobooks-tips/), published 2023-09-12, accessed 2026-11-02, read
- Observed appearance: Documented controls only (no pixel evidence): speed pill labeled '1x', circular back-15 and forward-15 glyphs flanking play/pause, sleep timer icon, download icon, three-dot overflow. Android uses vertical dots, iOS horizontal. Exact position of these controls relative to the scrubber is NOT verified. Popsci says 'skip buttons jump between chapters' which conflicts with Support's 15 s skip; likely previous/next remain chapter controls while the 15 s buttons are separate.
- Observed behavior: 15 s back/forward are fixed, not user-configurable (no setting found in any source read). Chapter previous/next likely also present (Popsci). The mobile 3-dot menu holds Rate/Share, not chapters or bookmarks. Spotify has NO bookmark control in Now Playing (a 2019 ebookfriendly article listed lack of bookmarks; Community 'Bookmark Position' idea exists, snippet only). Rating menu is gated on one chapter listened.
- Evidence quality: documented
- Conflicts / confidence: HIGH for 15 s skips, speed control, sleep timer icon, three-dot menu (current Support text). MEDIUM for chapter-skip semantics (Popsci 2023 vs Support). R1 deliberately differs: configurable 10/30/60 s skips; the R1 requirement allows R1-only settings but the default should match Spotify's 15 s and 15 should be an available option, which today it is not (10/30/60) per the route inventory; a product decision is needed. Side hardware keys: no Spotify equivalent.
- User requirement overrides: none recorded
- R1 adaptation: Book-mode transport row on the 480x800 Now Playing: centered play/pause 72 px green-on-white disc (match music layout), flanked by 'back N' and 'forward N' 56 px glyph buttons with the number rendered inside the arrow glyph in font_ui_16, and outer buttons: left = speed pill '1.0x' (min 56x44, font_ui_16 bold, #282828 pill), right = sleep timer moon glyph 44x44. Replace the vertical-ellipsis popover with the shared bottom sheet (#202124) containing: Chapters, Add bookmark, Bookmarks, About this book, Sleep timer, each row 56 px with 24 px leading glyph and font_ui_18 label. Make the skip step options 15/30/60 (default 15) in R1 settings while keeping 10 as legacy-stored value mapped to 15 if user hasn't customized; document as R1-only.
- Acceptance criteria:
  - Book mode shows exactly one transport row with back-N, play/pause, forward-N, plus speed pill and sleep glyph, with all hit areas >=44x44 px and no overlap at 480x800.
  - Pressing back-N/forward-N seeks by the configured seconds, clamps at 0 and at duration, and the label inside the glyph reflects the setting.
  - Hardware side keys perform the same skip as the on-screen buttons and keep working while the screen is off/locked per existing behavior.
  - The overflow opens the shared bottom sheet, not an anchored legacy popover.

### Playback speed selection for audiobooks (covers AB-NP-SPEED)

- Audit IDs: AB-NP-SPEED
- Research date 2026-11-02; context: Spotify mobile (iOS/Android). Support: tap the '1x' speed control in Now Playing. Popsci (Sep 12 2023): speed range 0.5x to 3.5x. Podcast speed picker historically a sheet with steps; the audiobook sheet's exact list of steps and layout is NOT verified in any source read. Speed does not affect allowance deduction (Support).
- Sources:
  - [Unlock Audiobooks (Spotify Support, US)](https://support.spotify.com/us/article/audiobooks-unlock/), published undated, accessed 2026-11-02, read
  - [Popular Science: Spotify audiobooks tips](https://www.popsci.com/diy/spotify-audiobooks-tips/), published 2023-09-12, accessed 2026-11-02, read
  - [Audiobooks in Premium plans (Spotify Support)](https://support.spotify.com/us/article/audiobooks-premium-plans/), published undated, accessed 2026-11-02, read
- Observed appearance: Pill/text button '1x' in the Now Playing controls row. Selection UI (bottom sheet with vertical list vs slider) is UNVERIFIED; do not claim popover vs sheet fidelity beyond 'bottom sheet is Spotify's general selection pattern' (INFERRED).
- Observed behavior: Tap the speed control; choose a speed; applies immediately and persists (persistence INFERRED). Range 0.5x-3.5x per Popsci 2023. R1 popover lists 0.5-2.0x in 7 steps, narrower than Spotify's documented range.
- Evidence quality: documented
- Conflicts / confidence: HIGH that a speed control exists, MEDIUM for the 0.5x-3.5x range (single 2023 source), LOW for step granularity and the picker layout. R1 maximum is hardware-dependent (decoder/time-stretch CPU on the R1 is UNMEASURED); do not extend beyond 2.0x without measurement.
- User requirement overrides: none recorded
- R1 adaptation: Use the shared bottom sheet: title 'Playback speed' font_ui_20, then a vertical list of 7 rows (0.5x,0.75x,1.0x,1.25x,1.5x,1.75x,2.0x), each 56 px high, font_ui_18 label left, green 24 px check on the active row (label also green), tap applies immediately and dismisses after 120 ms, sheet height = 56*7 + 64 header + 16 handle = ~472 px (cap at 60% of 800 = 480). Speed pill in the transport row shows 'N.NNx' trimmed ('1x', '1.25x') in font_ui_16 bold. Persist per book if currently per book; otherwise global, and document it.
- Acceptance criteria:
  - Tapping the speed pill opens the shared bottom sheet (not a popover) with the current rate checked.
  - Selecting a rate changes decoder speed within 500 ms without audible glitch or seeking, and the pill label updates.
  - Rate persists across track change within the book and across restart as documented.
  - Rows are >=48 px high and fit above the mini-player/footer without scrolling at 480x800.

### Sleep timer for audiobooks, incl. 'end of chapter' and the quick-panel/option toggles (covers AB-QP sleep button, sleep rows of AB-LEG-OPTIONS)

- Audit IDs: AB-QP, AB-LEG-OPTIONS
- Research date 2026-11-02; context: Spotify mobile. Sleep timer added to audiobooks Nov 21 2024 (Engadget/Thurrott), Premium, mobile app only per AlternativeTo. Support: tap sleep timer icon in Now Playing. For music/podcasts: three-dot menu then 'Sleep Timer', 'Stop Audio In' screen with 5, 10, 15, 30, 45 minutes and 1 hour plus 'End of Track' (How-To Geek, Sep 28 2021 iOS+Android; Thurrott Dec 2 2019 iOS). 'End of chapter' for audiobooks is only in community reports (iOS users say it stopped working); no official page confirms it.
- Sources:
  - [Engadget 2024-11-21](https://www.engadget.com/entertainment/streaming/spotify-drops-a-bunch-of-new-tools-for-audiobooks-including-a-sleep-timer-173327861.html), published 2024-11-21, accessed 2026-11-02, read
  - [Thurrott 2024-11-21](https://www.thurrott.com/?p=313485), published 2024-11-21, accessed 2026-11-02, read
  - [How-To Geek: sleep timer in Spotify](https://www.howtogeek.com/749269/how-to-set-up-a-sleep-timer-in-spotify/), published 2021-09-28, accessed 2026-11-02, read
  - [Thurrott: Spotify sleep timer on iOS (music)](https://www.thurrott.com/?p=223206), published 2019-12-02, accessed 2026-11-02, read
  - [Unlock Audiobooks (Spotify Support, US)](https://support.spotify.com/us/article/audiobooks-unlock/), published undated, accessed 2026-11-02, read
  - [Community: Audiobook timer not working (iOS) - JS page, title only](https://community.spotify.com/t5/iOS-iPhone-iPad/Audiobook-timer-not-working/m-p/6827790/highlight/true), published undated, accessed 2026-11-02, snippet
  - [FreeYourMusic: how to set Spotify sleep timer (updated Sep 2026 per search snippet) - fetch timed out](https://freeyourmusic.com/en/blog/how-to-set-spotify-sleep-timer), published 2026-09, accessed 2026-11-02, snippet
- Observed appearance: Moon/sleep timer icon in the audiobook Now Playing controls (Support). The picker is a sheet titled 'Stop Audio In' (How-To Geek 2021, music) listing 5/10/15/30/45 min and 1 hour, with 'End of Track' as a row; active timer offers 'Turn Off Timer'. For audiobook the end row label ('End of chapter') is community-reported only. Sheet styling NOT visually verified.
- Observed behavior: Choose duration or end-of-item; timer stops playback and shows countdown on the icon (countdown display INFERRED). The R1 'stop at end of chapter' switch and 'sleep timer' wheel durations are R1-only extensions in settings; Spotify sets the timer from Now Playing each time, not from a Settings page.
- Evidence quality: documented
- Conflicts / confidence: HIGH for the existence and entry point on audiobook Now Playing; MEDIUM for the preset list (2021 music source, may differ for audiobooks); LOW for 'End of chapter' (community only, reported flaky). Android vs iOS: menu path varies per community; Support says icon in Now Playing.
- User requirement overrides: none recorded
- R1 adaptation: Move the primary entry into the book Now Playing: moon glyph 44x44 opens the shared bottom sheet 'Sleep timer' with rows (56 px each): 5 min, 10 min, 15 min, 30 min, 45 min, 1 hour, 'End of chapter' (book mode) and, when active, 'Turn off timer' in #1ed760. Checkmark on active row. Active timer shows remaining minutes in font_ui_16 green under the moon glyph. Settings rows remain for default behaviors (R1-only): 'Stop at end of chapter' switch and 'Rewind after pause' pills (3/5/7/10 s) rendered as shared settings rows 64 px high with the Spotify green switch (52x32 track). Quick-panel button keeps tap=toggle and long-press=open the sleep sheet (not the legacy options page).
- Acceptance criteria:
  - Moon glyph in book Now Playing opens the shared bottom sheet with durations plus End of chapter and the active selection is checked.
  - A 5-minute timer pauses playback at 5:00 +/- 1 s and the timer indicator clears; 'End of chapter' pauses exactly at the chapter boundary and does not auto-advance.
  - Quick-panel long-press opens the same sheet, not the legacy Audiobooks options screen.
  - Timer state survives screen-off but is cancelled on manual stop; behavior documented in UI_SPEC.

### Audiobook chapters list and chapter navigation (covers AB-CHAPTERS)

- Audit IDs: AB-CHAPTERS
- Research date 2026-11-02; context: Spotify mobile. Chapters list is on the audiobook page (9to5Mac: Recap button 'directly above an audiobook's Chapters list', iOS, Nov 13 2025). Popsci (Sep 12 2023): chapters appear as separate queue items; skip buttons jump between chapters; 'Add to queue' via long-press on a chapter; whole book cannot be queued. Support: no chapter navigation control documented in the Now Playing view; chapters cannot be downloaded individually.
- Sources:
  - [9to5Mac: Recap above Chapters list](https://9to5mac.com/2025/11/13/spotify-tests-ai-powered-audiobook-recaps-on-ios-expands-playlist-shuffle-controls/), published 2025-11-13, accessed 2026-11-02, read
  - [Popular Science: Spotify audiobooks tips](https://www.popsci.com/diy/spotify-audiobooks-tips/), published 2023-09-12, accessed 2026-11-02, read
  - [Unlock Audiobooks (Spotify Support, US)](https://support.spotify.com/us/article/audiobooks-unlock/), published undated, accessed 2026-11-02, read
  - [Spotify SDK audiobook model docs (HTTP 410, snippet only)](https://www.mintlify.com/jonathan343/spotify-sdk/api/models/audiobook), published undated, accessed 2026-11-02, snippet
- Observed appearance: Chapters are a vertical list under the book header, each chapter a list row (title, duration), current chapter likely in green and finished chapters with a green check (community report). Row height and typography NOT verified.
- Observed behavior: Tap a chapter to start it; chapters are queue items so Next/Previous move between chapters; resume point is per chapter in the API. Spotify's Now Playing does not offer an in-player chapters sheet in any source read; chapters live on the book page. Bugs reported where chapters play out of order (community).
- Evidence quality: documented
- Conflicts / confidence: MEDIUM: list lives on the book page (iOS 2025). Whether Android Now Playing exposes a chapter list is UNVERIFIED. R1's in-player Chapters entry is an R1 convenience; present it as a sheet/page in Spotify language, not a legacy full-screen list.
- User requirement overrides: none recorded
- R1 adaptation: Primary: chapters rows live on the shared book page (see details record). Secondary R1 convenience: overflow sheet row 'Chapters' opens a tall bottom sheet (#202124, max height 85% of 800 = 680 px, drag handle) containing a windowed list (max 12 row widgets) of 64 px rows: left title font_ui_18 (green if current), right duration font_ui_16 #b3b3b3, in-file chapters show start time; a folder book's parts show 'Part N'. Tap seeks (or restarts the part), dismisses the sheet, and returns to Now Playing with no screen transition glitch. Back/swipe-down dismisses without seeking.
- Acceptance criteria:
  - The chapters surface uses the shared sheet or book page; no legacy full-screen list with legacy topbar is reachable.
  - The current chapter is auto-scrolled into view and drawn in #1ed760; the list never instantiates more than 12 row widgets.
  - Tapping a chapter seeks to its start (in-file) or loads the part (folder) within 1 s and Now Playing updates the chapter label.
  - Dismiss via drag, scrim tap and hardware back all return to Now Playing with playback unchanged.

### Bookmarks (add, list, delete) - R1-only, closest analogue is Spotify resume/auto-bookmark and shared sheets (covers AB-BOOKMARK-ADD, AB-BOOKMARKS-BOOK, AB-LEG-BOOKMARKS-INDEX)

- Audit IDs: AB-BOOKMARK-ADD, AB-BOOKMARKS-BOOK, AB-LEG-BOOKMARKS-INDEX
- Research date 2026-11-02; context: Spotify mobile has no user-facing bookmark list for audiobooks in any source read. Newsroom Nov 18 2025 mentions only 'automatic bookmarking' of the listening place. Community has long-standing requests for bookmark position (snippet only, unread); a 2019 article states Spotify lacked bookmarks. Spotify adds Page Match (Feb 2026) as place-sync, not bookmarks.
- Sources:
  - [Spotify Newsroom 2025-11-18 (automatic bookmarking)](https://newsroom.spotify.com/2025-11-18/audiobooks-in-premium-sweden-denmark-finland-iceland-monaco/), published 2025-11-18, accessed 2026-11-02, read
  - [Ebookfriendly: Eary iOS app (Spotify lacks bookmarks)](https://ebookfriendly.com/eary-ios-app-spotify-audiobooks/), published 2019-09-10, accessed 2026-11-02, read
  - [AlternativeTo: Page Match](https://www.alternativeto.net/news/2026/2/spotify-adds-in-app-real-book-purchases-and-a-new-tool-to-sync-progress-with-audiobooks/), published 2026-02-06, accessed 2026-11-02, read
  - [Community: Bookmark Position in an Audiobook or Podcast (Implemented Ideas) - snippet only](https://community.spotify.com:443/t5/Implemented-Ideas/All-Platforms-Bookmark-Position-in-an-Audiobook-or-Podcast/idc-p/1203821), published undated, accessed 2026-11-02, snippet
- Observed appearance: none (R1-only). Analogue: Spotify list rows in a sheet (cover-less row, title font ~16, secondary line #b3b3b3, overflow '...' per row) and snackbar confirmation at bottom ('Added to ...', swipeable; community notes it can cover the Done button).
- Observed behavior: No Spotify equivalent; closest are automatic resume position and queue. Toast/snackbar: Spotify shows a bottom pop-up bar on like/add (Pratt critique Feb 1 2023: feedback 'clear and immediate'; community: toast covers controls for ~5 s). Long-press row menus in Spotify use bottom sheets, not anchored popovers.
- Evidence quality: inferred
- Conflicts / confidence: HIGH confidence that Spotify has no bookmark feature on audiobooks (negative claim, based on absence in current Support text and a Nov 2025 newsroom mention only of automatic bookmarking, but MEDIUM overall because the Community idea status is unread). Keeping bookmarks is allowed as an R1-only feature per AGENTS.md; it must use the shared sheet language.
- User requirement overrides: none recorded
- R1 adaptation: 'Add bookmark' becomes a sheet row that saves instantly and shows a bottom snackbar (#282828? use #333333 on #121212 bg, 56 px high, 16 px side margin, 8 px radius, font_ui_16 white 'Bookmark saved', 3 s, positioned 72 px above the mini-player/footer so it never covers the transport row). 'Bookmarks' opens the tall bottom sheet list (same component as Chapters): rows 64 px: font_ui_18 chapter label, font_ui_16 #b3b3b3 'H:MM:SS', trailing 44x44 '...' button opening 'Delete bookmark' in a small sheet (replace long-press-only popover; keep long-press as shortcut). Index of books with bookmark counts becomes a library-style list reachable from the book page, not a legacy section corner button. Full states: 'bookmarks full' toast 'Bookmark limit reached', empty state glyph + 'No bookmarks yet'.
- Acceptance criteria:
  - Adding a bookmark shows the snackbar within 150 ms, does not overlap the transport controls, and auto-hides in 3 s +/- 0.5 s.
  - Bookmark list and delete use the shared sheet and confirm via the sheet row (no legacy popover); deletion removes only the bookmark record, never audio files.
  - Tapping a bookmark seeks the current book or starts the other book at that position and returns to Now Playing.
  - Empty, full (limit) and write-failure states each display distinct copy.

### Library, filter and Search for audiobooks; legacy Library/Series/Authors/Continue/Finished lists (covers AB-LIB-SEARCH-GAP, SHELL-TOOLS-AB, AB-LEG-SECTION, AB-LEG-BOOKS, AB-LEG-NAMES)

- Audit IDs: AB-LIB-SEARCH-GAP, SHELL-TOOLS-AB, AB-LEG-SECTION, AB-LEG-BOOKS, AB-LEG-NAMES
- Research date 2026-11-02; context: Spotify mobile Your Library: filters since the Apr 29 2021 redesign (albums/artists/playlists/podcasts, Downloaded; sort by recents/alphabetical/creator; grid view; pin up to 4) - audiobooks NOT mentioned in 2021 source. Popsci Sep 12 2023: Your Library > Audiobooks, sort by date added/publication date/author/alphabetical, pin audiobook via long-press. Smart Filters (RouteNote Sep 10 2025, Spotify newsroom Sep 5 2025): filter button top-left, applies to music/playlists/audiobooks/podcasts, Premium, mobile+tablet, listed markets. Search tab had an Audiobooks thumbnail (2023). Unlocked/saved audiobooks appear under Audiobooks in Your Library (Support search snippet).
- Sources:
  - [Spotify Newsroom: new Your Library (Apr 29 2021)](https://newsroom.spotify.com/2021-04-29/listeners-can-explore-their-spotify-collections-faster-and-easier-with-a-new-your-library-2/), published 2021-04-29, accessed 2026-11-02, read
  - [Popular Science: Spotify audiobooks tips](https://www.popsci.com/diy/spotify-audiobooks-tips/), published 2023-09-12, accessed 2026-11-02, read
  - [RouteNote: Smart Filters](https://routenote.com/blog/spotify-rolls-out-smart-filters-to-let-you-sort-your-library-for-the-perfect-moment/), published 2025-09-10, accessed 2026-11-02, read
  - [Android Central: Library dynamic filters (content truncated, headline only)](https://www.androidcentral.com/spotify-your-library-ui-update-dynamic-filters), published 2025, accessed 2026-11-02, snippet
  - [Audiobooks (Spotify Support, US) - HTTP 429 on fetch, search snippet only](https://support.spotify.com/us/article/audiobooks/), published undated, accessed 2026-11-02, snippet
- Observed appearance: Library: horizontally scrollable filter chips at the top (2021 redesign; chips for content types), sort row with a 'Recents' style label and list/grid toggle, rows with 48-56 px covers (2021). Audiobook rows: cover, title, author, and likely progress info (UNVERIFIED). Smart Filters introduce a top-left filter button in 2025 (MEDIUM). No 2026 screenshot read.
- Observed behavior: Your Library > Audiobooks shows saved/unlocked books; sort options listed above; pin via long-press. Authors in Spotify exist as author pages (pilot, Nov 2024) reached from a book page; Series is not documented as a Library filter. 'Finished' is not a documented filter. Search returns audiobooks as a result type/chip (INFERRED from the Search thumbnail in 2023).
- Evidence quality: documented
- Conflicts / confidence: MEDIUM for Library > Audiobooks and sort options (Popsci 2023); LOW for 2025-2026 chip/filter UI (Smart Filters changed it); Spotify no longer documents a top 'Audiobooks' home chip beyond 2023 text. R1 Authors/Series/Finished/Continue tiles have no Spotify analogue; must be re-expressed as Library chips or sort/filter rows in the Spotify shell. Today (route inventory) the Spotify Library and Search tabs cannot reach audiobooks: this is a gap against Spotify (documented) and AGENTS.md requirement that retained features use the Spotify shell.
- User requirement overrides: none recorded
- R1 adaptation: Your Library tab: add an 'Audiobooks' chip (44 px high pill, font_ui_16) next to existing chips; selecting it shows the book list as 72 px rows (cover 56 px, title font_ui_18, 'Author - Xh Ym left' font_ui_16 #b3b3b3, trailing 44x44 '...'), windowed to 12 row widgets, with a sort row 'Recents' + sheet picker (Recently played, Title, Author, Date added, Finished) and sub-chips 'In progress \| Not started \| Finished' replacing the Continue/Finished tiles. Authors and Series become group headers or a second-level chip 'Authors'/'Series' opening a collection-style list (name + count, 64 px rows, 48 px circular initial avatar). Search tab: add 'Audiobooks' result section/chip that queries title/author index. Remove the tools-grid 'Audiobooks' tile route to the legacy section, or redirect it into the Library tab with the chip preselected. Guard against empty/no-index state with the shared empty panel.
- Acceptance criteria:
  - From Your Library the Audiobooks chip lists all indexed books, sorted per the chosen sort, with at most 12 row widgets live.
  - Searching a title or author in Search returns audiobook results that open the book page and play.
  - The vertical-ellipsis tools grid no longer reaches a legacy Audiobooks section screen; every audiobook screen uses #121212 background and the shared header/back.
  - Authors/Series/Finished/Continue are reachable and filter correctly (counts match the index) via Spotify-style chips or rows.

### Empty state and 'Scan books' import (local audiobooks), closest analogue Spotify Local files (covers AB-HOME-EMPTY, AB-HOME-SCAN, AB-LEG-SCAN, AB-LEG-SCAN-CONFIRM)

- Audit IDs: AB-HOME-EMPTY, AB-HOME-SCAN, AB-LEG-SCAN, AB-LEG-SCAN-CONFIRM
- Research date 2026-11-02; context: Spotify mobile 'Local audio files' (Support, undated): Profile picture > Settings and privacy > Apps and devices > switch on 'Local audio files'; content then appears in Your Library as 'Local Files'; user adds files by copying them to the app folder (iOS Files app / Android USB). Support does not describe an empty state or progress UI. Spotify's own empty-state copy was not found despite a search: UNVERIFIED. Spotify scanning is not a user-visible progress page.
- Sources:
  - [Local files (Spotify Support, US)](https://support.spotify.com/us/article/local-files/), published undated, accessed 2026-11-02, read
  - [Listen offline (Spotify Support) - HTTP 429, snippet only](https://support.spotify.com/us/article/listen-offline/), published undated, accessed 2026-11-02, snippet
  - [Community: Spotify Your Library Empty - snippet only](https://community.spotify.com/t5/Your-Library/Spotify-Your-Library-Empty/td-p/5201625), published undated, accessed 2026-11-02, snippet
- Observed appearance: No Spotify empty-state visual was found. General Spotify pattern (INFERRED, not sourced): centered headline (bold, ~24 px), one-line secondary text in #b3b3b3, and a white/green pill CTA. The R1 copy 'Your next chapter starts here' with a book glyph is in keeping with that pattern but is R1 copy, not Spotify copy.
- Observed behavior: Spotify local files: toggle in settings, content appears in library; no scan button or progress. R1 background scan is an R1-only operation. Spotify confirmation for destructive/settings flows uses a modal dialog (centered card, title, body, two buttons) or sheet; I did not obtain a dated Spotify source for the dialog styling.
- Evidence quality: inferred
- Conflicts / confidence: MEDIUM for the Local files settings path; LOW for empty-state visuals and dialog styling; confidence in the R1 adaptation depends on internal design tokens. Scan confirm dialog is arguably unnecessary because scanning is non-destructive and offline; Spotify-like behavior is to start immediately and show an inline progress state.
- User requirement overrides: none recorded
- R1 adaptation: Empty panel centered in the content area above the mini-player: 96 px #282828 circle with 48 px book glyph, headline font_ui_24 bold white, hint font_ui_16 #b3b3b3 (max 3 lines, 380 px wide), CTA 'Scan books' = 200x56 px green #1ed760 pill, font_ui_18 bold black. 'Looking for your books...' state replaces the CTA with an indeterminate 4 px green progress line and live 'N books found' text (font_ui_16). Remove the separate full-screen scan page and red Cancel: scanning happens inline in the empty panel/feed header with a 'Cancel' text button (44 px) and ends with a snackbar 'N books added'. Drop the confirm dialog for Scan; if kept for the full rescan, use the shared sheet with title font_ui_20, body font_ui_16 #b3b3b3, primary green 'Scan' + secondary 'Cancel' text buttons, each 56 px.
- Acceptance criteria:
  - With no card/index/scan running the Audiobooks chip shows the empty panel with Scan books >=44 px high and no legacy topbar anywhere.
  - Starting a scan keeps the UI responsive (touch latency unchanged) and shows live count; Cancel stops the scan without deleting any audio file.
  - On completion the feed rebuilds in place (no screen switch) and a snackbar reports the count; a failed scan shows a distinct error panel with Retry.
  - No full-screen scan page with red Cancel/OK is reachable from the Spotify shell.

### Audiobook options and settings rows (controls row, Total/Chapter duration pills, rewind-after-pause, scan row) - R1-only, analogue Spotify Settings list rows (covers AB-LEG-OPTIONS, AB-LEG-CONTROLS settings parts)

- Audit IDs: AB-LEG-OPTIONS, AB-LEG-CONTROLS
- Research date 2026-11-02; context: Spotify mobile Settings and privacy: grouped list with section headers, rows with title + secondary text, toggles and chevrons; reached from profile picture (Support: profile picture > Settings and privacy). Spotify has no audiobook-specific settings page in any source read (speed and sleep timer are in Now Playing). Spotify's settings visual details were NOT fetched from a dated mobile source.
- Sources:
  - [Audiobooks in Premium plans (profile picture > Settings and privacy path)](https://support.spotify.com/us/article/audiobooks-premium-plans/), published undated, accessed 2026-11-02, read
  - [Local files (Settings and privacy > Apps and devices > Local audio files)](https://support.spotify.com/us/article/local-files/), published undated, accessed 2026-11-02, read
  - [Pratt IxD: Design Critique: Spotify (iPhone App)](https://ixd.prattsi.org/2023/02/design-critique-spotify-iphone-app/), published 2023-02-01, accessed 2026-11-02, read
- Observed appearance: Settings in Spotify are a black list screen with grey section headers, rows with white title, grey secondary description, switch at right (green when on) and chevron for sub-pages; bottom navigation stays visible 'throughout the entire application' with a filled green active icon (Pratt, Feb 2023, iPhone). Row heights/fonts NOT verified from pixels, the row pattern above is INFERRED from Spotify general knowledge and should be validated against a screenshot.
- Observed behavior: Switches apply immediately; pickers (e.g., streaming quality) use a radio-list sheet or sub-page with a check on the selected value. Audiobook options in R1 (skip sizes, duration display mode, rewind after pause) are R1-only and should adopt these patterns.
- Evidence quality: inferred
- Conflicts / confidence: LOW for the visual specifics, MEDIUM for flow (switch, picker, sub-page). Spotify offers no audiobook settings page so this is a documented R1-only adaptation.
- User requirement overrides: none recorded
- R1 adaptation: Use the shared settings rows (gui/shell/settingsrow.c) in a Spotify-style page: header 'Audiobooks' font_ui_24 bold with back chevron 48x48, section headers font_ui_16 #b3b3b3, rows 64 px high with title font_ui_18 white, subtitle font_ui_16 #b3b3b3, Spotify switch 52x32 (green #1ed760 on, #535353 off); pill choice rows ('Show duration' Total/Chapter, rewind 3/5/7/10 s, skip back/forward) as segmented 40 px pills inside a 56 px row, or open the shared radio-list bottom sheet with check. Replace lvgl duration wheels for sleep timer with the sleep sheet. Reach this page from the book overflow sheet ('Playback settings') and from the Settings tree, never via a legacy section corner button. Page background #121212, scrollable, no legacy topbar/title.
- Acceptance criteria:
  - Every row is >=56 px with >=44 px hit target, uses the shared settings row component and shows the current value or switch state.
  - Changing a pill/switch applies immediately and persists across restart.
  - The Audiobooks options page is reachable only through the Spotify-style navigation and its back returns to the originating screen (book page/Now Playing/Settings).
  - No LVGL default-theme widgets (roller wheels, default buttons) are visible.

Failed attempts:

- support.spotify.com/us/article/audiobooks-in-the-spotify-app/ returned a 404 page (invented URL from memory; no content).
- support.spotify.com/uk/article/audiobooks/ fetched as a 404 'Page not found' page; the search snippet listed it but fetch returned only footer links.
- support.spotify.com/ie/article/audiobooks-unlock/ fetched as 404; the us and ao-en variants of the same article fetched successfully and were used instead.
- support.spotify.com/us/article/sleep-timer/ is a 404; the sleep timer is not documented by a dedicated Support article I could reach.
- support.spotify.com/us/article/audiobooks/ and /us/article/listen-offline/ returned HTTP 429; only search-result snippets were seen (marked actually_read=false).
- tomsguide.com audiobooks how-to returned headline only (content truncated); nothing recorded from it.
- androidauthority.com listen-audiobooks-spotify returned HTTP 403.
- Spotify Community threads (Audiobook timer not working, More options for Sleep Timer control, Audiobook mark last played chapter) returned titles only or a login redirect; the Mark-as-played / End-of-chapter claims are community reports from search snippets only and are unverified.
- freeyourmusic.com sleep timer article (snippet says updated Sep 2026) timed out on fetch.
- androidcentral.com Library dynamic filters article returned navigation and headline only.
- mintlify.com Spotify SDK audiobook model docs returned HTTP 410 Gone.
- No YouTube video pages with dates were found by search for the audiobook mobile UI; no dated video evidence is available.
- No spotify.design or Encore design-system source on audiobooks, bottom sheets, snackbars or dialogs surfaced in search; mobile dialog/sheet/snackbar styling rests on a Pratt IxD critique (Feb 2023) and community posts, which are weak.
- No dated mobile screenshots (Mobbin/Page Flows) of the audiobook Home shelf, book page, Now Playing, speed sheet or sleep timer sheet could be read; every appearance field is text-derived, not pixel-verified, and the empty-state copy for Spotify's library was not found.
- No Android vs iOS split could be confirmed for speed, sleep timer or chapter list beyond: three-dot icon orientation (Support) and Recap being iOS-only beta (Newsroom Nov 13 2025). Current (2026) app version and active experiments are unknown.

## extras

### Account entry and Settings-and-privacy root (replaces Tools ellipsis button, Tools grid and More grid)

- Audit IDs: XTRA-TOOLS-BTN, XTRA-TOOLS-GRID, XTRA-MORE-GRID
- Research date 2026-11-02; context: Spotify mobile, Android and iOS, Sept 2025 (SlashGear Android walkthrough dated 2025-09-14) and current support.spotify.com articles (undated, fetched 2026-11-02). Premium and Free share the path. Exact drawer contents (What's new, Recents/Listening history, Settings and privacy) vary by version and experiment; 2023-2024 community reports only, not official.
- Sources:
  - [How To Open And Adjust Your Spotify Settings On Android (SlashGear)](https://www.slashgear.com/1965140/how-to-open-spotify-settings-on-android), published 2025-09-14, accessed 2026-11-02, read
  - [Equalizer - Spotify Support](https://support.spotify.com/us/article/equalizer/), published undated, accessed 2026-11-02, read
  - [Local files - Spotify Support](https://support.spotify.com/us/article/local-files/), published undated, accessed 2026-11-02, read
  - [Privacy and social controls - Spotify Support (search-result summary only; fetch returned HTTP 429)](https://support.spotify.com/us/article/privacy-and-social-controls/), published undated, accessed 2026-11-02, snippet
  - [PhoneArena: Spotify Recents replaces Listening History (snippet only; fetch HTTP 403)](https://www.phonearena.com/news/spotifys-new-feature-recents_id165212), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Spotify has NO gear or vertical-ellipsis in the Home header. The entry is the circular profile avatar at the upper left of Home and Your Library; it opens a left-side drawer/panel. Community/press reports (not fully read) say the drawer lists What's new, Recents (formerly Listening history) and Settings and privacy as icon+label rows. Settings and privacy is a full-page vertical list on #121212 of grouped section rows with chevrons, in this order (SlashGear, read): Account, Content and display, Playback, Privacy and social, Notifications, Apps and devices, Data-saving and offline, Media quality, Advertisements, About and support. Sub-pages are plain lists of toggle rows (title + grey description + green switch), sliders and radio rows. Android and iOS differ only in minor glyphs (three-dot orientation).
- Observed behavior: Tap avatar -> drawer slides in from left over a dimmed Home; tap Settings and privacy -> pushes a settings page; back arrow top-left pops one level; each category pushes its own page. Sub-pages hold live toggles with immediate effect (no Save). Support pages consistently route everything (equalizer, local files, Connect local-device visibility) through Settings and privacy > category, so R1-only utilities have a natural home as categories rather than a separate tile launcher.
- Evidence quality: documented
- Conflicts / confidence: High confidence on path and category order (two sources: SlashGear 2025-09 and Spotify support steps). Medium confidence on drawer contents (secondary snippets, versions vary; PhoneArena not read). One search summary mentioned a top-right gear on some guides; that is older/low quality and contradicts Spotify support, so ignored. Visual pixel details (row height, fonts) are inferred, not measured.
- User requirement overrides: none recorded
- R1 adaptation: Replace the header vertical-ellipsis (tools_screen) and gear (settings_screen) with ONE 44x44 (draw 32x32) circular avatar/initial button at left of the Home/Search/Library header (x=16,y=12), matching Spotify. Tap opens a left drawer, 360 px wide (75% of 480), #202124-> use #121212 panel with 50% black scrim, slide 220 ms (UI_SHEET_TRANSITION_MS). Drawer rows 64 px high, icon 24 + label font_ui_20, secondary font_ui_16 #b3b3b3: 'Settings and privacy', 'Music tools' (R1-only group), 'Recents' only if backing data exists (otherwise omit, do not stub). 'Settings and privacy' pushes a full page: header 64 px with back chevron (44x44 hit) + font_ui_24 title; group rows 72 px, font_ui_18 title / font_ui_16 #b3b3b3 value or description, chevron 24 px, divider none (Spotify uses spacing). Section order adapted: Account (omit, local-only), Playback (EQ, crossfade, gapless, DAC), Library and files, Audiobooks and books, Apps and devices (Bluetooth, DAC, USB), Data-saving and storage, About and support. R1-only 'Extras' category lists Game Boy, Flappy Bird, Metronome, Files, Books as 72 px rows (icon tile 48 px #282828 radius 8 + title + chevron). Retire the 2x3 tile gridpage and More grid as reachable routes. Build rows from settingsrow.c with bounded count (<=12 per page).
- Acceptance criteria:
  - No gridpage tile screen is reachable from Home/Search/Library; the header has exactly one avatar entry at left and no vertical-ellipsis or gear.
  - Avatar tap opens drawer with scrim; tapping scrim or swiping left closes it; Back from Settings returns to drawer-closed Home with scroll/tab state preserved.
  - Every former tools_screen/More destination (music, wireless, audiobooks, settings, DAC, Game Boy, Books, Files, Flappy Bird, Metronome) is reachable in <=3 taps from the avatar and is presented with #121212 background, font_ui_24 header and 72 px rows.
  - All rows and back targets have >=44 px hit areas measured from live bounds.
  - Mini-player stays visible below drawer-launched pages and music keeps playing across entry/exit.

### Settings sub-pages: toggle rows, steppers and radio pickers (DAC page toggles, Game Boy settings, Ebook reading-bar settings, Reader font/theme sheets)

- Audit IDs: XTRA-DAC, XTRA-GB-SETTINGS, XTRA-GB-SETTINGS-INGAME, XTRA-EBOOK-BAR-SETTINGS, XTRA-EBOOK-READER-FONT, XTRA-EBOOK-READER-THEME
- Research date 2026-11-02; context: Spotify mobile Android and iOS, Settings and privacy sub-pages as of the 2025 support articles. Equalizer: Android puts it under Playback > Listening controls, iOS directly under Playback; Android EQ may delegate to the device system EQ. Version/date of exact styling unknown.
- Sources:
  - [Equalizer - Spotify Support](https://support.spotify.com/us/article/equalizer/), published undated, accessed 2026-11-02, read
  - [How To Open And Adjust Your Spotify Settings On Android](https://www.slashgear.com/1965140/how-to-open-spotify-settings-on-android), published 2025-09-14, accessed 2026-11-02, read
  - [Data saver - Spotify Support (fetch HTTP 429)](https://support.spotify.com/us/article/data-saver/), published undated, accessed 2026-11-02, snippet
  - [Tom's Guide: Spotify equalizer (fetch returned title only)](https://www.tomsguide.com/how-to/how-to-use-the-spotify-equalizer-on-ios-and-android), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Settings sub-pages are dark lists: section heading (bold, white), rows with title left and a green pill switch right, optional grey description line beneath, and value rows ('Crossfade' slider with seconds label, quality rows with chevron to radio pickers where the selected item shows a green check). The equalizer is an on/off row plus a graph with draggable dots (bass to treble) and preset chips (support article read; preset names not given there). Green #1ed760 only marks active state/primary action.
- Observed behavior: Switches apply instantly with no Save/Cancel. Row with value opens a picker (radio list in a bottom sheet or sub-page); choosing closes it and shows the new value on the parent row. Equalizer cannot be changed while Connect is playing on another device (support, read) - the control is disabled/explains why. Sliders show the live value and update continuously.
- Evidence quality: documented
- Conflicts / confidence: Medium: behavior of instant apply and green switch is consistent across support text and general knowledge, but exact dimensions and the radio-picker visuals are inferred from the existing project design system and dated screenshots not obtainable here (Tom's Guide not read, YouTube not reachable). R1-only settings (GB palette, shader, boot ROM, reading bar) have no Spotify counterpart.
- User requirement overrides: none recorded
- R1 adaptation: Build every extras settings page from settingsrow.c rows: toggle row 72 px (title font_ui_18 white, description font_ui_16 #b3b3b3, 52x32 green #1ed760/#535353 switch, whole row tappable >=44). Value rows (GB palette 5 options, shader 4 options, page-turn mode, bar scope Book/Chapter) open the shared bottom sheet (#202124, 16 px radius, grabber, title font_ui_20, radio rows 64 px with green check) instead of pills/popovers; ebook theme paper/sepia/night is a radio sheet with a 40x40 swatch per row. Reader font stepper (size 14-50, line spacing, margins, word spacing) becomes a sheet with four rows: label + value + minus/plus 44x44 round #282828 buttons, plus a 3-line live preview; use the Spotify playback-speed pattern (slider + preset pills 44 px high) for text size. Bar settings keep a live paper preview card pinned at top (height 160) with toggle rows below. DAC page: 'USB DAC mode' and 'USB charging' as toggle rows with descriptions, sample rate/state as a read-only value row, disabled rows greyed #535353 with an explanatory line.
- Acceptance criteria:
  - Each of the listed pages renders on #121212 using only settingsrow/bottomsheet components with no legacy topbar, accent colours or pill popovers.
  - Toggles apply immediately, persist across restart, and reflect state after re-entry.
  - Value pickers open the shared bottom sheet and show the green check on the current value; sheet dismisses on scrim tap and Back.
  - Reader font sheet changes are visible in a live preview within 200 ms and persist; GB settings opened from the in-game menu return to the paused game, which resumes.
  - Live bounds show every row >=44 px high and no horizontal overflow at 480 px.

### Connect device picker as pattern for DAC / output-device page

- Audit IDs: XTRA-DAC
- Research date 2026-11-02; context: Spotify mobile Android and iOS. Old UI (Linn docs, undated, pre-2020 style) used a 'Devices Available' bar; since 2020-09-28 the Now Playing bar has a Connect device icon left of the heart (Android Police). Current support text: tap the device icon. iOS-specific: local network permission and no hardware volume button control; Android-specific: none on picker. Premium-gating in older Linn doc.
- Sources:
  - [Spotify Connect - Spotify Support](https://support.spotify.com/us/article/spotify-connect), published undated, accessed 2026-11-02, read
  - [Spotify's latest shortcut should help streamline casting - Android Police](https://www.androidpolice.com/2020/09/28/spotifys-latest-shortcut-should-help-streamline-casting/), published 2020-09-28, accessed 2026-11-02, read
  - [Linn: Spotify Mobile App](https://docs.linn.co.uk/wiki/index.php/Spotify:Mobile_App), published unknown (old UI), accessed 2026-11-02, read
- Observed appearance: A full-height dark picker (opened from the green device icon in the player/Now Playing bar) with title 'Connect to a device', 'This phone' first, other devices as rows with device icon + name + type; the active row is green with a small animated equaliser glyph (this equaliser/green detail is from product knowledge and the user brief, not verified by a read source). Volume slider reachable from the same menu (support, read). After selecting a device the Now Playing bar shows the device name in green (Linn, old UI).
- Observed behavior: Tap icon -> picker; tap device -> playback transfers, picker stays or closes; 'Change quality settings' available for supported devices; Settings > Apps and devices > Local device visibility filters the list to the local network (support, read). The device picker never has a Save step.
- Evidence quality: documented
- Conflicts / confidence: Medium: textual flow documented; the exact sheet layout and green equaliser glyph are not confirmed by a read source (visual detail inferred). Older Linn description conflicts with post-2020 icon-based entry; treat Linn as legacy.
- User requirement overrides: none recorded
- R1 adaptation: DAC is an R1 output route, so model it as a Connect-style sheet reachable from the avatar drawer > Apps and devices AND (optional) the player output icon. Sheet title 'Output device' font_ui_20; rows 72 px: 40 px icon in #282828 circle, name font_ui_18, state font_ui_16 #b3b3b3 ('44.1 kHz', 'USB DAC connected', 'Not connected'); active row text and icon #1ed760 with a 3-bar animated glyph that animates only while audio plays (<=4 fps timer to save CPU). Rows: 'This device (speaker/headphone)', 'USB DAC', Bluetooth entries if present. Beneath the list, a toggle row 'USB charging' with description. Selecting USB DAC switches DAC mode on, with a spinner row for <=2 s and snackbar on failure. Never block the UI thread when switching.
- Acceptance criteria:
  - DAC mode and USB charging are controlled from a Spotify-style sheet/page, with legacy DAC layout unreachable.
  - Active output row is green with glyph; sample-rate text updates from real driver state, not a constant.
  - Toggling DAC while music plays keeps or cleanly resumes playback within 3 s, with a visible failure snackbar if the mode change fails.
  - Rows >=44 px, no overlap, and state survives re-entry and restart where persisted.

### Local Files list and per-row context sheet (File explorer list and row menu)

- Audit IDs: XTRA-FILES-LIST, XTRA-FILES-ROWMENU
- Research date 2026-11-02; context: Spotify mobile Android and iOS, Local audio files. Support article (undated, fetched 2026-11-02) states enable path: profile picture > Settings and privacy > Apps and devices > Local audio files, then Your Library > Local Files. Android: files via download/USB; iOS: Files app > On My iPhone > Spotify folder. Premium vs Free reports conflict in secondary sources.
- Sources:
  - [Local files - Spotify Support](https://support.spotify.com/us/article/local-files/), published undated, accessed 2026-11-02, read
  - [Android Central: Spotify testing local song playback (body not retrieved)](https://www.androidcentral.com/spotify-app-testing-local-song-playback-library-ui-changes-and-more), published unknown, accessed 2026-11-02, snippet
  - [Local files for Android - Spotify Community (title only)](https://community.spotify.com/t5/Android/Local-files-for-Android/td-p/5303232), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Local Files appears as a pinned collection in Your Library (grey/purple tile with a folder-style glyph) and opens the standard collection page: header, shuffle/play, then a track list of 64-76 px rows with title, artist/subtitle and a trailing 3-dot button. Spotify has no folder tree, rename, move, copy or delete of files; those are R1-only.
- Observed behavior: Row tap plays; 3-dot opens the context bottom sheet (header with art/title/subtitle, then rows with icon + label). Local-file support requires a storage permission. Local files can be unavailable or removed from playlists on error (support, read).
- Evidence quality: documented
- Conflicts / confidence: High for enable path and location; low for the file tree because Spotify does not offer one, so folder/row-menu structure is an R1-only adaptation based on the shared collection page and context sheet patterns.
- User requirement overrides: none recorded
- R1 adaptation: Render the file explorer on the shared collection-page chrome: header 64 px with back chevron and current folder name (font_ui_24, ellipsised) plus a breadcrumb line font_ui_16 #b3b3b3 of the parent path; no legacy topbar. Rows 64 px (folder icon tile 48 px #282828 radius 4, name font_ui_18, subtitle font_ui_16 '12 items' or file size/date, trailing 44x44 3-dot). First row '..' (Up) only when not at sd_root_path. Use bounded pagination (render <=40 rows, lazy-load next page on scroll end) and show skeleton rows while the directory read runs off the UI thread. Folder-add action is a 56 px circular green FAB at lower right above the mini-player (+ glyph) instead of a corner button. Row 3-dot opens the shared bottom sheet (#202124): header = type icon + name + path; rows 56 px: Rename, Move to, Copy to, Delete (Delete label in #ff5a5f? only if the design system allows; otherwise white). Image/text files tap through to viewers with a Spotify-style header.
- Acceptance criteria:
  - File explorer uses collection-page header, 64 px rows and shared bottom sheet; no legacy list or topbar visible.
  - Row menu opens as a bottom sheet with exactly Rename, Move to, Copy to, Delete; scrim/Back dismisses with no state change.
  - Directory listing of >500 entries stays responsive (no UI-thread block >100 ms) and renders at most a bounded number of widgets.
  - Up navigation, folder state and scroll position restore after returning from a viewer; mini-player remains visible and playback uninterrupted.

### Name entry (New folder / Rename) and destination picker (Move to / Copy to)

- Audit IDs: XTRA-FILES-NAME, XTRA-FILES-PICKER
- Research date 2026-11-02; context: Spotify mobile Android/iOS playlist creation and Add-to-playlist flows. Support article create-playlists read 2026-11-02: mobile labels 'Create' > 'Playlist', 'Name your playlist, and tap Create', 'Add to this playlist', 'Edit', 'Done'; mobile renaming steps absent; 'Save' appears only in desktop steps. Latest app visuals (full-screen name entry with Cancel/Save, Add-to-playlist picker with New playlist and Done) are not confirmed by a read source.
- Sources:
  - [Create and edit playlists - Spotify Support](https://support.spotify.com/us/article/create-playlists/), published undated, accessed 2026-11-02, read
  - [Spotify newsroom: Liked Songs plus button (search snippet re: green check)](https://newsroom.spotify.com/tag/liked/), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Name entry is a full-screen dark page: centred large editable name (bold, underlined/caret), Cancel top-left (text), Save/Create green pill at the bottom or top-right, with the OS keyboard rising beneath. The Add-to-playlist picker is a list of collections (cover 48 px, name, count) with a 'New playlist' row on top and a green 'Done' pill. The visual layout is partly from product knowledge, not a read source.
- Observed behavior: Name entry: keyboard opens automatically, the confirm button is disabled/neutral until text is non-empty; Cancel discards without confirmation; Save commits and returns. Picker: selecting rows toggles a green check and Done commits; there is a New playlist shortcut. Creating a playlist on mobile uses a name step ('Name your playlist, and tap Create', read).
- Evidence quality: inferred
- Conflicts / confidence: Medium-low: only the text flow (name then Create; Edit/Done) is documented; Cancel/Save placement and picker visuals are inferred. Do not claim pixel fidelity.
- User requirement overrides: none recorded
- R1 adaptation: Name layer: full-screen #121212 page, header 64 px with 44x44 X at left (Cancel, discards, no confirm) and title 'New folder'/'Rename' font_ui_20; text field centred, height 56, font_ui_28 white, underline 2 px green on focus; helper line font_ui_16 #b3b3b3 for invalid characters/duplicate name; pill button 'Create'/'Save' 56 px high, 200 px wide, #1ed760 with black font_ui_18 bold, disabled #282828/#535353 while empty or invalid; Spotify keyboard (keyboard.c) occupies the lower ~330 px so the field and button must sit above it (field y~150, button y~230). Destination picker: full-height page (not a small sheet, because 480x800 allows >8 rows) with header 'Move to'/'Copy to', current path line, folder-only rows 64 px with chevron-in, '..' row, and a sticky bottom bar 72 px with green pill 'Move here'/'Copy here' (disabled when destination equals source or is a descendant of the moved folder). 'New folder' row at top of the picker reuses the name layer.
- Acceptance criteria:
  - Name layer and picker use no legacy topbar/title and share keyboard.c; X and Back cancel without writing to disk.
  - Create/Rename is disabled for empty, duplicate, or invalid names (contains / or \0) and shows the helper text; accepted names appear in the list immediately.
  - Move here/Copy here are disabled for same-folder and into-self destinations, verified with live state.
  - Keyboard never covers the field or confirm button at 480x800; both stay >=44 px hit areas.

### Confirmation dialogs, operation progress and snackbars/toasts (file delete/copy/move, bookmark added, open errors, Flappy launch error, Metronome blocked)

- Audit IDs: XTRA-FILES-OPS, XTRA-EBOOK-OPEN-ERROR, XTRA-EBOOK-BOOKMARK-ADD, XTRA-FLAPPY-ERROR, XTRA-METRO-BLOCKED, XTRA-GB-STATE
- Research date 2026-11-02; context: Spotify mobile Android/iOS. Delete playlist: support article (read) lists 'Delete Playlist' in the 3-dot menu with no confirmation text documented; blog sources mention a pop-up Delete confirmation (low quality). Save feedback: Spotify newsroom snippet says the Plus button turns into a green check (not a snackbar). Toast/snackbar and dialog styling not confirmed from any read source.
- Sources:
  - [Create and edit playlists - Spotify Support](https://support.spotify.com/us/article/create-playlists/), published undated, accessed 2026-11-02, read
  - [Spotify newsroom: Liked tag (search snippet)](https://newsroom.spotify.com/tag/liked/), published unknown, accessed 2026-11-02, snippet
  - [SoundGuys: how to delete a playlist (search snippet; low quality)](https://soundguys.com/how-to-delete-playlist-on-spotify-101556), published unknown, accessed 2026-11-02, snippet
- Observed appearance: UNVERIFIED. Based on product convention: confirmation is a centred rounded (radius ~16) dialog on #282828 with a bold title, one grey body line and two buttons - white outlined or text Cancel and a filled pill for the confirm action; snackbars are a floating dark rounded bar (#282828/white text, radius 8) above the mini-player/nav bar, auto-dismissing after about 4 s, with optional green text action.
- Observed behavior: Destructive actions: 3-dot menu -> Delete -> (probably) confirmation -> item vanishes, snackbar. Successful adds feedback is a state change on the button (green check) rather than a modal. Errors surface as bottom snackbars not modals. Long operations: no Spotify source found for a progress modal.
- Evidence quality: inferred
- Conflicts / confidence: Low: no Spotify source read confirms dialog or snackbar styling and delete confirmation wording is undocumented; treated as inferred from convention plus the project design tokens. Record as a research gap, not fidelity evidence.
- User requirement overrides: none recorded
- R1 adaptation: Dialog: 400x ~220 px, centred, #282828, radius 16, scrim 60% black; title font_ui_20 bold ('Delete 'name'?'), body font_ui_16 #b3b3b3 ('This permanently deletes the file from the card. This can't be undone.' - files ARE deleted here by explicit user action, unlike collection removal), buttons 56 px: Cancel (text white) and Delete (filled white pill or #e22134 only if tokens allow) - destructive confirm is never green. Progress for long copy/move: use a bottom sheet (not full modal) 'Copying 3 of 12' with a 4 px green progress bar on #535353, a 44 px 'Cancel' text button, and the worker thread updating via lv_async; sheet is non-dismissible by scrim while running, Back requests cancel. Snackbar: 448x56 at x=16, y = 800 - miniplayer(72) - navbar(~56) - 8 - 56, #282828, font_ui_16 white, optional green action text; 3.5 s auto-dismiss; one at a time (replace, no stack); used for 'Bookmark added', 'Bookmark already exists', 'Bookmarks full', 'Couldn't open this book', 'Couldn't start Flappy Bird', 'Saved state'. Metronome blocked: dialog 'Stop playback first?' with 'Cancel' / 'Stop and start' (Spotify takeover-style) instead of a bare notice.
- Acceptance criteria:
  - Delete always shows the confirm dialog with Cancel as default focus; Cancel leaves the file untouched; collection removal paths never delete audio files (unchanged guarantee).
  - Copy/move progress sheet shows live count and bar, can be cancelled, and leaves no partial destination file after cancel.
  - All former toasts/popups/notices in these routes render as the shared snackbar or dialog (no legacy popup) and are positioned above mini-player/nav at all 480x800 states.
  - Only one snackbar visible; auto-dismisses; never obscures the bottom nav hit areas.

### Audiobook player controls: sleep timer and playback speed (pattern for Metronome page and Reader font/theme)

- Audit IDs: XTRA-METRO, XTRA-METRO-BEATS, XTRA-EBOOK-READER-MENU, XTRA-EBOOK-READER-FONT
- Research date 2026-11-02; context: Spotify mobile Android and iOS audiobook Now Playing, Premium audiobooks. Sleep timer announced 2024-11-21 (Engadget, Spotify newsroom) for eligible Premium users on mobile; support article lists sleep timer icon, speed icon (1x), skip 15 s, and 3-dot menu (horizontal on iOS, vertical on Android) for rating. Presets 5/10/15/30/45 min, 1 hour, 'end of chapter' reported by community and newsroom text (secondary).
- Sources:
  - [Unlock Audiobooks - Spotify Support](https://support.spotify.com/us/article/audiobooks-unlock/), published undated, accessed 2026-11-02, read
  - [Spotify drops a bunch of new tools for audiobooks, including a sleep timer - Engadget](https://www.engadget.com/entertainment/streaming/spotify-drops-a-bunch-of-new-tools-for-audiobooks-including-a-sleep-timer-173327861.html), published 2024-11-21, accessed 2026-11-02, read
  - [6 Spotify Audiobook Features That Level Up Your Listening Experience - Spotify Newsroom](https://newsroom.spotify.com/2024-11-21/6-spotify-audiobook-features-that-level-up-your-listening-experience/), published 2024-11-21, accessed 2026-11-02, read
  - [Audio book playback speed - Spotify Community (search summary only)](https://community.spotify.com/t5/Other-Podcasts-Partners-etc/Audio-book-playback-speed/m-p/6443247/highlight/true), published unknown, accessed 2026-11-02, snippet
- Observed appearance: The audiobook Now Playing screen has a row of secondary controls under the seek bar: a speed chip showing '1x' at left, skip-back 15 s, play/pause (large white circle), skip-forward 15 s, and a sleep-timer icon at right. Sleep timer and speed each open a bottom sheet: title, list of presets with the current one in green, Close/Cancel at the bottom. Sheet exact visuals inferred.
- Observed behavior: Tap speed chip -> sheet of presets; selecting applies immediately and updates chip. Tap sleep icon -> sheet of durations incl. 'End of chapter'; icon turns green and shows countdown when active. Hours are counted at 1x regardless of speed (community, secondary).
- Evidence quality: documented
- Conflicts / confidence: Medium: controls and 2024-11-21 launch documented by three read sources; preset list and 'end of chapter' wording from newsroom/community only and could vary; sheet layout inferred. Support article does not mention chapters on Now Playing.
- User requirement overrides: none recorded
- R1 adaptation: Metronome page uses the audiobook control layout: large BPM numeral font_ui_28 centred (or largest available) with 'BPM' font_ui_16 #b3b3b3, 44x44 round minus/plus buttons either side, 8 px high slider track (green fill #1ed760, 20 px thumb with 44 px touch area), preset pills (60/80/100/120/140) 44 px high radius 22 with selected = green fill + black text, 'Tap tempo' pill 56 px, and a 72 px green play/stop circle (matching mini-player glyph). Beats-per-bar pill opens the shared bottom sheet 'Time signature' with radio rows 1/4 ... 7/8 and green check (replaces popover). Animated pendulum/beat dots stay but at <=15 fps and cease when leaving the page (metronome must stop on pop). Reader menu (Chapters, Font, Theme, Close the book) is a bottom bar of four 80 px icon+label items (#202124) mirroring the audiobook secondary control row; Font and Theme open sheets described in the settings record.
- Acceptance criteria:
  - Metronome BPM, slider, tap tempo, beats and play/stop use Spotify-style controls with >=44 px hit areas; Beats chooser is the shared bottom sheet.
  - Leaving the page (Back, avatar drawer, hardware keys) always stops the metronome and releases audio.
  - Reader menu bar and its sheets use shared sheet component and #202124; selecting a preset applies immediately and persists.
  - Slider and stepper agree (value shown equals audible tempo within one beat) and tap tempo clamps to configured BPM range.

### Audiobook chapters list and bookmarks (Reader chapters, bookmark add/list/delete)

- Audit IDs: XTRA-EBOOK-READER-CHAPTERS, XTRA-EBOOK-BOOKMARK-ADD, XTRA-EBOOK-MARKS-BOOKS, XTRA-EBOOK-MARKS-LIST
- Research date 2026-11-02; context: Spotify mobile audiobooks. Support article (read) says you can't download individual chapters and describes no chapter list or bookmark control on Now Playing; Engadget/newsroom launch articles (read) mention no chapters or bookmarks. A chapters sheet and bookmarks are therefore NOT documented in the researched sources.
- Sources:
  - [Unlock Audiobooks - Spotify Support](https://support.spotify.com/us/article/audiobooks-unlock/), published undated, accessed 2026-11-02, read
  - [Engadget: Spotify audiobook tools](https://www.engadget.com/entertainment/streaming/spotify-drops-a-bunch-of-new-tools-for-audiobooks-including-a-sleep-timer-173327861.html), published 2024-11-21, accessed 2026-11-02, read
  - [9to5Google: Spotify audiobooks Included in Premium](https://9to5google.com/?p=594268), published 2023-11-08, accessed 2026-11-02, read
- Observed appearance: No read source documents a Spotify chapters sheet or bookmark rows. In the audiobook page, chapters are listed as episode-like rows under the book header (title, duration, progress ring) per the general audiobook page structure (not verified here).
- Observed behavior: Audiobooks are split into chapters/tracks; the current chapter is what the sleep timer 'end of chapter' option refers to (newsroom, secondary). No documented bookmark action.
- Evidence quality: unavailable
- Conflicts / confidence: Low. Treat this as a research gap: neither chapters-sheet nor bookmark behavior was verified. The adaptation uses the project's own bottom sheet and track-row patterns, which are verified elsewhere in the product.
- User requirement overrides: none recorded
- R1 adaptation: Chapters: bottom sheet, 85% screen height (680 px), #202124, grabber, title 'Chapters' font_ui_20 plus book title font_ui_16 #b3b3b3. Rows 64 px: index/number font_ui_16 #b3b3b3 (width 32), chapter title font_ui_18 (current = #1ed760 with small green dot), optional page range right-aligned font_ui_16. Build lazily in batches of 12 (existing), auto-scroll to the current chapter, tap jumps and closes. Bookmarks: 'Add bookmark' as a row in the reader menu plus the existing double-tap gesture; confirmation via the shared snackbar 'Bookmark added' (duplicate: 'Already bookmarked'; full: 'Bookmarks full - delete one'). Bookmark books list = collection-style list, rows 72 px with 48 px cover (async, bounded cache), title font_ui_18, '3 bookmarks' font_ui_16 #b3b3b3. Bookmarks list rows 72 px: snippet two lines font_ui_16, location/progress font_ui_16 #b3b3b3, trailing 44x44 3-dot -> sheet with 'Delete bookmark' (replaces long-press popover; keep long-press as shortcut). Empty state: centred 'No bookmarks yet' font_ui_20 bold + one grey line + no CTA.
- Acceptance criteria:
  - Chapters, bookmark books and bookmark rows use the shared sheet/list components; no legacy list.
  - Current chapter is highlighted and scrolled into view on open; selecting jumps to the exact chapter and persists reading position.
  - Deleting the last bookmark shows the empty state without navigating away unexpectedly; deleting never touches the epub file.
  - Large TOC (>500 entries) opens in <300 ms (first batch) and remains scrollable on 64 MB budget.

### Audiobook / Library shelf and empty states (Books shelf, empty Ebook folder, GB list states)

- Audit IDs: XTRA-EBOOK-SHELF, XTRA-EBOOK-SHELF-EMPTY, XTRA-GB-LIST, XTRA-GB-LIST-STATES
- Research date 2026-11-02; context: Spotify mobile Your Library (Android/iOS) with filter chips (Playlists, Artists, Albums, Podcasts & shows, Audiobooks) and Local Files; Premium audiobook rows and 'Included in Premium' badge (9to5Google 2023-11-08, read). Empty-state wording ('Your library is empty' with pill CTA) is not confirmed by a read source.
- Sources:
  - [9to5Google: Spotify audiobooks Included in Premium](https://9to5google.com/?p=594268), published 2023-11-08, accessed 2026-11-02, read
  - [Local files - Spotify Support](https://support.spotify.com/us/article/local-files/), published undated, accessed 2026-11-02, read
  - [Android Police: new Library management UI (search result only)](https://www.androidpolice.com/2021/04/29/spotifys-new-and-better-library-management-ui-is-rolling-out-next-week/), published 2021-04-29, accessed 2026-11-02, snippet
- Observed appearance: Library filter chips are a horizontal row of pill chips (selected = green fill with black text, unselected #282828 with white text). Items appear as list rows or grid tiles with 1:1 covers; audiobook items use 1:1 covers with an 'Included in Premium' label. Skeleton loading rows and empty-state CTA styling inferred.
- Observed behavior: Tap a cover opens the item page; chips filter; library offers list/grid toggle. Empty states show a short headline, one grey line and a green pill CTA.
- Evidence quality: inferred
- Conflicts / confidence: Medium-low: only chip/cover structure is partially documented; empty/skeleton states inferred.
- User requirement overrides: none recorded
- R1 adaptation: Books shelf: collection page with two-column cover grid (cover 208x208 at 480 px with 16 px gutters, title font_ui_16 white up to 2 lines, author font_ui_16 #b3b3b3), cap of 12 visible cells in memory plus recycled cells; covers load asynchronously through a bounded cache (<=24 thumbnails). Header with back, 'Books' font_ui_24 and two 44x44 icon buttons (bookmarks, reading bar settings) at right replacing the corner buttons. Game Boy list: chips 'GB' / 'GBC' (selected green pill 44 high) over 72 px ROM rows (title font_ui_18, size/saves font_ui_16 #b3b3b3), gear in the header. States: scanning = 6 skeleton rows (#282828 pulses, no animation faster than 2 Hz); empty = centred 48 px icon, 'No books yet' / 'No games yet' font_ui_20 bold, line 'Put .epub files in the Ebook folder on the card' font_ui_16 #b3b3b3, optional pill 'Open Files' 56 high green; failure = 'Couldn't read the card' + 'Try again' pill.
- Acceptance criteria:
  - Shelf, GB list and their empty/scanning/error states are rendered with shared components (chips, rows, empty-state block), none with legacy layout.
  - Scanning never blocks the UI thread; skeleton shows within 100 ms; empty state names the exact folder path searched.
  - Cover/ROM lists use bounded widgets and cache; scrolling stays smooth with >200 items.
  - Retry from the error state re-scans and recovers without restart.

### R1-only utilities with no Spotify equivalent: Reader canvas, Game Boy play/menu/save-state, Flappy Bird

- Audit IDs: XTRA-EBOOK-READER, XTRA-GB-PLAY, XTRA-GB-MENU, XTRA-GB-STATE, XTRA-FLAPPY, XTRA-FILES-VIEWERS
- Research date 2026-11-02; context: No Spotify counterpart. Closest patterns: Now Playing overflow bottom sheet (header with art/title, then icon rows) and audiobook Now Playing secondary controls (read support article). Reader page chrome could be compared to Apple Books/Kindle but no source was read for them.
- Sources:
  - [Unlock Audiobooks - Spotify Support (Now Playing controls)](https://support.spotify.com/us/article/audiobooks-unlock/), published undated, accessed 2026-11-02, read
- Observed appearance: None (R1-only). Adaptation keeps the fullscreen canvases (page text, 480x432 emulator frame, Flappy playfield) but replaces the chrome around them with the Spotify language: #121212 surfaces, bottom sheets, green pills, font_ui_* type.
- Observed behavior: None documented. The Now Playing overflow sheet shows a header (cover 56 px, title, subtitle) and rows (icon + label, 56 px) with a Close affordance; dismissal by scrim/drag.
- Evidence quality: inferred
- Conflicts / confidence: Not applicable (R1-only). Recorded as a deliberate adaptation per AGENTS.md; no claim of Spotify behavior.
- User requirement overrides: none recorded
- R1 adaptation: Game menu (centre press): shared bottom sheet #202124, header = ROM name font_ui_20 + console/palette font_ui_16 #b3b3b3; rows 56 px: 'Create save state', 'Load save state' (disabled grey when no state, with date as secondary line), 'Settings', 'Quit game' (the last separated, white); emulator pauses while the sheet is open and resumes on dismiss. Save-state result via snackbar ('Saved to slot 1' / 'Couldn't save state'), not a popup. On-screen D-pad/A/B/Start/Select remain over #121212 with 56-72 px hit targets (A/B 72 px circle #282828 border, green only while pressed). Reader: no persistent header; long-press centre opens a bottom bar sheet (Chapters, Font, Theme, Close) in the shared sheet style; page toast for page position uses the snackbar style; book close returns to Books shelf with state preserved. Flappy Bird: keep full-screen game, but title/game-over overlays use Spotify pills (Play green 56 px, Menu/Exit outlined #535353) and font_ui_28/20; Exit returns to the Extras page. Text/image viewers: header 64 px with back + file name font_ui_20 + 3-dot to sheet (Rename/Delete is not exposed there); text uses font_ui_16 on #121212 with 16 px gutters; image viewer pinch/zoom optional, loading shows a spinner, failure shows the snackbar 'Couldn't open this file'.
- Acceptance criteria:
  - All in-game / in-reader menus use the shared sheet; no legacy popup or gridpage chrome remains.
  - Opening the game menu pauses emulation; dismissing resumes without audio glitches; Quit returns to the ROM list with its scroll position.
  - Flappy and GB failures (missing art/OOM, bad ROM) show snackbar/dialog and return cleanly, never leaving a black screen.
  - Touch zones in reader/emulator keep documented behavior (edges for page turn, centre long-press menu) and are verified against live bounds.

### Cross-cutting audio focus: music and hardware keys when an extra starts (Game Boy, Flappy, Metronome, Reader)

- Audit IDs: XTRA-XCUT-AUDIO, XTRA-METRO-BLOCKED, XTRA-GB-PLAY, XTRA-FLAPPY
- Research date 2026-11-02; context: Spotify mobile on Android/iOS follows OS audio-focus rules (pause on call/other media, resume when focus returns). No Spotify source was read that documents this precisely; only that Connect playback blocks local audio-setting changes (equalizer support article, read).
- Sources:
  - [Equalizer - Spotify Support (EQ can't be changed while using Connect)](https://support.spotify.com/us/article/equalizer/), published undated, accessed 2026-11-02, read
- Observed appearance: No visible UI is documented; interruptions surface via the Now Playing/mini-player state changing to paused and, for takeovers, a Connect banner.
- Observed behavior: Spotify pauses when another app takes audio focus and may resume when it ends; the mini-player always reflects the true state. The only documented coupling is that some audio settings are unavailable while playing through Connect.
- Evidence quality: unavailable
- Conflicts / confidence: Low; research gap. The adaptation is a product decision for R1, not Spotify fidelity.
- User requirement overrides: none recorded
- R1 adaptation: Policy: Game Boy and Flappy Bird pause (not stop) the music player on entry and show a one-time snackbar 'Music paused' (font_ui_16); on exit they offer no auto-resume unless the music was playing before entry, in which case resume with a 300 ms fade. Metronome refuses to start while music, Bluetooth receiver or AirPlay is playing via the dialog 'Stop playback first?' (Cancel / Stop and start). Reader never touches audio, so music continues; hardware volume keys always map to system volume; hardware play/pause maps to the music player except while a game is foreground (then it opens the game menu). Mini-player is hidden over full-screen games and returns on exit; it remains over Files/Books/Metronome pages.
- Acceptance criteria:
  - Entering/exiting each extra while music plays leaves the player in a defined, visible state (paused or playing) that matches the mini-player.
  - Metronome cannot play over active music/BT/AirPlay without explicit confirmation; confirming stops the other source cleanly.
  - Repeated entry/exit (10x) of GB, Flappy and Metronome leaves no stuck audio device, leaked threads, or lost queue position.
  - Hardware keys behave per policy and are verified with the simulator key events.

Failed attempts:

- support.spotify.com/us/article/privacy-and-social-controls/ WebFetch returned HTTP 429 (retried once, 429 again); only the search-result summary was seen, actually_read=false.
- support.spotify.com/na-af/article/local-files/ returned 429; the /us/ variant succeeded on retry.
- support.spotify.com/us/article/audiobooks/ returned 429; the /audiobooks-unlock/ article succeeded instead.
- support.spotify.com/us/article/playlists/, /storage/, /data-saver/ returned 429 or were not found; clear-cache returned 'Page not found'. No Spotify Storage/Data-saver article was read, so the storage page pattern is unresearched.
- community.spotify.com threads (Local-files-for-Android, Where-is-the-sleep-timer-control, How-do-I-access-settings, How-do-I-delete-a-playlist) returned only the page title, no replies; treated as not read.
- phonearena.com Recents article returned HTTP 403; trustedreviews Connect explainer returned HTTP 530; androidcentral and tomsguide returned only titles/nav.
- itc.ua article on Android Connect media switcher fetched but contained no relevant content.
- spotify.design / Encore web search returned only third-party Flutter packages; no official Spotify Design or Encore bottom-sheet/dialog/snackbar spec was found, so dialog and snackbar visuals are inferred.
- No dated YouTube video page was found or fetchable (search returned no YouTube URLs); no dated visual teardown of the current Android account drawer, Connect picker or Settings page was retrieved, so pixel-level appearance remains inferred.
- No Spotify source documents audiobook chapters sheet or bookmarks, delete-playlist confirmation text, snackbar styling, or audio-focus behavior; recorded as unavailable/inferred rather than invented.
- Hardware/device performance for all R1 adaptations remains unmeasured; heights/fonts are design proposals against ui_metrics.h tokens and were not rendered or tested (read-only task).

## core

### Bottom navigation (Home / Search / Your Library) and per-tab state; mini-player (Now Playing bar) tap, swipe and play toggle

- Audit IDs: SHELL-NAV, SHELL-MINI
- Research date 2026-11-02; context: iOS and Android. The three-tab bar dates from 18 Oct 2018 (Premium update, later all users). The Now Playing bar behaviours come from an iGeeksBlog roundup last updated 6 May 2026, which does not split iOS and Android. Experiments: Neowin reports an undated A/B test of a full-art Now Playing, which I did not read. Unknowns: current app version, per-tab back-stack behaviour.
- Sources:
  - [Spotify streamlines mobile navigation with Premium update](https://rainnews.com/?p=27701), published 2018-10-18, accessed 2026-11-02, read
  - [7 Spotify gestures every user should know](https://www.igeeksblog.com/spotify-gestures-every-user-should-know/), published updated 2026-05-06, accessed 2026-11-02, read
  - [Spotify Connect (Spotify Support)](https://support.spotify.com/article/spotify-connect), published undated, 2026 copyright, accessed 2026-11-02, read
  - [Spotify ditches the hamburger menu in iOS app redesign (search snippet only)](https://techcrunch.com/?p=1316422), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Bottom bar with exactly three tabs: Home, Search, Your Library. The Now Playing bar sits directly above it and shows the track title and artist, a device (Connect) icon, and a play/pause control. Both persist across the main tabs.
- Observed behavior: Tapping the Now Playing bar opens the full Now Playing screen. Swiping left or right on the bar skips to the next or previous track without opening the player. Swiping down on the expanded player returns to the page underneath. The device icon in the bar opens the Connect picker. Per-tab back stacks are assumed and not confirmed by any source I read.
- Evidence quality: documented
- Conflicts / confidence: High for the 3 tabs and bar tap/swipe. Medium for 'tapping the play glyph toggles' (inferred from the bar's controls). Low for per-tab back stacks (not sourced). The 2018 article does not mention a Premium tab. No dated screenshots were examined.
- User requirement overrides: none recorded
- R1 adaptation: Bottom nav 64 px tall, three equal 160 px hit zones, icon 24 plus font_ui_16 label, active tab white with green indicator and inactive #b3b3b3. Mini-player 64 px high, #282828, directly above the nav: 48 px cover, a title line in font_ui_16 and an artist line in font_ui_16 in #b3b3b3, and a play/pause glyph in a 56x56 hit box. Add a 2 px green progress hairline along the bar's bottom edge. Keep horizontal swipe on the mini-player = next/previous (the source documents this and it is missing from the SHELL-MINI journey). Vertical drag up and tap open Now Playing. Remember the last tab, and keep a separate scroll/back state per tab: leaving a collection page must not discard the other tabs' state.
- Acceptance criteria:
  - Tapping each of the 3 tabs from a collection, artist or discography page with the footer mounted switches tabs with no legacy screen flash, and the last tab is restored after restart.
  - The mini-player is 64 px high with a >=56x56 play/pause hit area. Tapping the glyph toggles playback without expanding Now Playing, and tapping elsewhere on the bar expands it.
  - A horizontal swipe on the mini-player skips track, or the gap is recorded in docs/UI_AUDIT.md.
  - Returning to a tab restores its scroll position (to be tested).

### Settings entry, Settings list rows/toggles, and the R1 'tools' grid (Music, Wireless, Audiobooks, More, Settings)

- Audit IDs: SHELL-TOOLS, SHELL-SETTINGS
- Research date 2026-11-02; context: iOS and Android. Older apps (about 2017 to 2021) had a gear on the Home header. Current apps open 'Settings and privacy' from the profile picture at top left of Home/Search/Your Library (or a left swipe), per Spotify community staff replies seen only as search-result snippets. Equalizer path confirmed on both platforms: profile picture, Settings and privacy, Playback. On Android the toggle is under 'Listening controls'. The Equalizer page has a 2026 copyright and no publish date. Unknowns: the exact current row styling.
- Sources:
  - [Equalizer (Spotify Support)](https://support.spotify.com/bs/article/equalizer/), published undated, 2026 copyright, accessed 2026-11-02, read
  - [Local Files (Spotify Support)](https://support.spotify.com/article/local-files/), published undated, accessed 2026-11-02, read
  - [How do I access settings? (Spotify Community, title only returned)](https://community.spotify.com/t5/Android/How-do-I-access-settings/m-p/5649310), published unknown, accessed 2026-11-02, snippet
  - [Design Critique: Spotify Mobile App (search snippet only)](https://ixd.prattsi.org/2017/01/design-critique-spotify-mobile-app/), published 2017-01, accessed 2026-11-02, snippet
- Observed appearance: Settings and privacy is a plain full-screen list with a back arrow. It has section headings (for example Playback, Apps and devices), rows with a title, an optional subtitle, and a trailing chevron or toggle, and Android adds a 'Listening controls' heading. Local audio files is a toggle under Apps and devices. The Equalizer has an on/off toggle, preset chips, and draggable dots for each band. Spotify has no grid of feature tiles.
- Observed behavior: Settings opens from the profile menu. Each row opens a sub-page or flips a toggle in place. Audio settings including the Equalizer cannot be changed while Connect is playing on another device. The Local Files toggle surfaces a 'Local Files' entry in Your Library.
- Evidence quality: documented
- Conflicts / confidence: High that settings are a list-based hierarchy with toggles. The profile-picture entry in current apps is medium (staff replies seen as snippets). The tools grid has no Spotify equivalent (R1-only). I did not find a dated visual for the row heights, so the numbers below are R1 choices.
- User requirement overrides: none recorded
- R1 adaptation: Replace the tools grid with a 'Settings and privacy' style list page in the Spotify shell: 56 px top bar (back arrow 48x48 hit area, title in font_ui_20 white) on #121212. Group headings use font_ui_16 in #b3b3b3 (not uppercase), and rows are 64 px with a font_ui_18 title, a font_ui_16 #b3b3b3 subtitle and a trailing chevron or a 52x28 green toggle (#1ed760 on, #535353 off). Place Music, Wireless (Bluetooth/Wi-Fi), Audiobooks and More as rows inside this list, and put the header ellipsis and gear on one entry, so there is a single route (Settings and privacy). The legacy gridpage must be unreachable. Equalizer sits under Playback with a toggle, preset pills and band sliders on dark surfaces. Block edits while casting is not relevant on R1 (no Connect).
- Acceptance criteria:
  - Home/Search header ellipsis and gear both reach the same Spotify-styled Settings list with the #121212 background, with no gridpage tile screen.
  - Every row is >=56 px high with a >=44 px hit area, toggles use #1ed760, and back returns to the originating tab at the same scroll state.
  - Every nested page (Music, Wireless, Audiobooks, More, Equalizer) reuses the same list/header component and has no legacy topbar.
  - The record lists R1-only rows (Cover Flow, Bluetooth, Power) and marks them 'none (R1-only)'.

### Home feed: filter chips, shelves, card play affordance, card long-press options, and 'Customize Home'

- Audit IDs: HOME-FEED, HOME-OPTIONS, HOME-CUSTOMIZE
- Research date 2026-11-02; context: iOS and Android. The August 2022 redesign (Android first, iOS after) added Music and 'Podcasts & Shows' feeds at the top of Home. I found NO source for an 'All' chip or an 'Audiobooks' chip, so those are unverified. Separate Play and Shuffle buttons on collections reached Premium users from 2022 to 2023 (a community post updated 2023-11-12). Long-press behaviour is inconsistent per community reports. Customizing the Home order is not a Spotify feature: none (R1-only).
- Sources:
  - [Spotify updates Home screen with new personalized discovery feeds](https://techcrunch.com/2022/08/09/spotify-updates-home-screen-with-new-personalized-discovery-feeds-music-podcasts), published 2022-08-09, accessed 2026-11-02, read
  - [Spotify redesigns app home screen](https://9to5mac.com/2022/08/09/spotify-redesigns-app-home-screen/), published 2022-08-09, accessed 2026-11-02, read
  - [7 Spotify gestures every user should know](https://www.igeeksblog.com/spotify-gestures-every-user-should-know/), published updated 2026-05-06, accessed 2026-11-02, read
  - [Android Long Press (Spotify Community, search snippet only)](https://community.spotify.com/t5/Live-Ideas/Android-Long-Press-on-Android/idc-p/5246178/highlight/true), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Home has a row of pill chips at the top (Music and Podcasts & Shows in the sources read). Below it are a 2-column quick-access grid of compact cards (cover on the left, title on the right) and horizontal shelves of square cover cards with a title and subtitle underneath. The feed cards carry like, share and play buttons.
- Observed behavior: Tapping a chip filters the whole feed, and an 'X' on the active chip returns to the full Home. Tapping a card opens the page and the play control starts playback. Long-press or the card ellipsis opens a bottom sheet of context actions. Swiping right on a track row adds it to the queue (green queue icon).
- Evidence quality: documented
- Conflicts / confidence: Chips and card affordances are high confidence for 2022 (two sources agree). 'All' and 'Audiobooks' chips are unverified, so they are inferred as the product's own extension. I did not read an official Home long-press sheet spec (medium to low). Customize Home has no counterpart.
- User requirement overrides: none recorded
- R1 adaptation: Keep the chips (All/Music/Audiobooks) as 40 px high pills with a 44 px touch strip, font_ui_16, active = #1ed760 fill with black text, inactive = #282828 with white text, and tapping the active chip returns to All. Quick access: 2 columns x 56 px tiles on #282828. Shelf cards are 144x144 covers with a font_ui_16 title (1 line) and a #b3b3b3 subtitle, with a 40 px green play disc at the cover's lower right (>=44 hit area with padding) that plays without navigating. Long-press and the '...' open the shared bottom sheet (sheet #202124, rows 56 px, font_ui_18). Replace the +/- text toggles and 'Up' in Customize Home with the shared sheet pattern: a row per section with a toggle (green) and a drag grip at the right, 56 px rows, a 'Done' green pill, and mark it 'none (R1-only)'. Bound each shelf to a paginated query of at most 12 cards and load covers asynchronously.
- Acceptance criteria:
  - Chip taps change the feed and the active chip toggles off to All, with a >=44 px hit height.
  - Card tap opens the collection and the green play disc starts playback without navigation; long-press opens the shared sheet with the 4 listed actions.
  - Customize Home uses sheet rows with toggles and grips (no '+/-' text), persists across restart, and always keeps at least one section visible.
  - Each shelf is capped (<=12 cards) with async covers.

### Audiobooks on Home and the audiobook details sheet; audiobook player (chapters, bookmarks, speed, sleep timer)

- Audit IDs: HOME-AUDIOBOOKS, NP-AUDIOBOOK
- Research date 2026-11-02; context: Mobile (iOS and Android). Audiobooks launched for US Premium in Nov 2023 (15 h per month) and were found on Home, in Search and in an audiobooks hub per launch coverage seen in search snippets. Nov 21 2024 added a sleep timer, audiobook playlists, countdown pages and follow-along visuals, for select regions and mobile only. The official audiobook support page I tried returned 404. A support snippet says 'Playback speed 1x' is tapped in Now Playing. Chapters and bookmarks are NOT confirmed by any page I read.
- Sources:
  - [Spotify drops a bunch of new tools for audiobooks, including a sleep timer](https://www.engadget.com/entertainment/streaming/spotify-drops-a-bunch-of-new-tools-for-audiobooks-including-a-sleep-timer-173327861.html), published 2024-11-21, accessed 2026-11-02, read
  - [Spotify enhances audiobook experience with author pages, video clips, sleep timer](https://alternativeto.net/news/2024/11/spotify-enhances-audiobook-experience-with-author-pages-video-clips-sleep-timer-and-more), published 2024-11-21, accessed 2026-11-02, read
  - [Spotify newsroom: sleep timer update](https://newsroom.spotify.com/tag/sleep), published 2025-08-29, accessed 2026-11-02, read
  - [Audiobooks (Spotify Support): returned a 404 page](https://support.spotify.com/uk/article/audiobooks/), published n/a, accessed 2026-11-02, snippet
  - [Audio book playback speed (Spotify Community, title only)](https://community.spotify.com/t5/Other-Podcasts-Partners-etc/Audio-book-playback-speed/m-p/6443247/highlight/true), published unknown, accessed 2026-11-02, snippet
- Observed appearance: Audiobook covers are portrait, and a details page shows the cover, title, author, narrator, length, a green play control, and a chapter list. In Now Playing the controls include a 'Playback speed 1x' button and, for audiobooks and podcasts, a moon icon for the sleep timer.
- Observed behavior: The sleep timer offers 5, 10, 15, 30, 45 min, 1 hour or end of track/episode, and shows the remaining time next to the Sleep Timer entry in the three-dot menu. Speed is a single button that cycles or opens a chooser. Chapters, bookmarks and notes: unverified.
- Evidence quality: inferred
- Conflicts / confidence: Sleep timer: high (newsroom 2022 and 2025 agree). Speed button: medium (support snippet only, wording not read). Chapters/bookmarks/summary: low, no source. The R1 chapter/bookmark/summary pages are R1-only extras, so they use the closest pattern (queue-style sheet).
- User requirement overrides: none recorded
- R1 adaptation: Audiobook details sheet: 480 px wide, #202124, a 160x160 cover at the top, title in font_ui_24, author in font_ui_16 #b3b3b3, a full-width 48 px green 'Start' or 'Resume' pill and a row of icon buttons (chapters, bookmarks), each >=48x48. In the audiobook Now Playing keep the Spotify layout but replace the native popovers: the speed button opens a shared bottom sheet with rows 0.75x, 1x, 1.25x, 1.5x, 2x (56 px each, check on the current one), and the sleep timer is a sheet with the exact option list above. Chapters and Bookmarks open as shared bottom sheets (a row per chapter with the time on the right in #b3b3b3; swipe-left deletes bookmarks). Summary is a scrollable sheet in font_ui_16. Mark chapters/bookmarks 'none (R1-only)'.
- Acceptance criteria:
  - The audiobook player ellipsis, speed button, Chapters, Bookmarks and Summary all open Spotify-style sheets, with no legacy popover or page.
  - The speed sheet persists the chosen value across restart and applies it during playback.
  - The sleep timer exposes the 7 documented options and shows remaining time.
  - Start/Resume does not expand Now Playing (Home journey) and Resume restores the saved position.

### Search tab: field, filter chips, results, 'See all', and browse tiles

- Audit IDs: SRCH-TAB, SRCH-BROWSE, SRCH-SEEALL
- Research date 2026-11-02; context: iOS and Android. The 2018 redesign shows the listener's top genres plus the old Browse content before a query is typed. Result filters (Top, Artists, Songs, Playlists, Albums, Podcasts & Shows, Episodes, Profiles) rolled out from 21 May 2021. A Spotify support article says search also matches lyrics (3 or more words) and shows 'Lyrics match'. The exact current 'Browse all' tile look is not documented in anything I read, and the 'Files & folders' card has no counterpart (R1-only).
- Sources:
  - [Spotify adds filters to search results on the mobile app](https://routenote.com/blog/?p=85553), published 2021-05-21, accessed 2026-11-02, read
  - [Spotify streamlines mobile navigation with Premium update](https://rainnews.com/?p=27701), published 2018-10-18, accessed 2026-11-02, read
  - [Search (Spotify Support)](https://support.spotify.com/article/search/), published undated, accessed 2026-11-02, read
  - [Case study: Follow up to Spotify UI updates (search snippet only)](https://bootcamp.uxdesign.cc/case-study-follow-up-to-spotify-ui-updates-1aa018ed3e23), published unknown, accessed 2026-11-02, snippet
- Observed appearance: A search field sits at the top. With an empty query the page shows 'Your top genres' and 'Browse all' as two-column coloured category tiles with a title at the top left. After typing, filter chips appear below the field and results are rows (cover, title, type and artist).
- Observed behavior: Results update as you type and the filter chips narrow the result type. Selecting a song plays it, and selecting an album, artist or playlist opens its page. Swiping right on a track row queues it. The R1 'See all results' drill-down matches the 'see all' pattern but not a documented Spotify one.
- Evidence quality: documented
- Conflicts / confidence: High for filters (2021) and top-genres-first layout (2018). Low for the tile dimensions and colours. 'Files & folders' is R1-only. Legacy 'See all' that opens an unfinished native search page contradicts Spotify, where results are inline under the filter.
- User requirement overrides: none recorded
- R1 adaptation: Search header: a 48 px white-on-#282828 rounded field (font_ui_18) with a clear X, and 16 px gutters. Chips: All, Songs, Albums, Artists, Playlists, Lyrics, 40 px high, horizontally scrollable. Result rows are 64 px (48 px cover, title in font_ui_16 white, subtitle in font_ui_16 #b3b3b3, trailing '...' with a 44 px hit area). Browse tiles are 2 columns x 96 px, with 8 px radius and a distinct dark accent tint on #282828, label in font_ui_18 top left. 'See all' must expand the SAME inline results list (paged, 20 rows per page) and keep the field text and keyboard state, instead of opening the legacy page. 'Files & folders' becomes the last full-width browse tile and opens a Spotify-styled file list (Local Files pattern).
- Acceptance criteria:
  - 'See all results' never opens the legacy search page; it shows the paged inline list with the query preserved.
  - Typing filters with the debounce, with chips narrowing results; row hit areas are >=44 px.
  - Result '...' opens the shared bottom sheet (not a native popover); a swipe-right queues with a toast.
  - Browse tiles are keyboard-hidden and reachable when the query is empty.

### On-screen keyboard and search entry (Search field, Library search, Find in playlist, naming layers)

- Audit IDs: SRCH-KEYBOARD, LIB-SEARCH, COL-FIND, LIB-CREATE
- Research date 2026-11-02; context: System IME on iOS and Android, so there is no Spotify-drawn keyboard. Nothing I read describes the keyboard's own styling. The only evidence is that the search field sits at the top of the page. For playlist find: the 'Find in playlist' field and Sort button appear on a pull-down (support article on sort and filter, mobile section).
- Sources:
  - [Sort and filter (Spotify Support)](https://support.spotify.com/article/sort-and-filter/), published undated, accessed 2026-11-02, read
  - [Create playlists (Spotify Support)](https://support.spotify.com/my-ms/article/create-playlists/), published undated, accessed 2026-11-02, read
  - [7 Spotify gestures every user should know](https://www.igeeksblog.com/spotify-gestures-every-user-should-know/), published updated 2026-05-06, accessed 2026-11-02, read
- Observed appearance: The system keyboard slides over the lower part of the screen, the Now Playing bar and tab bar are covered, and the field keeps focus under the header. In playlists, a search field with a sort button appears at the top after a pull-down.
- Observed behavior: Typing filters live. The keyboard's search/Done key dismisses it and results stay. Naming a playlist uses a full-screen name field with a Create button.
- Evidence quality: inferred
- Conflicts / confidence: Medium for pull-down find (support article, plus a community note that it is inconsistent across versions). The keyboard look is not Spotify-specific, so the R1 keyboard is an adaptation.
- User requirement overrides: none recorded
- R1 adaptation: One shared keyboard (gui/shell/keyboard.c) for all fields: 310 px tall, #121212 keys background with #282828 keys, labels in font_ui_20, key height >=56 px, a green action key labelled Search/Done/Create depending on context. When it opens, hide the mini-player and nav (already done for Search) and keep the field visible; extend this to Library search, Find and naming layers. The action key must submit/dismiss AND keep results visible (the legacy search page requires the search key, so replace it). Field text in font_ui_18, caret #1ed760.
- Acceptance criteria:
  - Search, Library search, Find on this page, Add songs and playlist/folder naming all use the same keyboard component and action key.
  - The field is never covered, and the keyboard hides on Done, nav tap or sheet close with the query preserved.
  - The mini-player returns when the keyboard hides.

### Your Library: filter chips, sort, list/grid, pins, folders, Library search, Create sheet, and collection options

- Audit IDs: LIB-ROOT, LIB-SEARCH, LIB-CREATE, LIB-OPTIONS
- Research date 2026-11-02; context: iOS and Android. The 2021 redesign (press release 29 Apr 2021) added one search bar, dynamic filters, a sort, and pinning up to four items by swiping right. The sort and filter support article says mobile has a sort icon, list/grid view icons, pin by swipe right or press-and-hold, and that the app remembers choices. Folders can now be created on mobile through '+' then 'Folder' (support article; Android Authority reported the server-side rollout on 24 Apr 2026 for iOS and Android). Some accounts do not have it yet. Folders hold playlists only, no custom cover.
- Sources:
  - [Sort and filter (Spotify Support)](https://support.spotify.com/article/sort-and-filter/), published undated, accessed 2026-11-02, read
  - [Spotify Redesigns 'Your Library' App Tab](https://www.iphoneincanada.ca/news/spotify-your-library/), published 2021-04-29, accessed 2026-11-02, read
  - [Playlist folders (Spotify Support)](https://support.spotify.com/article/playlist-folders/), published undated, accessed 2026-11-02, read
  - [Spotify playlist folders mobile rollout](https://www.androidauthority.com/spotify-playlist-folders-mobile-rollout-3660742/), published 2026-04-24, accessed 2026-11-02, read
  - [Create playlists (Spotify Support)](https://support.spotify.com/my-ms/article/create-playlists/), published undated, accessed 2026-11-02, read
  - [New Your Library view (Spotify Community, search snippet only)](https://community.spotify.com/t5/Your-Library/New-Your-Library-view-iOS-amp-Android/m-p/5196161), published 2021, accessed 2026-11-02, snippet
- Observed appearance: A header with the profile picture, a title, a search magnifier and a '+' create button. Below it come filter chips (Playlists, Artists, Albums, Podcasts), then a sort row (sort label on the left, list/grid icon on the right), then rows or tiles with the type and creator as a subtitle. Pinned items are listed first. The Create sheet is a bottom sheet with 'Playlist' (and 'Folder' when rolled out).
- Observed behavior: Tapping a chip filters and the app remembers sort per filter (community post, app 8.6.64). A pin is toggled with a right swipe or the long-press menu. Sort options are Recents, Recently added, Alphabetical, Creator, and Custom order for playlists, set on desktop. Create then Playlist opens a name screen with a Create button, and Folder asks for a name. Playlist menu: Add to this playlist, Edit, Add to folder, Delete playlist (the support article does not say whether there is a confirmation).
- Evidence quality: documented
- Conflicts / confidence: High for chips, sort, grid/list, pins and folder creation. Medium for the 4-pin limit (2021 sources). Whether tapping an active chip clears it is not stated in what I read, which is plausible but unverified. Delete confirmation is not described, so keep R1's confirm sheet as a safe adaptation.
- User requirement overrides: none recorded
- R1 adaptation: Header 56 px: title in font_ui_24 left, magnifier and '+' icon buttons 48x48 right. Chips 40 px pills (Playlists, Albums, Artists, plus All); sort row 44 px with the sort label in font_ui_16 and a List/Grid toggle 48x48. List rows 64 px with a 48 px cover (round for artists), grid = 2 columns of 208 px tiles. Pinned rows get a green pin glyph. Keep Previous/More paging as a final 56 px row but prefer lazy page loading (50 rows). Sort sheet = shared bottom sheet with a check on the active option and the five options, persisted per filter. Create sheet rows 'Create playlist' and 'Create playlist folder', then the shared naming layer with a green 'Create' key. Delete uses a confirm sheet with a red 'Delete' and 'Cancel'. Folders contain only playlists. Never delete audio files.
- Acceptance criteria:
  - Sort choice persists per filter and across restart, and the check mark shows the active sort.
  - List/Grid toggle and pins survive restart, and at most 4 pins are enforced or the limit is documented.
  - Create playlist and Create folder both finish with the item visible and selected, with no orphaned keyboard.
  - Delete playlist shows the confirm sheet, removes only the playlist record, and leaves audio files on disk.
  - All Library overlays are bottom sheets, not native popovers.

### Collection pages: album, playlist, Liked Songs, All songs/Most played, header play and shuffle, find/sort pull-down, track '...' menu, multi-select and edit

- Audit IDs: COL-ALBUM, COL-PLAYLIST, COL-LIKED, COL-ALLSONGS, COL-NAMES, COL-TRACK-MENU, COL-SELECT, COL-FIND, COL-PLAYBACK, COL-EDIT, PL-LEGACY
- Research date 2026-11-02; context: iOS and Android. Separate Play and Shuffle buttons for Premium: announced 1 Aug 2022, a community post updated 2023-11-12 says they are available on mobile and desktop. Free users cannot play in order on mobile. Pull-down on a playlist shows filters, a search field and a sort control (support article). Edit (three dots, then Edit) offers a remove icon per track and a drag handle (reorder is Premium only), per the create-playlists support article. Adding songs: 'Add to this playlist' or the track menu 'Add to Playlist', then Done. Swipe right on a track queues it, and swipe left likes it (iGeeksBlog 2026-05-06). The Plus becomes a green check after saving, and tapping it changes the destination (newsroom, 27 Feb 2023). Unknowns: the exact track-menu item list in the current app.
- Sources:
  - [Spotify's new button is an alternative to pressing play](https://www.howtogeek.com/822506/spotifys-new-button-is-an-alternative-to-pressing-play/), published 2022-08-01, accessed 2026-11-02, read
  - [Sort and filter (Spotify Support)](https://support.spotify.com/article/sort-and-filter/), published undated, accessed 2026-11-02, read
  - [Create playlists (Spotify Support)](https://support.spotify.com/my-ms/article/create-playlists/), published undated, accessed 2026-11-02, read
  - [7 Spotify gestures every user should know](https://www.igeeksblog.com/spotify-gestures-every-user-should-know/), published updated 2026-05-06, accessed 2026-11-02, read
  - [Spotify newsroom: save to Liked Songs (Plus becomes green check)](https://newsroom.spotify.com/tag/your-liked-songs/), published 2023-02-27, accessed 2026-11-02, read
  - [Local Files (Spotify Support)](https://support.spotify.com/article/local-files/), published undated, accessed 2026-11-02, read
  - [Spotify separate play and shuffle buttons (HTTP 410)](https://www.techhive.com/article/824961/spotify-separate-play-shuffle-buttons.html), published 2022-08, accessed 2026-11-02, snippet
- Observed appearance: A cover, title, creator line and meta line (for example 'Playlist · N songs'), then an action row with a save/download/ellipsis group at the left and a shuffle toggle plus a large green round play button at the right. Track rows show the title, an artist subtitle and a trailing '...'. The pull-down reveals a find field and a sort control.
- Observed behavior: Play starts the collection in order and Shuffle (a toggle) shuffles it. Tapping a row plays from it. '...' opens a bottom sheet of track actions. Edit mode shows remove and drag handles. In sorting inside a playlist the app remembers the choice. Multi-select is not documented for Spotify mobile and is an R1 extension, so use Edit.
- Evidence quality: documented
- Conflicts / confidence: High for the header controls, pull-down and Edit/reorder. Medium for the track menu content (community-sourced). Spotify's 'Liked Songs' header has no ellipsis in some versions, which is unverified. The legacy PL-LEGACY pick/browse page and COL-SELECT corner buttons have no counterpart and are R1-only.
- User requirement overrides: none recorded
- R1 adaptation: One collection scaffold: a 56 px back bar, a 160x160 cover centred or left, title in font_ui_24, creator in font_ui_16 #b3b3b3, meta in font_ui_16. Action row 56 px: shuffle toggle (green when on) and a 56x56 green play disc. Track rows 64 px, with a trailing '...' of at least 48x48 that opens the shared bottom sheet (Add to Liked, Add to queue, Add to playlist, Remove from this playlist, Go to album, Details). Replace the native popover, the Details page and the legacy playlist chooser with sheets: 'Add to playlist' uses the NP-ADD-PLAYLIST sheet; Details is a scrollable sheet. Replace COL-SELECT corner buttons with the Edit mode (Spotify pattern): 'Edit' in the sheet turns rows into remove icons and drag grips, with a 'Done' action, and a confirm sheet for removal. Artist-album and genre/year name lists reuse the Library list rows and the same collection page. Pull-down find: field plus sort button cycling Original/Title/Artist/Album/Added, with a Done key on the keyboard. Cap rows to a window of about 12 widgets using row recycling.
- Acceptance criteria:
  - Album, playlist, Liked, All songs, Most played and genre/artist lists use one scaffold, and none opens a legacy tracks panel.
  - Track '...' opens the shared sheet with the listed items, and Details opens a Spotify-styled sheet or page with no legacy topbar.
  - Edit mode replaces the legacy selection corner buttons, with remove (confirm) and drag-reorder that persists after restart.
  - Shuffle toggle changes the next Play only, and Play starts the first row in order when Shuffle is off.
  - Pull-down find filters with the debounce and the sort button cycles through 5 modes without changing the stored playlist order.

### Artist page and Discography (filters, See all, release tap)

- Audit IDs: ART-DISCO
- Research date 2026-11-02; context: iOS and Android. A Spotify staff reply (via community search snippet) says 'See discography' under Popular releases offers Albums and Singles and EPs filters. Spotify's provider support page confirms the classification: Albums, Singles and EPs and Compilations hold product-level main-artist or remixer credits, and Appears On holds track-level credits only. Community reports (May 2025) say 'Appears On' lists may be capped near 50. 'Featured On' vs 'Appears On' naming is unresolved, with the source using 'Appears On'.
- Sources:
  - [My music isn't showing under the Albums, Singles and EP, Compilations section](https://providersupport.spotify.com/article/my-music-isnt-showing-under-the-albums-singles-and-ep-compilations-section), published undated, accessed 2026-11-02, read
  - [How to see the entirety of the Appears on section (Spotify Community, snippet only)](https://community.spotify.com/t5/Desktop-Windows/How-to-see-the-entirety-of-the-quot-Appears-on-quot-section-from/m-p/5226540/highlight/true), published unknown, accessed 2026-11-02, snippet
  - [7 Spotify gestures every user should know (tapping the artist name opens the artist page)](https://www.igeeksblog.com/spotify-gestures-every-user-should-know/), published updated 2026-05-06, accessed 2026-11-02, read
- Observed appearance: The artist page has a hero image, a follow button and a green play control, followed by Popular tracks and a Discography/Releases section. 'See discography' opens a list with filter chips (All, Albums, Singles and EPs, and so on).
- Observed behavior: A chip filters the release list, and tapping a release opens its album page. Following is a button on the artist header (the R1 'Follow/Unfollow' in LIB-OPTIONS).
- Evidence quality: documented
- Conflicts / confidence: Classification is high (official provider page). Chip labels and the hero look are medium (staff snippet, not read). 'Featured On' vs 'Appears On' differs, and I recommend 'Appears On'.
- User requirement overrides: none recorded
- R1 adaptation: Page = shared scaffold with a 56 px back bar, the artist name in font_ui_28 over a #121212 gradient (no live blur), a Follow pill (40 px, outlined) and a green play disc. Chips: All, Albums, Singles & EPs, Appears On (rename Featured On), 40 px high. Release rows 72 px (56 px cover, title in font_ui_16, a '2019 · Album' subtitle in #b3b3b3), paged 20 at a time with 'Show more' as a final row. Mount the footer (nav plus mini-player) and keep the chip state when returning from an album.
- Acceptance criteria:
  - Chips filter by release type with counts that match the library data, and the last chip persists when returning from an album.
  - A release tap opens the shared album page, and back returns to the same scroll offset.
  - Footer tab taps from the discography switch tabs without leaving a stale page.

### Now Playing (reference layout 3): artwork, title/artist taps, add-to-playlist plus/check, lyrics card and full lyrics, Queue and the '...' menu, artist chooser

- Audit IDs: NP-REF, NP-OPTIONS, NP-ARTISTS, NP-ADD-PLAYLIST, NP-LYRICS, NP-LEGACY-LAYOUTS, NP-DETAILS
- Research date 2026-11-02; context: iOS and Android. Lyrics: the support article says tap the Now Playing bar, scroll to Lyrics, tap the lyrics for the full-screen view; Lyrics preview under the artwork is on by default and can be switched off from the three-dot menu (three horizontal dots on iOS, vertical on Android) then Lyrics. The Plus becomes a green check, tapping it changes the destination (newsroom 27 Feb 2023, iOS and Android). Tapping the title opens the album and tapping the artist opens the artist page (iGeeksBlog, May 2026). Sleep Timer is at the bottom of the three-dot menu for music (newsroom 2025-08-29). A May 2025 report (snippet only, Android Authority returned 403) says the Queue was redesigned under a three-line icon with shuffle, smart shuffle, repeat and sleep timer. 'Add to playlist' has a 'Saved in' group and a search bar in some versions (community snippets). Experiments: a Now Playing test replaced queue with share (Neowin, 403, undated).
- Sources:
  - [View lyrics (Spotify Support)](https://support.spotify.com/us/article/view-lyrics/), published undated, accessed 2026-11-02, read
  - [Spotify newsroom: Plus becomes green check](https://newsroom.spotify.com/tag/your-liked-songs/), published 2023-02-27, accessed 2026-11-02, read
  - [Spotify newsroom: sleep timer](https://newsroom.spotify.com/tag/sleep), published 2025-08-29, accessed 2026-11-02, read
  - [7 Spotify gestures every user should know](https://www.igeeksblog.com/spotify-gestures-every-user-should-know/), published updated 2026-05-06, accessed 2026-11-02, read
  - [Spotify Connect (Spotify Support)](https://support.spotify.com/article/spotify-connect), published undated, accessed 2026-11-02, read
  - [Spotify queue redesign (HTTP 403, snippet only)](https://www.androidauthority.com/p-3597405), published 2025-05, accessed 2026-11-02, snippet
  - [New update with 'saved in' (Spotify Community, snippet only)](https://community.spotify.com/t5/iOS-iPhone-iPad/New-update-with-saved-in-ruined-making-playlists/m-p/6933500/highlight/true), published unknown, accessed 2026-11-02, snippet
- Observed appearance: A large square artwork, with the title (white, bold) and artist (grey) below it. A Plus/green-check button sits at the right of the title and a three-dot menu at the top right with a down chevron at the top left. Under the artwork are a progress slider, then shuffle, previous, a large play disc, next and repeat. The bottom row has a device (Connect) icon and queue/share icons. Lyrics appear as a card below with a Lyrics preview.
- Observed behavior: Swipe down closes the player. Title opens the album and artist opens the artist page. Plus saves to Liked Songs and a green check opens the destination sheet. Lyrics card tap opens full-screen lyrics where tapping a line highlights it for sharing (seek-on-tap is an R1 extension). The three-dot menu holds Add to playlist, Add to queue, Go to album/artist, Sleep Timer, Show credits. R1 'Details' maps to credits.
- Evidence quality: documented
- Conflicts / confidence: High for lyrics, Plus/check and tap targets. Medium for the three-dot menu contents (partial documentation) and the queue placement (a 2025 snippet and an experiment disagree with R1's lower-right Queue). The legacy layouts (Standard/Waveform/Studio) have no Spotify counterpart.
- User requirement overrides: none recorded
- R1 adaptation: Keep layout 3 at 480x800: 56 px top bar (chevron-down left, ellipsis 48x48 right), artwork 384x384 centred, title in font_ui_24 with the Plus/check button (48x48) on the right, artist in font_ui_18 #b3b3b3. Progress slider 8 px high with a 44 px touch strip. Transport: play disc 72 px, others 56 px. Lyrics card 3 lines in font_ui_18 on #282828 (8 px radius). All menus go through the shared bottom sheet (#202124, rows 56 px): the '...' menu = Add to playlist, Add to queue, Go to queue, View album, View artists, Lyrics preview On/Off, Full lyrics, Sleep timer (new, from the documented path), Track information. Show 'Saved in' and 'Playlists' groups with a search field, '+ New playlist' and a green Done pill in the Add-to-playlist sheet. Mixed items (Details, the queue) must use the same list rows and not the legacy trackmenu popover. Because Settings > Music > Player layout can pick Standard/Waveform/Studio/Sonix, either restyle those layouts' '...' to the shared sheet or hide the picker from the Spotify shell and record it as 'none (R1-only)'.
- Acceptance criteria:
  - Every Now Playing overlay ('...', artists, add to playlist, lyrics fallbacks, details) is the shared bottom sheet, with no legacy popover or page.
  - The Plus/check reflects Liked/playlist membership, and the sheet shows 'Saved in' correctly after toggling and restart.
  - Artwork, title, artist, lyrics card and Queue hit areas are >=44 px.
  - Switching Player layout never leaves a non-Spotify ellipsis menu reachable from the Spotify shell.

### Queue sheet (Now playing, Next in queue, Next from, history, edit, clear, reshuffle)

- Audit IDs: NP-QUEUE
- Research date 2026-11-02; context: iOS and Android. A search snippet from Spotify's community FAQ and Business Insider says the queue has 'Next in queue' and 'Next from' sections plus an Edit button at the top right with 'Move up' at the bottom. A May 2025 report (403, snippet only) says shuffle, smart shuffle, repeat and sleep timer moved into the queue. iGeeksBlog (May 2026) documents swipe left to remove and long-press then drag to reorder (Premium). I could not read the FAQ page (title only returned).
- Sources:
  - [7 Spotify gestures every user should know](https://www.igeeksblog.com/spotify-gestures-every-user-should-know/), published updated 2026-05-06, accessed 2026-11-02, read
  - [How does the Play Queue work? (Spotify Community, title only returned)](https://community.spotify.com/t5/FAQs/How-does-the-Play-Queue-work/ta-p/1308142/redirect_from_archived_page/true), published unknown, accessed 2026-11-02, snippet
  - [Queue (Spotify Support): 404](https://support.spotify.com/article/queue/), published n/a, accessed 2026-11-02, snippet
  - [Spotify queue redesign (403, snippet only)](https://www.androidauthority.com/p-3597405), published 2025-05, accessed 2026-11-02, snippet
- Observed appearance: A full-height sheet titled Queue with a 'Now playing' row, 'Next in queue' for manually added tracks, and 'Next from: [context]' below. Rows carry a drag handle at the right in edit mode.
- Observed behavior: Tap a row to play it, swipe left to remove, an Edit mode to select rows, 'Move up' and drag to reorder, and a 'Clear queue' control.
- Evidence quality: community report
- Conflicts / confidence: Medium for the section names (consistent across community and one article). Low for current control placement (the 2025 redesign could not be read). R1's history rows and 'Reshuffle' are extras, and the nearest Spotify pattern is Smart Shuffle.
- User requirement overrides: none recorded
- R1 adaptation: Full-height shared sheet (top 80 px gap, #202124). Header row 'Queue' in font_ui_20 with 'Edit' (48x48 text button) on the right. Sections in font_ui_16 #b3b3b3: Now playing, Next in queue, Next from <context>, and History collapsed above. Rows 64 px with a 48 px cover and a 48x48 grip in edit mode; use a bounded window (about 12 live rows) for long queues. Keep 'Clear queued' and 'Reshuffle' as two 44 px pills in the footer. Swipe-left removal and Edit select/move-up/drag use identical visuals to COL-EDIT, and keep history drag-to-queue as 'none (R1-only)'.
- Acceptance criteria:
  - Queue ordering semantics match the stored queue after play, remove, move-up and clear, and survive Now Playing collapse.
  - The 4 sections are labelled as above, with 'Next from' showing the real context name.
  - Edit mode uses the same selection and drag UI as the collection Edit mode.
  - A 1000-track context does not create more than about 12-20 live row widgets.

### Shared bottom sheets, toasts/snackbars, and confirm dialogs

- Audit IDs: SHEET-COMPONENT, LIB-OPTIONS, HOME-OPTIONS, COL-TRACK-MENU
- Research date 2026-11-02; context: iOS and Android. Spotify context menus are bottom sheets opened by the '...' or a long press. Long-press support varies by area (a community staff reply says some long presses are not expected to open a menu). A toast such as 'Added to Liked Songs' shows at the bottom for about 5 seconds (community reports; no official text, and the Change action is described only for the destination sheet). The delete-playlist support text does not describe a confirmation. I found no official Spotify spec of sheet radius or scrim.
- Sources:
  - [Spotify newsroom: green check changes destination](https://newsroom.spotify.com/tag/your-liked-songs/), published 2023-02-27, accessed 2026-11-02, read
  - [7 Spotify gestures every user should know](https://www.igeeksblog.com/spotify-gestures-every-user-should-know/), published updated 2026-05-06, accessed 2026-11-02, read
  - [Create playlists (Spotify Support)](https://support.spotify.com/my-ms/article/create-playlists/), published undated, accessed 2026-11-02, read
  - [Added-to message blocking Done button (Spotify Community, snippet only)](https://community.spotify.com/t5/Live-Ideas/Mobile-Prevent-fix-Added-to-message-blocking-Done-button-UI/idi-p/7460074), published unknown, accessed 2026-11-02, snippet
- Observed appearance: A dark sheet with rounded top corners rises from the bottom over a dimmed page. A handle bar sits at the top and a header (cover, title, subtitle) heads a list of icon-plus-label rows. Toasts are a small dark bar near the bottom.
- Observed behavior: Swipe down or tap outside dismisses it, choosing an action closes the sheet, and a toast confirms. Nested choices (Add to playlist) replace the content with a new sheet.
- Evidence quality: inferred
- Conflicts / confidence: Medium. Visual details are not sourced, and toast text and timing are community-reported. The toast can hide the Done button, so on R1 it must not cover primary buttons.
- User requirement overrides: none recorded
- R1 adaptation: Bottom sheet: #202124 on a #000 scrim at 60 percent, 16 px top radius, 40x4 handle, optional header row 72 px (48 px cover, font_ui_18 title, font_ui_16 #b3b3b3 subtitle), rows 56 px (24 px icon, font_ui_18 label), destructive rows in a red tone, max height 80 percent of 800 px with an internal scroll, and a 220 ms slide-up (no real-time blur). Dismiss by handle swipe, scrim tap or hardware Back. Toast: #282828 pill, font_ui_16, 2.5 s, 72 px above the mini-player, never above a sheet's primary button. Confirm sheet: title in font_ui_20, body in font_ui_16, a full-width red or green 48 px pill and a text 'Cancel'. Route every popover (trackmenu, tools, legacy confirmations) through these components.
- Acceptance criteria:
  - Every overlay reachable from the Spotify shell is this component, and the audit finds no theme_style popover.
  - Hardware Back and scrim tap dismiss with no state change.
  - The toast never covers a sheet button, and Delete/Remove always show a confirm sheet.
  - Rows are >=56 px high with >=44 px hit areas.

### Local Files / microSD browser and storage (Files & folders)

- Audit IDs: SRCH-FILES, COVERFLOW-LEGACY
- Research date 2026-11-02; context: iOS and Android. 'Local Files' appears in Your Library after enabling 'Local audio files' under Settings and privacy, Apps and devices (support article, read). The article does not describe the screen. Folder browsing and Cover Flow have no Spotify counterpart (R1-only).
- Sources:
  - [Local Files (Spotify Support)](https://support.spotify.com/article/local-files/), published undated, accessed 2026-11-02, read
- Observed appearance: Local Files is one entry in Your Library that opens a normal track-list collection page, not a file tree. The page itself is not described by Spotify.
- Observed behavior: Local tracks play like normal ones, and crossfade only works between local files with the same sample rate and channel count.
- Evidence quality: documented
- Conflicts / confidence: High that it is a library entry, low for appearance. The folder tree and microSD path view are R1-only. Cover Flow has no Spotify equivalent.
- User requirement overrides: none recorded
- R1 adaptation: Present 'Files & folders' as a Library-style list: 56 px back bar with the current path in font_ui_16 #b3b3b3, folder rows 64 px with a folder glyph and a chevron, and audio rows with a 48 px cover or a note glyph and a trailing '...' that opens the shared sheet. Tapping a track starts playback WITHOUT forcing Now Playing, and the mini-player appears. Hide the legacy file-browser chrome. Cover Flow: either rebuild as a Spotify-styled sheet or record it as an R1-only gap with a reachable fallback in the shell, but never leave the legacy music grid reachable.
- Acceptance criteria:
  - The Files & folders browser uses the shared header, rows and sheet, with no legacy topbar.
  - Playing from a file does not force Now Playing expansion, and the mini-player is mounted.
  - Cover Flow and the legacy Music grid are unreachable or restyled, and any exception is listed in docs/UI_AUDIT.md.

### Device picker / Connect and Sleep timer (R1-only counterpart: Bluetooth output and Settings sheets)

- Audit IDs: SHELL-TOOLS, SHELL-SETTINGS, NP-OPTIONS
- Research date 2026-11-02; context: iOS and Android. Connect: tap the device icon in the Now Playing bar and choose a device (support article, read). The three-dot menu next to a device offers 'Log out of this device'. On iOS a volume slider appears at the bottom or in the device sheet. The sleep timer is in the three-dot menu for music and a moon icon for podcasts (newsroom 2022 and 2025). R1 has no Spotify Connect, so the Bluetooth device list and sleep timer are R1-only uses of the same patterns.
- Sources:
  - [Spotify Connect (Spotify Support)](https://support.spotify.com/article/spotify-connect), published undated, accessed 2026-11-02, read
  - [How to make Spotify's sleep timer part of any bedtime routine](https://newsroom.spotify.com/2022-08-02/how-to-make-spotifys-sleep-timer-part-of-any-bedtime-routine/), published 2022-08-02, accessed 2026-11-02, read
  - [Spotify newsroom: sleep timer](https://newsroom.spotify.com/tag/sleep), published 2025-08-29, accessed 2026-11-02, read
- Observed appearance: The device picker is a bottom sheet listing 'This device' and the others, with the active device in green and a volume slider. The sleep timer is a sheet with a list of durations.
- Observed behavior: Selecting a device switches output immediately. The sleep timer applies right away and shows the remaining time on its menu item.
- Evidence quality: documented
- Conflicts / confidence: High for both patterns. Whether the picker is a sheet (versus full screen) is medium. Applying them to Bluetooth output and R1 settings is an adaptation.
- User requirement overrides: none recorded
- R1 adaptation: Bluetooth/Wireless: a bottom-sheet or list page of paired and nearby devices (rows 64 px, font_ui_18 name, state subtitle in font_ui_16, the active device name in #1ed760), a scan action as a 44 px pill and a 'Forget' row in the '...' sheet. A volume slider row (8 px track, 48 px touch). Sleep timer: shared sheet with 5, 10, 15, 30, 45 min, 1 hour and End of track, an 'Off' row when active, and the remaining time in the menu row.
- Acceptance criteria:
  - Bluetooth connect/disconnect/forget work through sheet rows, and the active device is green.
  - The sleep timer sheet offers the 7 options, stops playback at expiry and shows remaining time, with no legacy popover.
  - Volume changes do not interrupt playback.

Failed attempts:

- support.spotify.com/article/queue/ returned 'Page not found'; no official queue page was read.
- support.spotify.com/uk/article/audiobooks/ returned a 404 page; no official audiobook page (chapters, bookmarks, speed) was read.
- androidauthority.com/p-3597405 (May 2025 queue redesign) and neowin.net Now Playing test returned HTTP 403, so the queue redesign and full-art experiment are snippet-only.
- techhive.com separate play/shuffle article returned HTTP 410. The same facts were taken from howtogeek (2022-08-01) instead.
- All community.spotify.com fetches (play queue FAQ, how do I access settings, audiobook playback speed, new Library view) returned only the page title, so those are snippet-only. The Settings entry in the profile menu and the queue section names are therefore medium confidence.
- tomsguide.com equalizer how-to returned a truncated excerpt with no steps. Equalizer UI was taken from the Spotify support page instead.
- No 'All' or 'Audiobooks' Home chip was found in any source. Only Music and Podcasts & Shows (Aug 2022) are verified. The audiobook chip is recorded as unverified.
- No spotify.design page or dated YouTube video page was found or read. A spotify.design search returned third-party write-ups only, and the one YouTube-oriented search returned only community threads.
- No official source describes the current Search 'Browse all' tile geometry, the bottom-sheet radius/scrim, or toast timing. Those R1 values are design choices, not observed Spotify values.
- Not researched: Android versus iOS side-by-side screenshots with app version numbers. All platform notes are from article text, not dated screenshots.

