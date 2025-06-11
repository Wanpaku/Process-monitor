========================
About Process monitor application
========================


Description
===========

This program created showing active processes and used memory.

Copyright (C) 2025  Teg Miles.
Process monitor is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

Features
========

  * Show active processes.
  * Show total used memory.

Authors
========

  - Teg Miles (movarocks2@gmail.com)
  - The logo icon was taken from https://www.flaticon.com/free-icon/content-management-system_2630878?term=system+monitor&page=1&position=1&origin=search&related_id=2630878 Eucalyp - Flaticon.

Requirements
============

* Qt6 (6.6.1 or newer), CMake (3.16 or newer), C++ compiler (GCC, Clang or MSVC).

One of the following operating systems:

* **Microsoft Windows**: 64-bit Windows 11 or higher
* **Linux**: x86_64 with kernel 6.14.0 or higher.  *Manjaro 23.0.0 (or newer) recommended.*


Configuration
=============

Configuration for the application is stored in the ``Process_monitor.conf`` file or registry item
in a directory appropriate to your OS.  Refer to this table:

========== ==============================================
System     Directory
========== ==============================================
Linux, BSD ``$XDG_HOME/`` if defined, else ``~/.config/``
Windows    ``\HKEY_CURRENT_USER\Software\Process_monitor``
========== ==============================================
