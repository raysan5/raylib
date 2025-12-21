#!/usr/bin/env python3
"""Generate a test DDS file with mipmaps to verify swizzling fix"""

import struct
import os

def write_dword(f, value):
    """Write a 32-bit unsigned integer (little-endian)"""
    f.write(struct.pack('<I', value))

def write_word(f, value):
    """Write a 16-bit unsigned integer (little-endian)"""
    f.write(struct.pack('<H', value))

def generate_bgra_image(width, height):
    """Generate a simple test pattern in BGRA format"""
    data = bytearray(width * height * 4)
    for y in range(height):
        for x in range(width):
            idx = (y * width + x) * 4
            # Left half red, right half blue (in BGRA format)
            if x < width // 2:
                data[idx + 0] = 0      # B
                data[idx + 1] = 0      # G
                data[idx + 2] = 255    # R
                data[idx + 3] = 255    # A
            else:
                data[idx + 0] = 255    # B
                data[idx + 1] = 0      # G
                data[idx + 2] = 0      # R
                data[idx + 3] = 255    # A
    return bytes(data)

def generate_mipmap(data, width, height):
    """Generate a mipmap level by downscaling 2x"""
    new_width = max(1, width // 2)
    new_height = max(1, height // 2)
    new_data = bytearray(new_width * new_height * 4)
    
    for y in range(new_height):
        for x in range(new_width):
            # Simple box filter
            src_x = x * 2
            src_y = y * 2
            
            idx = (y * new_width + x) * 4
            src_idx = (src_y * width + src_x) * 4
            
            # Copy pixel (simple nearest neighbor)
            new_data[idx:idx+4] = data[src_idx:src_idx+4]
    
    return bytes(new_data)

def write_dds_file(filename, width, height):
    """Write a DDS file with RGBA format and mipmaps"""
    
    # Calculate mipmap count
    mipmap_count = 1
    mip_w, mip_h = width, height
    while mip_w > 1 or mip_h > 1:
        mip_w = max(1, mip_w // 2)
        mip_h = max(1, mip_h // 2)
        mipmap_count += 1
    
    print(f"Generating DDS: {width}x{height} with {mipmap_count} mipmaps")
    
    with open(filename, 'wb') as f:
        # Write DDS magic number
        f.write(b'DDS ')
        
        # Write DDS header (124 bytes)
        write_dword(f, 124)  # dwSize
        write_dword(f, 0x1 | 0x2 | 0x4 | 0x1000 | 0x20000)  # dwFlags (CAPS, HEIGHT, WIDTH, PIXELFORMAT, MIPMAPCOUNT)
        write_dword(f, height)  # dwHeight
        write_dword(f, width)   # dwWidth
        write_dword(f, width * 4)  # dwPitchOrLinearSize
        write_dword(f, 0)  # dwDepth
        write_dword(f, mipmap_count)  # dwMipMapCount
        
        # Reserved1[11]
        for _ in range(11):
            write_dword(f, 0)
        
        # DDS_PIXELFORMAT (32 bytes)
        write_dword(f, 32)  # dwSize
        write_dword(f, 0x41)  # dwFlags (RGBA)
        write_dword(f, 0)  # dwFourCC
        write_dword(f, 32)  # dwRGBBitCount
        write_dword(f, 0x00FF0000)  # dwRBitMask (R at byte 2)
        write_dword(f, 0x0000FF00)  # dwGBitMask (G at byte 1)
        write_dword(f, 0x000000FF)  # dwBBitMask (B at byte 0)
        write_dword(f, 0xFF000000)  # dwABitMask (A at byte 3)
        
        # dwCaps
        write_dword(f, 0x1000 | 0x8 | 0x400000)  # TEXTURE | COMPLEX | MIPMAP
        write_dword(f, 0)  # dwCaps2
        write_dword(f, 0)  # dwCaps3
        write_dword(f, 0)  # dwCaps4
        write_dword(f, 0)  # dwReserved2
        
        # Write image data with mipmaps
        mip_w, mip_h = width, height
        mip_data = generate_bgra_image(width, height)
        
        for mip in range(mipmap_count):
            print(f"  Mipmap {mip}: {mip_w}x{mip_h} ({len(mip_data)} bytes)")
            f.write(mip_data)
            
            if mip < mipmap_count - 1:
                mip_data = generate_mipmap(mip_data, mip_w, mip_h)
                mip_w = max(1, mip_w // 2)
                mip_h = max(1, mip_h // 2)
    
    print(f"✓ Created {filename}")

if __name__ == "__main__":
    # Create resources directory if it doesn't exist
    os.makedirs("resources", exist_ok=True)
    
    # Generate test DDS file
    write_dds_file("resources/test_rgba_mipmaps.dds", 256, 256)
    
    print("\nTest file created!")
    print("Left half should be RED, right half should be BLUE")
    print("If the fix is working, all mipmap levels will show correct colors")
