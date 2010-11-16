#!/usr/bin/env python

import struct

def write_tileset(filename, name, desc, base_gfx_file, overlay_gfx_file):
    src = open(base_gfx_file, 'rb')
    base_data = src.read()
    src.close()
    src = open(overlay_gfx_file, 'rb')
    overlay_data = src.read()
    src.close()

    tis = open(filename, 'wb')
    tis.write('CCTILE01')
    tis.write(struct.pack('I', len(name)))
    tis.write(name)
    tis.write(struct.pack('I', len(desc)))
    tis.write(desc)
    tis.write(struct.pack('B', 32))
    tis.write(struct.pack('I', len(base_data)))
    tis.write(base_data)
    tis.write(struct.pack('I', len(overlay_data)))
    tis.write(overlay_data)
    tis.close()

# Generate default tilesets if called from the command line
if __name__ == '__main__':
    write_tileset('TW32.tis', 'TileWorld/Editor 32x32',
                'Default 32x32 TileWorld Editor Graphics',
                'TW32_base.png', 'TW32_overlay.png')

    write_tileset('WEP.tis', 'MSCC/Editor Color',
                'Microsoft WEP Default 32x32 Editor Graphics',
                'MSCC_base.png', 'MSCC_overlay.png')
