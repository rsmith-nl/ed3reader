Reading EBI 40 temperature logger files
#######################################

:date: 2026-03-15
:tags: C, reverse engineering, temperature logger
:author: Roland Smith

.. Last modified: 2026-03-15T11:41:01+0100
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
