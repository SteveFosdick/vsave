# BBC Micro BASIC Variable Save and Load
Ths project provides three programs.  To run on the BBC Micro itself are programs to save and then re-load the complete set of BASIC variables to a file.  To run on a PC or other system with an ANSI C compiler is a program to list all the variables saved in the file.
## Usage

To save the variables from the current BASIC program:
`*VSAVE <file>`

To load a set of variable into the current BASIC program replacing all those already there:
`*VLOAD <file>`

In each case where `<file>` is the name of the file on the current filing system to save/load the variables to.

On a PC or other system on which vlist.c has been compiled, to see all the variables contained within a file: `vlist <file> [...]`

## Restrictions

The VSAVE and VLOAD programs are tube-compatible.  Because of the area of memory used they must not be invoked by constructing the command dynamically with the BASIC OSCLI keyword, or from within a FOR..NEXT loop, a REPEAT..UNTIL loop, a subroutine called with GOSUB or a PROC or FN.

## Building

A Makefile is provided that assumes a POSIX-like system, for example Linux, or maybe the MSYS subsystem on Windows.  The assembler used by the Makefile, laxasm, is a cross-assembler available here: https://github.com/SteveFosdick/laxasm  The assembler syntax is intended to be compatible with both ADE+ and the Lancaster native assemblers for the BBC micro and the VSAVE and VLOAD programs should be able to assembled with one of those.

The vlist.c program is ANSI C and should compile on any system with an ANSI C compiler.
