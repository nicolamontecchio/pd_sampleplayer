#!/usr/bin/env python
# encoding: utf-8
import sys
import re
import os

# http://store.samplephonics.com/products/the-leeds-town-hall-organ
# N.B. this just parses the first set of samples for each preset, discards the second


def parse_preset(fstream_in):
    # line stream to tuples (pitch, filename, loop_start, loop_end)
    def clamp(p):
        return max(12, min(96, int(p)))
    last_pitch = -1
    for line in fstream_in:
        if line.startswith('<region>'):
            pitch      = clamp(re.search('lokey=(\d+) ', line).group(1))
            filename   = re.search('sample=([^ ]+) ', line).group(1)
            loop_start = re.search('loopstart=(\d+) ', line).group(1)
            loop_end   = re.search('loopend=(\d+) ', line).group(1)
            if pitch > last_pitch:
                yield (str(pitch), filename, loop_start, loop_end)
                last_pitch = pitch


if __name__ == '__main__':
    base_dir = sys.argv[1]
    presets = [f for f in os.listdir(base_dir) if f.endswith('.sfz')]
    for pf in presets:
        with open(os.path.join(base_dir, pf[:-4] + '.pdsampler'), 'w') as out:
            for l in parse_preset(open(os.path.join(base_dir, pf))):
                print >> out, ' '.join(list(l))
