# xr25_diag
Diagnostic software for Renault XR25-compatible cars

These additional packages are required, assuming you already have a working GNU C++ toolchain:
- gtkmm-3.0
- cairomm

To compile the software, change the cwd to xr25_diag/ and run:
$ make
To enable debug code, add DEBUG=1:
$ make DEBUG=1
