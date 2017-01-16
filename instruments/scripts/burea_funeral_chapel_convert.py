#!/usr/bin/env python
# encoding: utf-8

# Convert the Burea Funeral Chapel sample set from
# http://www.familjenpalo.se/vpo/download

import sys
import re
import os
import glob
import subprocess
import random


SAMPLE_BEGIN = 22050
SAMPLE_END = 66150
XFADE_DUR = 11025

if __name__ == '__main__':

    _, dir_in = sys.argv
    dir_out = 'instruments/burea'
    os.mkdir(dir_out)

    registers = ['Gedackt8', 'PedGedackt8', 'Principal2', 'Rorflojt4',
                 'Salicional8', 'Subbas16', 'VoixCeleste8']
    p = re.compile('.*/(\d\d\d)-.*')

    with open(os.path.join(dir_out, 'instrument.txt'), 'w') as out:
        for ri, reg in enumerate(registers):
            reg_dir = os.path.join(dir_in, reg)
            print 'processing register %s in %s' % (reg, reg_dir)
            reg_files = sorted(glob.glob(reg_dir + '/*.wav'))
            for file_in in reg_files:
                midi_note = p.findall(file_in)[0]
                output_note = '%d%s' % (ri, midi_note)
                output_fname = os.path.join(dir_out, output_note + ".wav")
                s_b = SAMPLE_BEGIN + random.randint(-4096,4096)
                s_e = SAMPLE_END + random.randint(-4096,4096)
                proc = subprocess.Popen(['./looper', file_in, output_fname,
                                         str(s_b),
                                         str(s_e),
                                         str(XFADE_DUR)],
                                        stdout=subprocess.PIPE)
                proc.communicate()
                print >> out, '%s %s %d %d' % (output_note, output_fname, s_b + XFADE_DUR, s_e)
