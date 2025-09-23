#!/usr/bin/env python3
"""
CVIModel Header Parser
Parses and displays CVIModel file header information based on the official structure
from cvi_runtime/include/runtime/model.hpp


example:
wget https://github.com/sophgo/tdl_models/raw/refs/heads/main/cv181x/scrfd_det_face_432_768_INT8_cv181x.cvimodel
python3 parse_cvimodel.py ~/work/codes/cvi_alios_open/models/scrfd_det_face_432_768_INT8_cv181x.cvimodel

python3 solutions/usb_cam/script/parse_cvimodel.py ~/work/codes/cvi_alios_open/models/scrfd_det_face_432_768_INT8_cv181x.cvimodel --verify-md5 --hexdump

"""

import struct
import sys
import os
import argparse
import hashlib
from typing import Optional

class CVIModelHeader:
    """CVIModel header structure matching cvi_runtime/include/runtime/model.hpp"""
    
    # Structure format for struct.unpack
    # < = little-endian, 8s = 8-byte string, I = uint32, B = uint8, 16s = 16-byte string, 2s = 2-byte string
    FORMAT = '<8sIBB16s16s2s'
    SIZE = struct.calcsize(FORMAT)  # Should be 32 bytes
    
    def __init__(self, data: bytes):
        """Parse CVIModel header from binary data"""
        if len(data) < self.SIZE:
            raise ValueError(f"Header data too small: {len(data)} < {self.SIZE}")
        
        unpacked = struct.unpack(self.FORMAT, data[:self.SIZE])
        
        self.magic = unpacked[0].decode('ascii', errors='ignore').rstrip('\x00')
        self.body_size = unpacked[1]    # not the byte size, but the number of parameters of the model
        self.major = unpacked[2]
        self.minor = unpacked[3]
        self.md5 = unpacked[4].hex() if unpacked[4] else "00000000000000000000000000000000"
        self.chip = unpacked[5].decode('ascii', errors='ignore').rstrip('\x00')
        self.padding = unpacked[6]
        
        # Calculate total file size
        self.total_size = self.SIZE + self.body_size
    
    def is_valid(self) -> bool:
        """Check if this is a valid CVIModel header"""
        return self.magic == "CviModel"
    
    def print_info(self):
        """Print formatted header information"""
        print("=" * 60)
        print("CVIModel Header Information")
        print("=" * 60)
        
        print(f"Magic:           {self.magic}")
        print(f"Valid:           {'✅ Yes' if self.is_valid() else '❌ No'}")
        print(f"Version:         {self.major}.{self.minor}")
        print(f"Chip Type:       {self.chip}")
        print(f"Body Size:       {self.body_size:,} bytes ({self.body_size / 1024:.1f} KB)")
        print(f"Header Size:     {self.SIZE} bytes")
        print(f"Total Size:      {self.total_size:,} bytes ({self.total_size / 1024:.1f} KB)")
        print(f"MD5 Checksum:    {self.md5}")
        
        # Show MD5 verification results if available
        if hasattr(self, 'calculated_md5'):
            print(f"Payload MD5:     {self.calculated_md5}")
            print(f"MD5 Match:       {'✅ Yes' if self.md5_match else '❌ No'}")
        
        print(f"Padding:         {self.padding} hex: {self.padding.hex()}")
        
        print("\n" + "=" * 60)
        print("File Analysis")
        print("=" * 60)
        
        if self.is_valid():
            print("✅ Valid CVIModel file")
            print(f"📊 Model parameters size: {self.body_size:,} bytes")
            print(f"🎯 Target chip: {self.chip}")
            print(f"📋 Version: {self.major}.{self.minor}")
            
            # Show MD5 verification status
            if hasattr(self, 'md5_match'):
                if self.md5_match:
                    print("🔐 MD5 verification: ✅ PASSED")
                else:
                    print("🔐 MD5 verification: ❌ FAILED")
                    print("⚠️  File integrity check failed!")
        else:
            print("❌ Invalid CVIModel file")
            print(f"🔍 Expected magic: 'CviModel'")
            print(f"🔍 Found magic: '{self.magic}'")

def calculate_payload_md5(filepath: str, header_size: int, payload_size: int) -> str:
    """Calculate MD5 hash of the payload data only (excluding header)"""
    hash_md5 = hashlib.md5()
    try:
        with open(filepath, "rb") as f:
            # Skip the header
            f.seek(header_size)
            # Read only the payload data
            remaining = payload_size
            while remaining > 0:
                chunk_size = min(4096, remaining)
                chunk = f.read(chunk_size)
                if not chunk:
                    break
                hash_md5.update(chunk)
                remaining -= len(chunk)
        return hash_md5.hexdigest()
    except Exception as e:
        print(f"❌ Error calculating payload MD5: {e}")
        return ""

