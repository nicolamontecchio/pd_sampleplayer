#!/usr/bin/env python
# encoding: utf-8
import sys
import re
import os

# http://store.samplephonics.com/products/the-leeds-town-hall-organ
# N.B. this just parses the first set of samples for each preset, discards the second


def parse_preset(fstream_in, bsae_dir):
    # line stream to tuples (pitch, filename, loop_start, loop_end)
    def clamp(p):
        return max(12, min(96, int(p)))
    last_pitch = -1
    for line in fstream_in:
        if line.startswith('<region>'):
            pitch      = clamp(re.search('lokey=(\d+) ', line).group(1))
            filename_  = re.search('sample=([^ ]+) ', line).group(1)
            filename   = re.search('K_([0-9]+)_Pre([0-9])_[^_]+_RR([0-9])\.wav', filename_)
            loop_start = re.search('loopstart=(\d+) ', line).group(1)
            loop_end   = re.search('loopend=(\d+) ', line).group(1)
            if filename and pitch > last_pitch:
                yield ('addsample', str(pitch), os.path.join(base_dir, filename.group(0)), loop_start, loop_end)
                last_pitch = pitch


if __name__ == '__main__':
    base_dir = sys.argv[1]
    presets = sorted([f for f in os.listdir(base_dir) if f.endswith('.sfz')])
    for pf in presets[1:]:  # first preset is missing a sample on central C (!)
        with open(os.path.join(base_dir, pf[:-4] + '.pdsampler'), 'w') as out:
            for l in parse_preset(open(os.path.join(base_dir, pf)), base_dir):
                print >> out, ' '.join(list(l))
