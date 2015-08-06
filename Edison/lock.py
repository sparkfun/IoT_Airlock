import binascii
import struct
import time
from bluepy.btle import UUID, Peripheral

# Message constants
MSG_LOCK = 0x10
MSG_UNLOCK = 0x11
MSG_STATE_REQ = 0x12

# Define read and write UUIDs
read_uuid = UUID(0x2221)
write_uuid = UUID(0x2222)

# Create a connection to the RFduino
#p = Peripheral("F9:D8:C2:B9:77:E9", "random")
p = Peripheral("D4:2C:92:60:C2:D5", "random")


try:

    # Create handles for read and write characteristics
    w_ch = p.getCharacteristics(uuid=write_uuid)[0]
    r_ch = p.getCharacteristics(uuid=read_uuid)[0]

    # Tell the Lockitron to lock
    msg = struct.pack('i', MSG_LOCK)
    print "Writing: " + str(msg)
    w_ch.write(msg)

finally:
    p.disconnect()

