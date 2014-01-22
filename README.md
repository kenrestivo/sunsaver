# Sunsaver Linux Command line interface

This is a fork of [the Sunsaver interface software created by Tom Rinehart](http://westyd1982.wordpress.com/2010/03/26/linux-and-mac-os-x-software-to-read-data-from-the-sunsaver-mppt-using-modbus/)

Updated by ken restivo to add a Makefile and some other minor modifications.

# Installing

I had to use [libmodbus](http://www.libmodbus.org/) 2.0.3 because 3.02 suggested in the original blogpost did not compile properly on my system.

This fork requires [a forked version of libmodbus](https://github.com/kenrestivo/libmodbus) which supports half-duplex operation.

As the original author mentioned in his blog post, you'll also need a build system for linux or OSX.

I built and run libmodbus and this software successfully on a [Beaglebone](http://beagleboard.org/bone/) running Angstrom linux, and on Debian Linux on X86-64, i386, and ARMEL.

You'll also need a SunSaver MPPT and a MSC interface or equivalent.

Have fun!

# License

GPL Version 3.
