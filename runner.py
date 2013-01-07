#!/usr/bin/env python

### This file tests the various counters ###

import matplotlib
matplotlib.use('Agg')

import platform
import subprocess
import sys
import pylab as plt

COUNTERS = ('spinlock', 'cas', 'bitonic:4', 'bitonic:8', 'bitonic:16')
THREADS = (1, 2, 4, 8, 16, 32, 64)

def run_configuration(counter, nthreads):
  '''returns the total throughput at this configuration'''
  args = ['./network', '--counter', counter, '--num-threads', str(nthreads)]
  p = subprocess.Popen(args, stdin=open('/dev/null', 'r'), stdout=subprocess.PIPE)
  r = p.stdout.read()
  p.wait()
  return float(r.strip())

if __name__ == '__main__':
  (_, outfile) = sys.argv
  for counter in COUNTERS:
    values = [run_configuration(counter, n) for n in THREADS]
    print counter, values
    plt.plot(THREADS, values)
  plt.xlabel('num threads')
  plt.ylabel('ops/sec')
  plt.title(platform.node())
  plt.legend(COUNTERS)
  plt.savefig(outfile)
