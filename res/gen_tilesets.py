#!/usr/bin/env python3

import struct
import json
import sys

def write_tileset(filename, spec):
    with open(spec['base'], 'rb') as src:
        base_data = src.read()
    with open(spec['overlay'], 'rb') as src:
        overlay_data = src.read()

    if 'cc2' in spec:
        with open(spec['cc2'], 'rb') as src:
            cc2_data = src.read()
    else:
        cc2_data = b''

    tis = open(filename, 'wb')
    tis.write(b'CCTILE02')
    tis.write(struct.pack('I', len(spec['name'])))
    tis.write(bytes(spec['name'], 'utf-8'))
    tis.write(struct.pack('I', len(spec['desc'])))
    tis.write(bytes(spec['desc'], 'utf-8'))
    tis.write(struct.pack('B', spec['size']))
    tis.write(struct.pack('I', len(base_data)))
    tis.write(base_data)
    tis.write(struct.pack('I', len(overlay_data)))
    tis.write(overlay_data)
    tis.write(struct.pack('I', len(cc2_data)))
    tis.write(cc2_data)
    tis.close()

# Generate default tilesets if called from the command line
if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: {} spec.json [...]'.format(sys.argv[0]))
        sys.exit(1)

    for arg in sys.argv[1:]:
        with open(arg, 'r') as spec_file:
            spec = json.load(spec_file)
        tis_filename = arg.rsplit('.', 1)[0] + '.tis'
        write_tileset(tis_filename, spec)
