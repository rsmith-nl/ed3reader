Reading EBI 40 temperature logger files
#######################################

:date: 2026-03-15
:tags: C, reverse engineering, temperature logger
:author: Roland Smith

.. Last modified: 2026-03-17T22:46:05+0100
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

“AI” generated contributions and rewrites are not welcome.


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

    usage: ed3reader [-h] [-v] [-l] [--log=(debug|info|warn|error|crit)] infile [outfile]

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
    -c, --comments        write header info as comments in the output file
    --log                 logging level debug,info,(default) warn,error,crit

So normal usage is::

   ed3reader Custom00.ed3 output.txt

The contents of ``output.txt`` look like this::

    ISO8601 datetime        excel datevalue ch1     ch2
    2025-05-19T15:06:03     45796.629201    23.5    31.1
    2025-05-19T15:07:03     45796.629896    63.7    31.5
    2025-05-19T15:08:03     45796.630590    67.2    33.2
    2025-05-19T15:09:03     45796.631285    69.2    35.9
    2025-05-19T15:10:03     45796.631979    72.8    39.1
    2025-05-19T15:11:03     45796.632674    73.8    42.4
    2025-05-19T15:12:03     45796.633368    74.0    45.4
    2025-05-19T15:13:03     45796.634063    75.8    48.4
    2025-05-19T15:14:03     45796.634757    74.9    51.1
    2025-05-19T15:15:03     45796.635451    74.8    53.6
    ...

The first column contains the datetime of the sample is ISO 8601 format.
The second colum is the same date and time as an ms-excel datevalue, that is days
since 1900-1-1 (plus 2, because of excel bugs).
This is followed by as many columns as there are active channels.
The columns are separated by horizontal tabs (ascii character 9).

The author would normally use ``gnuplot`` to create a graph from this data.
But you could also import the data into a spreadsheet program.


Using ed3reader output with gnuplot
===================================

To use the output data with ``gnuplot``, the data for the x-axis needs to be
set to ``time``, and the ``timefmt`` should be set::

    set timefmt "%Y-%m-%dT%H:%M:%S"
    set xdata time

If showing the year on each label on the x-axis is not needed, also add this::

    set format x "%m-%d\n%H:%M"

Plotting the actual data is done like this::

   plot 'ed3.txt' using 1:3 w l ls 1 title "channel 1", \
   '' using 1:4  every 4 w l ls 2 title "channel 2"


Using ed3reader output with ms-excel
====================================

When importing the file in excel, set the format to ``delimited``, and set the
horizontal tab as the delimiter character.
Mark the first column as not to be imported.
If necessary, change the input settings as using a decimal point, not a comma.

After the import, the fist column should be set to date value format.