def parse_cvimodel_file(filepath: str, verify_md5: bool = False) -> Optional[CVIModelHeader]:
    """Parse CVIModel file and return header information"""
    try:
        if not os.path.exists(filepath):
            print(f"❌ Error: File '{filepath}' does not exist")
            return None
        
        file_size = os.path.getsize(filepath)
        print(f"📁 File: {filepath}")
        print(f"📏 File size: {file_size:,} bytes ({file_size / 1024:.1f} KB)")
        print()
        
        with open(filepath, 'rb') as f:
            # Read header data
            header_data = f.read(CVIModelHeader.SIZE)
            
            if len(header_data) < CVIModelHeader.SIZE:
                print(f"❌ Error: File too small for CVIModel header ({len(header_data)} < {CVIModelHeader.SIZE})")
                return None
            
            header = CVIModelHeader(header_data)
            
            # Calculate and compare MD5 if requested
            if verify_md5:
                print("🔍 Calculating payload MD5...")
                calculated_md5 = calculate_payload_md5(filepath, CVIModelHeader.SIZE, header.body_size)
                header.calculated_md5 = calculated_md5
                header.md5_match = (header.md5.lower() == calculated_md5.lower())
                
                if header.md5_match:
                    print("✅ MD5 checksums match!")
                else:
                    print("❌ MD5 checksums do NOT match!")
                    print(f"   Header MD5:  {header.md5}")
                    print(f"   Payload MD5: {calculated_md5}")
                print()
                      
            return header
            
    except Exception as e:
        print(f"❌ Error parsing file: {e}")
        return None

def hexdump_header(filepath: str, bytes_count: int = 64):
    """Show hexdump of the first few bytes"""
    try:
        with open(filepath, 'rb') as f:
            data = f.read(bytes_count)
            
        print(f"\n🔍 Hexdump (first {bytes_count} bytes):")
        print("-" * 50)
        
        for i in range(0, len(data), 16):
            hex_part = ' '.join(f'{b:02x}' for b in data[i:i+16])
            ascii_part = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in data[i:i+16])
            print(f"{i:08x}: {hex_part:<48} |{ascii_part}|")
            
    except Exception as e:
        print(f"❌ Error creating hexdump: {e}")

def main():
    parser = argparse.ArgumentParser(
        description="Parse CVIModel file header information",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 parse_cvimodel.py model.cvimodel
  python3 parse_cvimodel.py model.cvimodel --hexdump
  python3 parse_cvimodel.py model.cvimodel --hexdump 128
  python3 parse_cvimodel.py model.cvimodel --verify-md5
  python3 parse_cvimodel.py model.cvimodel --verify-md5 --hexdump
        """
    )
    
    parser.add_argument('file', help='CVIModel file to parse')
    parser.add_argument('--hexdump', action='store_true', 
                       help='Show hexdump of file header')
    parser.add_argument('--hexdump-bytes', type=int, default=64,
                       help='Number of bytes to show in hexdump (default: 64)')
    parser.add_argument('--verify-md5', action='store_true',
                       help='Calculate and verify MD5 checksum of payload data against header')
    
    args = parser.parse_args()
    
    # Parse the CVIModel file
    header = parse_cvimodel_file(args.file, verify_md5=args.verify_md5)
    
    if header:
        header.print_info()
        
        if args.hexdump:
            hexdump_header(args.file, args.hexdump_bytes)
    else:
        print("❌ Failed to parse CVIModel file")
        sys.exit(1)

if __name__ == "__main__":
    main()


'''



📁 File: generated/data/weight.bin
📏 File size: 551,576 bytes (538.6 KB)

============================================================
CVIModel Header Information
============================================================
Magic:           CviModel
Valid:           ✅ Yes
Version:         1.4
Chip Type:       cv181x
Body Size:       12,040 bytes (11.8 KB)
Header Size:     48 bytes
Total Size:      12,088 bytes (11.8 KB)
MD5 Checksum:    89526e0ddf29cb5eed4a55951c058e7b
Padding:         4376

============================================================
File Analysis
============================================================
✅ Valid CVIModel file
📊 Model parameters: 12,040
🎯 Target chip: cv181x
📋 Version: 1.4

🔍 Hexdump (first 64 bytes):
--------------------------------------------------
00000000: 43 76 69 4d 6f 64 65 6c 08 2f 00 00 01 04 89 52  |CviModel./.....R|
00000010: 6e 0d df 29 cb 5e ed 4a 55 95 1c 05 8e 7b 63 76  |n..).^.JU....{cv|
00000020: 31 38 31 78 00 00 00 00 00 00 00 00 00 00 43 76  |181x..........Cv|
00000030: 1c 00 00 00 18 00 24 00 05 00 08 00 0c 00 00 00  |......$.........|


'''