README
======

Content of this folder !
------------------------

This folder contains the code for a C++ software that communicate with
the OSVR Server API to get the orientation of a Head Mounted Display (HMD)
and extract the right portion of a 360-Degree video and render it to
the HMD.

The folders ``src'' and ``LibAvWrapper'' contains the C++ source code
of this software.
Some file are there to allow the generation of a Makefile using cmake.

The folder PythonInterface contains some python scripts to run a user
test or to perform some data process. This folder contains its own
README.

Requirements !!
---------------

To compile this software needs:

- the libav library (from ffmpeg). Becareful to install a recent enough
library because only the last API from the libav library is yet supported

- the osvr-core and osvr-rendermanager software/libraries from the OSVR
opensource project. https://github.com/OSVR/OSVR-Core
https://github.com/sensics/OSVR-RenderManager

- a recent version of GCC that support C++14 standard (I currently use
GCC version 6.3.1)


Compilation steps
-----------------

Create a build directory:  > mkdir build
Go inside this directory:  > cd build
Generate the Makefile:     > cmake ..
Compile the software:      > make

Tested platform:
----------------

All the codes and scripts were tested on linux: archlinux with
up-to-date packages.
In theory, the C++ software will work on windows with the windows
version of the OSVR API. We never tested it.

We used the Razer OSVR HDK2 Head Mounted Display. The sofware should
work as it is with other HMDs such as the Oculus Rift or the HTC Vive
if the osvr driver for those HMDs are installed.

We use a Nvidia GE 1060 graphic card.


Contacts:
---------
E-mail: xavier.corbillon[at]imt-atlantique.fr
Website: http://dash.ipv6.enstb.fr/headMovements
