DLLs in The Sims Online (and also SimCity 4 and The Sims 2) use a special
interface, based on Microsoft COM.

These DLLs each export exactly one function:
void * GZDllGetGZCOMDirector(void)

This function creates and sets up a C++ object, with variables and member
functions, and returns a pointer to that object. This is your standard
C++ v-table.

TSOSimulatorClientD.dll is the most important DLL in the game. It implements
the SimAntics virtual machine which executes all the objects in the game.
In our situation, we need to figure out everything it does, because we lack
any information regarding the SimAntics instruction set architecture.
A text dump of this DLL is not nearly enough to find this. The files in the
objectdata/globals folder are not nearly enough. The page on
simtech.sourceforge.net documenting all they know about SimAntics is not
nearly enough. We need to run this DLL in a disassembler and figure out the
meaning of every opcode used in every behavior script of the game.