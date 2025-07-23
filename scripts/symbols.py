#!/usr/bin/env python3

import os, sys

from elftools.elf.elffile import ELFFile

symbols = []

with open(sys.argv[1], 'rb') as f:
    elf = ELFFile(f)
    dynsym = elf.get_section_by_name('.dynsym')

    if not dynsym:
        raise RuntimeError("No .dynsym section")

    for symbol in dynsym.iter_symbols():
        if symbol['st_info']['bind'] == 'STB_GLOBAL' and symbol['st_shndx'] != 'SHN_UNDEF':
            symbols.append(symbol.name)

symbols.sort()
symbols.remove('mod_init')
symbols.remove('mod_preinit')

print('LIBRARY ' + os.path.basename(sys.argv[1]))
print('EXPORTS')
for sym in symbols:
    print('  ' + sym)
