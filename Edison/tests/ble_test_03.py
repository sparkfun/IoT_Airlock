import binascii
import struct
import time
from bluepy.bluepy.btle import UUID, Peripheral

# Message constants
MSG_LOCK = 0x10
MSG_UNLOCK = 0x11
MSG_STATE_REQ = 0x12

# Define read and write UUIDs
read_uuid = UUID(0x2221)
write_uuid = UUID(0x2222)

# Create a connection to the RFduino
p = Peripheral("F9:D8:C2:B9:77:E9", "random")

try:

    # Create handles for read and write characteristics
    w_ch = p.getCharacteristics(uuid=write_uuid)[0]
    r_ch = p.getCharacteristics(uuid=read_uuid)[0]

    # If we can, continuously send lock and unlock requests
    if ( r_ch.supportsRead() ):
        while 1:

            # Tell the Lockitron to lock
            msg = struct.pack('i', MSG_LOCK)
            print "Writing: " + str(msg)
            w_ch.write(msg)
            time.sleep(3)

            # Tell the Lockitron to unlock
            msg = struct.pack('i', MSG_UNLOCK)
            print "Writing: " + str(msg)
            w_ch.write(msg)
            time.sleep(3)

            # Get lock state
            msg = struct.pack('i', MSG_STATE_REQ)
            print "Writing: " + str(msg)
            w_ch.write(msg)
            time.sleep(0.5)
            if ( r_ch.supportsRead() ):
                val = binascii.b2a_hex(r_ch.read())
                print "Lock state: " + str(val)
            time.sleep(3)

finally:
    p.disconnect()

