#!/usr/bin/env python
# encoding: utf-8
# upsampling lowpass filter design

import numpy as np
from scipy.signal import *
import matplotlib.pyplot as plt


Wn = 20 / 88.2


N = 8

b, a = butter(N, Wn, analog=False)

w, h = freqz(b, a)
clf()
plt.semilogx(w, 20 * np.log10(abs(h)))
margins(0, 0.1)


print 'numerator:'
print ','.join([str(f) for f in b])
print 'denominator:'
print ','.join([str(f) for f in a])
