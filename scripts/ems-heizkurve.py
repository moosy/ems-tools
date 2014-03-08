#!/usr/bin/python
# -*- coding: utf-8 -*-
import contextlib
import errno
import os
import subprocess
import sys
import time
sys.path.append('/emsincludes/')
import config

@contextlib.contextmanager

def do_plot():

    filename = "hk.png"
    targetpath = config.graphtargetpath

    process = subprocess.Popen("gnuplot", shell = False, stdin = subprocess.PIPE)
    process.stdin.write("set terminal png font 'arial' 12 size 800, 450\n")
    process.stdin.write("set grid lc rgb '#aaaaaa' lt 1 lw 0,5\n")
    process.stdin.write("set title 'Heizkurve'\n")
    process.stdin.write("set xrange [-10:20]\n")
    process.stdin.write("set yrange [5:45]\n")
    process.stdin.write("set xlabel 'Aussentemperatur [°C]'\n")
    process.stdin.write("set ylabel 'Vorlauftemperatur [°C]'\n")
    process.stdin.write("set xtics 1\n")
    process.stdin.write("set ytics 2\n")
    process.stdin.write("set output '%s'\n" % os.path.join(targetpath, filename))
    process.stdin.write("plot")
    process.stdin.write(" '/tmp/hkt.dat' using 1:2 with lines smooth bezier lw 3 title 'Tag', " )
    if os.path.exists("/tmp/hkn.dat"):
        process.stdin.write(" '/tmp/hkn.dat' using 1:2 with lines smooth bezier lw 3 lt 3 title 'Nacht', " )
    process.stdin.write(" '/tmp/hka.dat' using 1:2 with lines smooth bezier lw 2 lt -1 title 'aktuell'\n" )
    process.stdin.close()
    process.wait()


# main starts here

do_plot();

