# CPvM

CPvM is a compatibility layer that allows many CP/M 2.2 applications to run
natively on [SymbOS](http://symbos.de), the multitasking graphical OS for 8-bit
computers such as the Amstrad CPC and MSX2.

CPvM is a work in progress, but can already run many common CP/M applications,
such as Zork and Microsoft BASIC. CPvM targets VT-52 terminal emulation, but
enough ADM-3A and ANSI/VT-100 codes are also supported that many programs for
those terminals (Kaypro software, etc.) will also run without modification.

CPvM includes code by Prevtenet and Prodatron.

## Running

CPvM can be launched from within SymbOS SymShell either by specifying the
pathname of the CP/M program (and its parameters) directly, e.g.,

	cpvm cbasic.com b:\hello.bas

or by running it without any parameters, which launches an interactive CP/M
command-line session:

	cpvm

## Building (Windows-only)

* `tpa.asm` contains the non-relocatable BDOS and BIOS entry points. Assemble
  this using the assembler built into [WinApe](http://www.winape.net/), then
  run the Python script `make_bin.py` to convert the resulting `tpa.bin` into
  the needed format for the C build process.
* The main build uses [SDCC](https://sdcc.sourceforge.net/) and Nerlaska's
  SymbosMake SDK. The latter is now unavailable online; a partially updated
  version is included in the `sdk` directory (patched to support SDCC 4.1.12+,
  which uses a default calling convention incompatible with the original design
  of the SDK).
* With these prerequisites, navigate to the `cpm` folder and run `make.bat`.
