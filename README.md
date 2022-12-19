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

A Makefile is provided that assumes a POSIX-like system, for example Linux, or maybe the MSYS subsystem on Windows.  The assembler used by the Makefile, laxasm, is a cross-assembler available here: https://github.com/SteveFosdick/laxasm.  The assembler syntax is intended to be compatible with both ADE+ and the Lancaster native assemblers for the BBC micro and the VSAVE and VLOAD programs should be able to assembled with one of those.

The vlist.c program is ANSI C and should compile on any system with an ANSI C compiler.

## How it Works

### How Variables are Stored

There are two types of variables in the 6502 implementation of BBC BASIC as found on the BBC Micro.  Resident integer variables are stored at fixed positions at the start of page four of memory, i.e. between addresses &0400 and &46B.  All other variables are stored on a heap in a number of linked list.  There is one linked list for each character a variable name is allowed to start with and the  head pointers for these lists are also stored in page four from &0480 to &4F5.  BASIC also maintains two values in zero page: LOMEM which is the start of the heap and VARTOP which is the end of the heap.  Finally, in the case of a string variable, the value stored in the linked list is actually a string control block which contains the starting address of the characters within the string as well as the current and alloctaed lengths.

More information on this is available in the books that examine how BASIC works, for example the Advanced BASIC ROM User Guide.

### Saving

Between the resident integer variables and the linked list head pointers is a floating point temporary area.  The VSAVE program starts by putting some values in this area.  First a copy of LOMEM from zero page, recording where the heap started at the point the variables were saved, then the length of the heap, i.e. VARTOP-LOMEM, then a fixed string BBCBASVR which identifies the file as a BBC BASIC variable save file.

The VSAVE program then uses OSGBPB to save most of page four, i.e. &0400 to &04F5 inclusive, then the heap itself.  No changes are made to any of the pointers during the save.

### Loading

The VLOAD program uses OSGBPB to load the saved version of page four back into memory.  This has the effect of restoring the resident integer variables and also the linked list head pointers, though the pointers are not usable yet.  It then checks the fixed string to make sure this is a BBC BASIC variable save file.  Assuming it is, it calculates the delta between the old and new LOMEM, i.e. between the value stored in the file and the value in zero page.  This delta will need to be added to every pointer within the heap.

Next, it uses OSGBPB to load the heap from the file into memory at the new LOMEM.

The final step is to correct all the pointers within the heap.  It does this by stepping through each of the head pointers in turn and, for each one, first correcting the head pointer by adding the delta worked out above, then correcting each next pointer and  following it, repeating until the end of the list is reached.

In the event the variable is a simple string, the value will be a string control block in which case the pointer to the characters of the string will be corrected by adding the delta.

In the event the variable is an array of strings the program needs to work out how many string control blocks are in the array, which it does by multiplying the dimensions of the array.  Once it has a total number of cells it steps through them all, adding the delta to the pointer in each string control block.

### Memory Usage

In order to be tube compatible, these programs avoid using areas often used for small programs such as cassette and RS423 buffers and these would not be available on the tube processor.  On the other hand the programs are specifically designed to be run when BASIC is the current language and so can use parts of the BASIC language workspace that are not expected to be in use at the time the program is run.

In the case of VSAVE, this is pages five and six, i.e. &0500-&0600.  BASIC uses page five for stacks associated with control structures, hence the restriction on not calling from within FOR..NEXT, REPEAT..UNTIL and GOSUB.  BASIC uses page six as a string workspace, hence not executing from a generated string via OSCLI.  Page seven, used for line input from within BASIC is not used as this would prevent these programs from being run from the command line.

In the case of VLOAD the sitution is slighly more complex as the program does not fit in two pages.  The solution is to also use the end of page four which is about to be overwritten anyway and the program is assembled so that loading page four from the file overwrites the beginning of the VLOAD program in memory but only a part that has already run and will not be needed again.
