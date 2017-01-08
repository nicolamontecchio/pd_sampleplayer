#!/usr/bin/env python
# encoding: utf-8

# Convert the Burea Funeral Chapel sample set from
# http://www.familjenpalo.se/vpo/download

import sys
import re
import os
import glob
import subprocess


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
                output_note = '%d%s.wav' % (ri, midi_note)
                output_fname = os.path.join(dir_out, output_note)
                proc = subprocess.Popen(['./looper', file_in, output_fname,
                                         '22050', '66150', '5512'],
                                        stdout=subprocess.PIPE)
                output, _ = proc.communicate()
                print >> out, '%s %s %s' % (output_fname, output_note, output.strip())
