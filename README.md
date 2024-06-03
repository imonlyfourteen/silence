## Python script to split WAV files by silence

Currently only 16-bit little-endian WAV files (RIFF) are supported.

### Dependencies

- Python 3.6+
- GCC
- pkgconf

### Usage example

Put `silence.py` and `silence.c` to a directory and run

```bash
$ chmod +x silence.py
$ ./silence.py --help   # or just use arguments to do the job
```

```
usage: silence.py [-h] [-t THRESH] [-d DUR] [-l LEN] [-o OUTPUT_DIR] [-s SIMULATE] file

Split 16-bit WAV (RIFF, little-endian) files by silent parts.

positional arguments:
  file                  WAV file

options:
  -h, --help            show this help message and exit
  -t THRESH, --thresh THRESH
                        silence threshold in dBfs (default: -35)
  -d DUR, --dur DUR     silence minimum duration in seconds (default: 0.2)
  -l LEN, --len LEN     maximum WAV segment length in seconds (default: 60)
  -o OUTPUT_DIR, --output_dir OUTPUT_DIR
                        WAV segment output directory (default: .)
  -s SIMULATE, --simulate SIMULATE
                        do not write WAV segments, only print log
```

When run first time `silence.so` Python module will be created in the script's directory:

```bash
$ ls
silence.c  silence.py  silence.so
```

Splitting a WAV file:

```bash
$ ./silence.py 1.wav -o parts/
Saving "parts/00000.wav", 47.255 s ... done.
Saving "parts/00001.wav", 42.818 s ... done.
Saving "parts/00002.wav", 42.014 s ... done.
Saving "parts/00003.wav", 57.083 s ... done.
Saving "parts/00004.wav", 56.709 s ... done.
Saving "parts/00005.wav", 57.963 s ... done.
Saving "parts/00006.wav", 48.490 s ... done.
Saving "parts/00007.wav", 11.699 s ... done.
```

Note, that resulting segments may be larger than `--len` if no silence was detected
inside a segment. In this case try to increase silence threshold or reduce minimum
silence duration, `-t` and `-d` arguments respectively.

To just check segment lengths without writing files, run the script with `-s` argument:

```bash
$ ./silence.py 1.wav -o parts/ -s
```

### Performance

I assume the script works pretty fast, though there were no optimizations made.
It takes about 3 seconds to split 1 hour of 2-channel 48kHz in ~60 second chunks,
including compilation of the `silence.so` module. Note that overwriting segments is
slower than creating new files so it is recommended to delete segments before next try.

Memory consumption mostly depends on the `--len` argument since whole segment of that
length is read at one time.

### Convert any audio to a 16-bit little-endian WAV file

```bash
$ ffmpeg -i my_audio.mp3 -c:a pcm_s16le -ac 1 -ar 44100 my_audio.wav
```
