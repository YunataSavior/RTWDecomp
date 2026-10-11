// VC6 gives a variable declared in a for statement the scope of the enclosing block. The game redefines for to give
// it the standard scope, the switch way: nearly every loop in RomeTW.exe (10,257 of the 10,725 that start from zero)
// begins by storing 0 in a temporary it never reads, which is this switch's value. ("switch (0) default: for"
// compiles the same.)
#ifndef FOR_SCOPE_H
#define FOR_SCOPE_H

#define for switch (0) case 0: default: for

#endif
