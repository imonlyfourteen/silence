#!/usr/bin/python

import sys, os, argparse, wave

# Compile and import the module
mpath = os.path.join(os.path.dirname(__file__), 'silence')
if not os.path.exists(mpath + '.so'):
    comm = f'gcc -O2 -shared -fPIC `pkg-config --cflags python3` "{mpath}.c" -lm -lpython3 -o "{mpath}.so"'
    ret = os.system(comm)
    if ret != 0:
        raise Exception('Could not compile the "silence.so" module.')
        
from silence import detect

# Parse arguments
parser = argparse.ArgumentParser(
    description="""
        Split 16-bit WAV (RIFF, little-endian) files by silent parts.
    """
)
add = parser.add_argument
add('file', type=str, help='WAV file')
add('-t', '--thresh', type=float, help='silence threshold in dBfs (default: -35)', default=-35)
add('-d', '--dur', type=float, help='silence minimum duration in seconds (default: 0.2)', default=0.2)
add('-l', '--len', type=float, help='maximum WAV segment length in seconds (default: 60)', default=60)
add('-o', '--output_dir', type=str, help='WAV segment output directory (default: .)', default='.')
add('-s', '--simulate', help='do not write WAV segments, only print log', action='store_true')
args = parser.parse_args()

# Create output directory if not exists
if not args.simulate:
    if not os.path.exists(args.output_dir):
        os.mkdir(args.output_dir, 0o755)
    elif not os.path.isdir(args.output_dir):
        exit(f'Output directory path "{args.output_dir}" is not a directory.')

# Process the file
wav = wave.open(args.file, 'rb')
fr = wav.getframerate()
wav_len = wav.getnframes() / fr

def get_split_times():
    ssegs = detect(args.file, args.thresh, args.dur)
    if not ssegs:
        exit('No silence segments found.')
    mids = [sum(i)/2 for i in ssegs]
    return [0] + mids + [wav_len]

segment_i = 0
def wav_extract(start, end):
    global segment_i
    new_file_name = os.path.join(args.output_dir, f'{segment_i:05}.wav')
    segment_i += 1
    print(f'Saving "{new_file_name}", {end - start:.3f} s ... ', end='', flush=True)
    if not args.simulate:
        wav.setpos(int(start * fr))
        seg = wav.readframes(int((end - start) * fr))
        new_wav = wave.open(new_file_name, 'wb')
        new_wav.setparams(wav.getparams())
        new_wav.writeframes(seg)
        new_wav.close()
    print('done.')

def main():
    tsplit = get_split_times()
    last_i = 0
    for i in range(1, len(tsplit)):
        if tsplit[i] - tsplit[last_i] > args.len:
            if last_i != i-1:
                wav_extract(tsplit[last_i], tsplit[i-1])
                last_i = i-1
            if tsplit[i] - tsplit[i-1] > args.len:
                wav_extract(tsplit[i-1], tsplit[i])
                last_i = i
    if last_i != len(tsplit) - 1:
        wav_extract(tsplit[last_i], tsplit[-1])

main()