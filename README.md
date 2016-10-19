# xr25_diag
Diagnostic software for Renault XR25-compatible cars; this software emerged due
to the need of diagnosing an old Renault car. Existing software either not
worked or were not free, so I reversed the former and wrote xr25_diag.

Compiling
---------
These additional packages are required, assuming you already have a working GNU C++ toolchain:
- gtkmm-3.0
- cairomm

To compile the software, change the cwd to xr25_diag/ and run:
    $ make
To enable debug code, add DEBUG=1:
    $ make DEBUG=1

About parsers
-------------
The meaning of frame octets change depending on the ECU; the following are
available:
- Fenix1parser: not tested
- Fenix3parser: valid for Renault 19, some Renault 21 and probably R25
- Fenix52Bparser: use with Renault 21 2.0 TXi

More information
----------------
See doc/other_documentation.pdf.