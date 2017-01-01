#!/usr/bin/python2
import numpy as np
import matplotlib.pyplot as plt
import sys

# 60 Hz, 5s window
buf = [0]*60*5
plt.axis([0, len(buf), 0, 15])
plt.ion()

while True:
    plt.cla()
    # update 6 times a second
    for i in range(10):
        buf = buf[1:] + [int(sys.stdin.readline())]
    plt.plot(range(len(buf)), buf)
    plt.pause(0.0001)
