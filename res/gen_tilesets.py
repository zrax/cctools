#!/usr/bin/env python3

import struct

def write_tileset(filename, tile_size, name, desc, base_gfx_file,
                  overlay_gfx_file, cc2_gfx_file = None):
    with open(base_gfx_file, 'rb') as src:
        base_data = src.read()
    with open(overlay_gfx_file, 'rb') as src:
        overlay_data = src.read()

    if cc2_gfx_file is not None:
        with open(cc2_gfx_file, 'rb') as src:
            cc2_data = src.read()
    else:
        cc2_data = b''

    tis = open(filename, 'wb')
    tis.write(b'CCTILE02')
    tis.write(struct.pack('I', len(name)))
    tis.write(bytes(name, 'utf-8'))
    tis.write(struct.pack('I', len(desc)))
    tis.write(bytes(desc, 'utf-8'))
    tis.write(struct.pack('B', tile_size))
    tis.write(struct.pack('I', len(base_data)))
    tis.write(base_data)
    tis.write(struct.pack('I', len(overlay_data)))
    tis.write(overlay_data)
    tis.write(struct.pack('I', len(cc2_data)))
    tis.write(cc2_data)
    tis.close()

# Generate default tilesets if called from the command line
if __name__ == '__main__':
    write_tileset('TW32.tis', 32, 'TileWorld/Editor 32x32',
                  'TileWorld 32x32 Editor Graphics',
                  'TW32_base.png', 'TW32_overlay.png')

    write_tileset('WEP.tis', 32, 'MSCC/Editor Color',
                  'Microsoft WEP 32x32 Editor Graphics',
                  'MSCC_base.png', 'MSCC_overlay.png')

    write_tileset('CC2.tis', 32, 'CC2/Editor',
                  "Chip's Challenge 2 32x32 Editor Graphics",
                  'CC2_base.png', 'CC2_overlay.png', 'CC2_all.png')
