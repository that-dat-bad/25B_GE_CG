import zlib
import struct

def make_png(width, height, color):
    # PNG signature
    png_header = b'\x89PNG\r\n\x1a\n'
    
    # IHDR chunk
    # width (4 bytes), height (4 bytes), bit depth (1 byte), color type (1 byte, 2=RGB),
    # compression method (1 byte), filter method (1 byte), interlace method (1 byte)
    ihdr_data = struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0)
    def make_chunk(type, data):
        return struct.pack('>I', len(data)) + type + data + struct.pack('>I', zlib.crc32(type + data) & 0xffffffff)
    
    ihdr = make_chunk(b'IHDR', ihdr_data)
    
    # IDAT chunk
    # Each row starts with a filter type byte (0 = None)
    raw_data = b''
    for y in range(height):
        raw_data += b'\x00' # Filter type
        for x in range(width):
            raw_data += bytes(color)
            
    idat = make_chunk(b'IDAT', zlib.compress(raw_data))
    
    # IEND chunk
    iend = make_chunk(b'IEND', b'')
    
    return png_header + ihdr + idat + iend

# Create 1x1 white pixel
white_png = make_png(1, 1, (255, 255, 255))

with open('c:/forClass/25B/25B_GE_CG/Resources/white1x1.png', 'wb') as f:
    f.write(white_png)

# Create 1x1 red pixel for lock-on etc
red_png = make_png(1, 1, (255, 0, 0))
with open('c:/forClass/25B/25B_GE_CG/Resources/lockOn.png', 'wb') as f:
    f.write(red_png)
