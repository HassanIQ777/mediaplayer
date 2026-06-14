# MediaPlayer

MediaPlayer is a terminal media browser for quickly finding and playing local
audio and video files with `mpv`.

It recursively scans a directory, shows the media files in a keyboard-driven
terminal UI, supports search and sorting, tracks recently played media, and has
extra integrations for Termux/Android.

## Features

- Recursively scans a folder for audio and video files.
- Plays selected media through `mpv`.
- Keyboard navigation with arrow keys and Vim-like shortcuts.
- Search by filename or path.
- Toggle chronological and alphabetical sorting.
- Tracks play history and most-played/latest media.
- Configurable number of visible rows.
- Clipboard support for copying selected file URIs on desktop.
- Termux support for Android sharing, toast notifications, and MPV activity
  launching.

## Supported File Types

Video:

```text
.mp4 .mkv .mov .avi .wmv .flv .webm .mpeg
```

Audio:

```text
.mp3 .wav .flac .aac .ogg .m4a .wma .aiff
```

## Requirements

- A C++20 compiler, such as `g++`
- `make`
- `mpv`

Optional desktop clipboard support requires one of:

- `wl-copy` from `wl-clipboard`
- `xclip`
- `xsel`

Optional Android/Termux features use:

- `termux-share`
- `termux-toast`
- Android MPV package/activity: `is.xyz.mpv/.MPVActivity`

## Build

From the project root:

```sh
make
```

This builds the `mediaplayer` executable in the project directory.

For a debug build with sanitizers:

```sh
make debug
```

To remove build artifacts:

```sh
make clean
```

## Usage

Run MediaPlayer with the directory you want to browse:

```sh
./mediaplayer /path/to/media
```

Show help:

```sh
./mediaplayer --help
```

Show the version:

```sh
./mediaplayer -v
```

You can also use the Makefile runner:

```sh
make run ARGS="/path/to/media"
```

## Controls

| Key | Action |
| --- | --- |
| `Up` / `k` | Move selection up |
| `Down` / `m` | Move selection down |
| `Left` / `j` | Page up |
| `Right` / `l` | Page down |
| `Enter` / `Space` / `/` | Play selected media |
| `s` | Search |
| `r` | Reset current filter |
| `R` | Rescan files |
| `f` | Toggle chronological/alphabetical sort |
| `.` | Show latest/most-played media list |
| `,` | Open settings |
| `S` | Copy selected file URI on desktop, share file on Termux |
| `h` | Show in-app help |
| `q` | Quit |

## App Data

MediaPlayer creates a `mediaplayer` directory inside the scanned folder:

```text
/path/to/media/mediaplayer/
```

It stores:

- `exception-list` - paths to exclude while scanning.
- `history` - playback history.
- `latest-media` - play counts used by the latest media view.
- `settings.json` - user settings, currently including the visible row count.

The `exception-list` file is read as a plain text list and is used to skip paths
during recursive scans.

## Settings

Open settings with `,` inside the app.

Current setting:

- `columns` - the maximum number of media entries shown at once.

The setting is stored in:

```text
/path/to/media/mediaplayer/settings.json
```

## License

This project is licensed under the GNU General Public License v3.0. See
[`LICENSE`](LICENSE) for details.
