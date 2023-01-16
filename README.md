# megawin-isp
Command line ISP utility for flashing Megawin MG82F6D17 controller

Currently tested on linux (Fedora 36) & WIN 10 with MG82F6D17 only

TODO:
 - [ ] Code cleanup
 - [ ] Support for RTS/DTR based reset control

> <PRE>NOTE: MG82F6D17 has only HWBS programmed, which causes
>                 ISP code to boot only on power cycling.
>                 To jump to ISP code on reset pin program
>                 HWBS2 fuse (USING the ICP programmer)</PRE>

You will need mingw/cygwin windows compilation suite to build 
on windows platform.

Pre-compiled binaries available on binaries folder (Win10 & Linux)
