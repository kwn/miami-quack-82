#!/usr/bin/env python3
import argparse
import os
import struct
import sys
import zlib

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def read_chunks(path):
    data = open(path, "rb").read()
    if not data.startswith(PNG_SIGNATURE):
        raise ValueError("Input is not a PNG file")

    chunks = []
    offset = len(PNG_SIGNATURE)
    while offset < len(data):
        length = struct.unpack(">I", data[offset:offset + 4])[0]
        chunk_type = data[offset + 4:offset + 8]
        chunk_data = data[offset + 8:offset + 8 + length]
        chunks.append((chunk_type, chunk_data))
        offset += 12 + length
        if chunk_type == b"IEND":
            break
    return chunks


def write_chunk(handle, chunk_type, chunk_data):
    handle.write(struct.pack(">I", len(chunk_data)))
    handle.write(chunk_type)
    handle.write(chunk_data)
    handle.write(struct.pack(">I", zlib.crc32(chunk_type + chunk_data) & 0xFFFFFFFF))


def read_gpl_palette(path):
    palette = bytearray()
    with open(path, "r", encoding="utf-8") as handle:
        for line in handle:
            stripped = line.strip()
            if not stripped or stripped.startswith("#"):
                continue
            parts = stripped.split()
            if len(parts) < 3:
                continue
            try:
                palette.extend((int(parts[0]), int(parts[1]), int(parts[2])))
            except ValueError:
                continue
    return bytes(palette)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--game-palette", required=True)
    args = parser.parse_args()

    chunks = read_chunks(args.input)
    ihdr = next((data for chunk_type, data in chunks if chunk_type == b"IHDR"), None)
    if ihdr is None:
        raise ValueError("PNG has no IHDR chunk")

    width, height, bit_depth, color_type, compression, filter_method, interlace = struct.unpack(">IIBBBBB", ihdr)
    if color_type != 3 or bit_depth != 8:
        raise ValueError("Expected 8-bit indexed PNG")
    if compression != 0 or filter_method != 0 or interlace != 0:
        raise ValueError("Unsupported PNG compression, filter or interlace mode")

    palette = read_gpl_palette(args.game_palette)

    os.makedirs(os.path.dirname(args.output), exist_ok=True)
    with open(args.output, "wb") as handle:
        handle.write(PNG_SIGNATURE)
        for chunk_type, chunk_data in chunks:
            if chunk_type == b"PLTE":
                write_chunk(handle, b"PLTE", palette)
            else:
                write_chunk(handle, chunk_type, chunk_data)


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(f"prepare_game_palette_png.py: {error}", file=sys.stderr)
        sys.exit(1)
