#!/usr/bin/python2
import numpy as np
import matplotlib.pyplot as plt
import sys

buf = [0]*300
plt.axis([0, len(buf), 0, 16])
plt.ion()

while True:
    plt.cla()
    for i in range(16):
        buf = buf[1:] + [int(sys.stdin.readline())]
    plt.plot(range(len(buf)), buf)
    plt.pause(0.0001)
