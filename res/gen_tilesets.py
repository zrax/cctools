#!/usr/bin/env python

import struct

name = 'TileWorld/Editor 32x32'
desc = 'Default 32x32 TileWorld Editor Graphics'

src = open('TW32_base.png', 'rb')
base_data = src.read()
src.close()
src = open('TW32_overlay.png', 'rb')
overlay_data = src.read()
src.close()

tis = open('TW32.tis', 'wb')
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


name = 'MSCC/Editor Color'
desc = 'Microsoft WEP Default 32x32 Editor Graphics'

src = open('MSCC_base.png', 'rb')
base_data = src.read()
src.close()
src = open('MSCC_overlay.png', 'rb')
overlay_data = src.read()
src.close()

tis = open('WEP.tis', 'wb')
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
