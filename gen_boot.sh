#!/usr/bin/env python3
# Embeds src/boot.ppd into src/boot_embedded.h as a C string array.
# Run this before 'mpd build ppd' whenever boot.ppd changes.
import sys, os

src  = os.path.join(os.path.dirname(__file__), 'src', 'boot.ppd')
dest = os.path.join(os.path.dirname(__file__), 'src', 'boot_embedded.h')

with open(src, 'r') as f:
    lines = f.read().splitlines()

with open(dest, 'w') as f:
    f.write('// Auto-generated from src/boot.ppd — do not edit manually.\n')
    f.write('// Re-generate with: ./gen_boot.sh\n')
    f.write('#ifndef BOOT_EMBEDDED_H\n')
    f.write('#define BOOT_EMBEDDED_H\n')
    f.write('#include <stdint.h>\n')
    f.write('static const char _ppd_boot_embedded[] =\n')
    for line in lines:
        escaped = line.replace('\\', '\\\\').replace('"', '\\"')
        f.write(f'  "{escaped}\\n"\n')
    f.write('  "";\n')
    f.write('static const uint32_t _ppd_boot_embedded_len =\n')
    f.write('  sizeof(_ppd_boot_embedded) - 1;\n')
    f.write('#endif // BOOT_EMBEDDED_H\n')

print(f'generated src/boot_embedded.h ({len(lines)} lines)')
