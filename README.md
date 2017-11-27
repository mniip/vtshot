#VtShot

VtShot is a linux console recording tool. Input can be grabbed from a framebuffer or a VCSA device. Output can be in PNG, PPM or GIF format. Can also produce animations: animated GIFs and sequences of PPM or PNG files.

VtShot can record an animation from a screen until terminated by SIGINT (^C), or it can launch a shell, start recording, and finish recording when the shell exits.

VtShot does not require root privileges. Usually you only need to be in the `video` group to access the framebuffer, and in the `tty` group to access the VCSA. For more info see `stat` of the respective device nodes in `/dev/`.

#Usage
```
Usage:
    vtshot (<file> | -b) [-d <device>] [-f <fps>] [-bDdFghmPpqVv]

  -d | --device <file>    Set the input device (default: /dev/fb0 or /dev/tty0).
  -f | --fb               Capture input from a framebuffer device (default).
  -V | --vcsa             Capture input from a VCSA device.
  -m | --mmap             Use memory mapping (slower on some machines, faster on others).
  -b | --benchmark        Do not capture the image, provide timing information instead.

  -p | --png              Set the output format to PNG (default).
  -P | --ppm              Set the output format to PPM.
  -g | --gif              Set the output format to GIF.

  -s | --sequence         Record an animation, terminated by SIGINT (^C).
  -S | --shell            Launch $SHELL and record an animation, until the shell exits.
  -F | --fps <number>     Set the animation FPS (default: 24.0).

  -q | --quiet            Suppress error messages.
  -v | --verbose          Show more informational messages.
  -D | --debug            Show debug information.
  -h | --help             Show this help.

```
