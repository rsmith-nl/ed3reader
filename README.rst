Reading EBI 40 temperature logger files
#######################################

:date: 2026-03-15
:tags: C, reverse engineering, temperature logger
:author: Roland Smith

.. Last modified: 2026-05-10T13:29:55+0200
.. vim:spelllang=en

At work we use an EBI 40 6-channel temperature logger.
This outputs files with an ``ed3`` extension.
The software that reads those files is only available for ms-windows.

So I set out to build a reader.

.. PELICAN_END_SUMMARY

Introduction
============

The first version of that reader was a quick & dirty Python script.
I want to make this program available for my colleagues, and the IT department
at work won't support Python scripts; the program has to be a single
executable.

So I set out to rewrite it in C, in order to be able to compile it for POSIX
systems and ms-windows.


No AI policy
============

This code is by a human and for humans.
"AI" / LLM-generated rewrites and additions are not welcome.


Building the software
=====================

POSIX
-----

To build the software, just issue the ``make`` command.
The build has been tested with ``clang`` (19.1.7) and ``gcc`` (14.2.0).

ms-windows
----------

Pre-built binaries for ms-windows are available in the github releases.

The only *tested* build method on ms-windows is using w64devkit_.
Open ``w64devkit``, change to the directory where you have stored the source,
and issue the command ``make -f Makefile.win32``.

.. _w64devkit: https://github.com/skeeto/w64devkit


Installation
============

The program does not *require* installation to run. Only the binary is
required.

POSIX
-----

On a POSIX system, ``make install`` will install the program in ``~/.local/bin``.

For convenience you can add that location to your ``$PATH``, or you can
manually install it in any other location in your ``$PATH``.


Using ed3reader
===============

The ``ed3reader`` program is meant to be run from a terminal, e.g. the console
or an ``Xterm`` on POSIX systems and ``Windows Terminal`` on ms-windows.

Open a terminal and type in the following, ending with the ``<enter>`` key::

   ed3reader -h

This should print the following online help to the terminal::

    usage: ed3reader [-h] [-v] [-l] [-c] [--log=(debug|info|warn|error|crit)] infile [outfile]

    Program for converting ed3 files from an EBI 40 temperature logger to plain text.
    Information from the data file header will be written to stderr, unless
    the -c option is used. In that case it is added as comments to the output file.

    positional argument:
    infile  -- input file in ed3 format.
    outfile -- output file name. If this is not given output goes to stdout.

    options:
    -h, --help            show this help message and exit
    -v, --version         show program's version number and exit
    -l, --license         print the license
    -c, --csv             write output in CSV format
    --log                 logging level debug,info,(default) warn,error,crit


For ms-windows users, normal usage is::

   ed3reader -c Custom00.ed3 output.csv

The contents of ``output.csv`` look like this::

    ...
    Excel datevalue,ch1,ch2
    45796.629201,23.5,31.1
    45796.629896,63.7,31.5
    45796.630590,67.2,33.2
    45796.631285,69.2,35.9
    45796.631979,72.8,39.1
    45796.632674,73.8,42.4
    45796.633368,74.0,45.4
    45796.634063,75.8,48.4
    45796.634757,74.9,51.1
    45796.635451,74.8,53.6
    45796.636146,74.6,55.8
    ...

The first column contains date and time as an ms-excel datevalue, that is days
since 1900-1-1 (plus 2, because of excel bugs).
This is followed by as many columns as there are active channels.
The columns are separated by comma's.
This kind of output can easily be loaded into ms-excel.

The CSV output can also be loaded in other spreadsheets like ``gnumeric``.
Otherwise, on POSIX platforms the following use is generally recommended::

   ed3reader Custom00.ed3 output.d

This generates the following output, where the data/time in ISO 8601 format is
followed by the colums with temperature data, separated by horizontal tabs::

    # ISO8601 datetime      ch1     ch2
    2025-05-19T15:06:03     23.5    31.1
    2025-05-19T15:07:03     63.7    31.5
    2025-05-19T15:08:03     67.2    33.2
    2025-05-19T15:09:03     69.2    35.9
    2025-05-19T15:10:03     72.8    39.1
    2025-05-19T15:11:03     73.8    42.4
    2025-05-19T15:12:03     74.0    45.4
    2025-05-19T15:13:03     75.8    48.4
    2025-05-19T15:14:03     74.9    51.1
    2025-05-19T15:15:03     74.8    53.6
    2025-05-19T15:16:03     74.6    55.8

To use the output data with ``gnuplot``, the data for the x-axis needs to be
set to ``time``, and the ``timefmt`` should be set::

    set timefmt "%Y-%m-%dT%H:%M:%S"
    set xdata time

If showing the date on each label on the x-axis is not needed, also add this::

    set format x "%H:%M"

Plotting the actual data is done like this::

   plot 'ed3.txt' using 1:2 w l ls 1 title "channel 1", \
   '' using 1:3 w l ls 2 title "channel 2"
