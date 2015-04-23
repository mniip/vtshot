#VtShot

Linux console recording tool. So far can only pull data from framebuffer devices, and output portable pixmaps (PPM).

#Usage
```
    vtshot (-o <file> | -b) [-d <device>] [-bDhmPpqv]

  -o <file>   --output <file>    Set the output file.
  -d <device> --device <device>  Set the input device (default: /dev/fb0).
  -b          --benchmark        Do nog capture the image, provide timing information instead
  -m          --mmap             Use memory mapping (slower on some machines, faster on others)
  -p          --png              Set the output format to PNG (default)
  -P          --ppm              Set the output format to PPM
  -q          --quiet            Suppress error messages.
  -v          --verbose          Show more informational messages.
  -D          --debug            Show debug information.
  -h          --help             Show this help.

```
