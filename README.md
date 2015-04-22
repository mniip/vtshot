#VtShot

Linux console recording tool. So far can only pull data from framebuffer devices, and output portable pixmaps (PPM).

#Usage
```
Usage:
    vtshot -o <file> [-d <device>] [-qvDh]

  -o <file>   --output <file>    Set the output file.
  -d <device> --device <device>  Set the input device (default: /dev/fb0).
  -q          --quiet            Suppress error messages.
  -v          --verbose          Show more informational messages.
  -D          --debug            Show debug information.
  -h          --help             Show this help.
```
