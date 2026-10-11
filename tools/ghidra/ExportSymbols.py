# Export the program's functions and named data as decomp-toolkit symbols.txt lines (config/<version>/symbols.txt).
# Functions Ghidra named itself (FUN_...) are written as fn_XXXXXXXX and data not named by hand is skipped. Names are written as
# they are in Ghidra, so name a function as the compiler would (_name for C, the mangled name for C++, _name@N for
# __stdcall) for it to match its compiled counterpart.
# @category RTWDecomp
# @runtime Jython

from ghidra.program.model.symbol import SourceType

program = currentProgram
memory = program.getMemory()
listing = program.getListing()
out = askFile("symbols.txt to write", "Export")


def section(address):
    block = memory.getBlock(address)
    if block is None:
        return None
    name = block.getName()
    if name == ".data" and not block.isInitialized():
        return ".bss"
    return name


rows = []
for function in program.getFunctionManager().getFunctions(True):
    if function.isThunk() or function.isExternal():
        continue
    entry = function.getEntryPoint()
    name = function.getName() if function.getSymbol().getSource() != SourceType.DEFAULT \
        else "fn_%08X" % entry.getOffset()
    size = function.getBody().getMaxAddress().subtract(entry) + 1
    rows.append((entry.getOffset(), "%s = %s:0x%08X; // type:function size:0x%X" % (
        name, section(entry), entry.getOffset(), size)))

for symbol in program.getSymbolTable().getAllSymbols(True):
    address = symbol.getAddress()
    # (only names given by hand: import slots get their __imp_ names from split.py)
    if not address.isMemoryAddress() or symbol.getSource() != SourceType.USER_DEFINED:
        continue
    if listing.getFunctionAt(address) is not None or section(address) in (None, ".text", ".rsrc"):
        continue
    data = listing.getDataAt(address)
    size = " size:0x%X" % data.getLength() if data is not None else ""
    rows.append((address.getOffset(), "%s = %s:0x%08X; // type:object%s" % (
        symbol.getName(), section(address), address.getOffset(), size)))

rows.sort()
handle = open(out.getAbsolutePath(), "w")
for _, line in rows:
    handle.write(line + "\n")
handle.close()
print("wrote %d symbols to %s" % (len(rows), out.getAbsolutePath()))
