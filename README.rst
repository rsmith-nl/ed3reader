Reading EBI 40 temperature logger files
#######################################

:date: 2026-03-15
:tags: C, reverse engineering, temperature logger
:author: Roland Smith

.. Last modified: 2026-03-16T18:05:17+0100
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

    ed3reader-c version: 2026.03.14
    usage: ed3reader [-h] [-v] [-l] [--log=(debug|info|warn|error|crit)] infile [outfile]

    Program for converting ed3 files from an EBI 40 temperature logger to plain text.

    positional argument:
    infile  -- input file in ed3 format.
    outfile -- output file name. If this is not given output goes to stdout.

    options:
    -h, --help            show this help message and exit
    -v, --version         show program's version number and exit
    -l, --license         print the license
    --log                 logging level debug,info,(default) warn,error,crit


So normal usage is::

   ed3reader Custom00.ed3 output.txt

The output looks like this::

    # Channel count: 2
    # Data count: 42 samples
    # Temperature unit: °C
    # Bits per sample: 16
    # Comma shift 1 positions to the left.
    # Measurement interval 1 minutes.
    # Start date: 2025-05-19T15:06:03
    # ISO8601 datetime  ch1  ch2
    2025-05-19T15:06:03 23.5 31.1
    2025-05-19T15:07:03 63.7 31.5
    2025-05-19T15:08:03 67.2 33.2
    2025-05-19T15:09:03 69.2 35.9
    2025-05-19T15:10:03 72.8 39.1
    2025-05-19T15:11:03 73.8 42.4
    ...

Lines starting with ``#`` are comments.
The first column contains the datetime of the sample is ISO 8601 format.
This is followed by as many columns as there are active channels.

The author would normally use ``gnuplot`` to create a graph from this data.
But you could also import the data into a spreadsheet program.
