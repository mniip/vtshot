#VtShot

Linux console recording tool. Input can be grabbed from a framebuffer or a VCSA. Output can be in either PNG or PPM format.

#Usage
```
    vtshot (-o <file> | -b) [-d <device>] [-bfDhmPpqVv]

  -o | --output <file>    Set the output file.
  -d | --device <device>  Set the input device (default: /dev/fb0 or /dev/tty0).
  -b | --benchmark        Do not capture the image, provide timing information instead.
  -m | --mmap             Use memory mapping (slower on some machines, faster on others).
  -p | --png              Set the output format to PNG (default).
  -P | --ppm              Set the output format to PPM.
  -f | --fb               Capture input from a framebuffer device.
  -V | --vcsa             Capture input from a VCSA device.
  -q | --quiet            Suppress error messages.
  -v | --verbose          Show more informational messages.
  -D | --debug            Show debug information.
  -h | --help             Show this help.

```
