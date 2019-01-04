#!/usr/bin/env python3

import struct

def write_tileset(filename, tile_size, name, desc, base_gfx_file, overlay_gfx_file):
    src = open(base_gfx_file, 'rb')
    base_data = src.read()
    src.close()
    src = open(overlay_gfx_file, 'rb')
    overlay_data = src.read()
    src.close()

    tis = open(filename, 'wb')
    tis.write(b'CCTILE01')
    tis.write(struct.pack('I', len(name)))
    tis.write(bytes(name, 'utf-8'))
    tis.write(struct.pack('I', len(desc)))
    tis.write(bytes(desc, 'utf-8'))
    tis.write(struct.pack('B', tile_size))
    tis.write(struct.pack('I', len(base_data)))
    tis.write(base_data)
    tis.write(struct.pack('I', len(overlay_data)))
    tis.write(overlay_data)
    tis.close()

# Generate default tilesets if called from the command line
if __name__ == '__main__':
    write_tileset('TW32.tis', 32, 'TileWorld/Editor 32x32',
                  'Default 32x32 TileWorld Editor Graphics',
                  'TW32_base.png', 'TW32_overlay.png')

    write_tileset('WEP.tis', 32, 'MSCC/Editor Color',
                  'Microsoft WEP Default 32x32 Editor Graphics',
                  'MSCC_base.png', 'MSCC_overlay.png')

    write_tileset('CC2.tis', 32, 'CC2/Editor',
                  "Default 32x32 Chip's Challenge 2 Editor Graphics",
                  'CC2_base.png', 'CC2_overlay.png')
