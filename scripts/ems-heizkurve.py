#!/usr/bin/python
# -*- coding: utf-8 -*-
import contextlib
import errno
import os
import io
import subprocess
import sys
import time

if len(sys.argv) > 1:
  if os.path.exists('/emsincludes'+sys.argv[1]):
    sys.path.append('/emsincludes'+sys.argv[1])
else:
  sys.path.append('/emsincludes/')
import config

@contextlib.contextmanager

def do_plot():

    filename = "hk.png"
    targetpath = config.graphtargetpath
    print (targetpath)
    process = subprocess.Popen("gnuplot", shell = False, stdin = subprocess.PIPE)
    print (process)
    stdin_wrapper = io.TextIOWrapper(process.stdin, 'utf-8')
    stdin_wrapper.write("set terminal png font 'arial' 12 size 800, 450\n")
    stdin_wrapper.write("set grid lc rgb '#aaaaaa' lt 1 lw 0.5\n")
    stdin_wrapper.write("set title 'Heizkurve'\n")
    stdin_wrapper.write("set xrange [-10:20]\n")
    stdin_wrapper.write("set yrange [5:45]\n")
    stdin_wrapper.write("set xlabel 'Aussentemperatur [°C]'\n")
    stdin_wrapper.write("set ylabel 'Vorlauftemperatur [°C]'\n")
    stdin_wrapper.write("set xtics 1\n")
    stdin_wrapper.write("set ytics 2\n")
    stdin_wrapper.write("set output '%s'\n" % os.path.join(targetpath, filename))
    stdin_wrapper.write("plot")
    stdin_wrapper.write(" '/tmp/hkt.dat' using 1:2 with lines smooth bezier lw 3 title 'Tag', " )
    if os.path.exists("/tmp/hkn.dat"):
        stdin_wrapper.write(" '/tmp/hkn.dat' using 1:2 with lines smooth bezier lw 3 lt 3 title 'Nacht', " )
    stdin_wrapper.write(" '/tmp/hka.dat' using 1:2 with lines smooth bezier lw 2 lt -1 title 'aktuell'\n" )
    stdin_wrapper.close()
    process.wait()

# main starts here

do_plot();

