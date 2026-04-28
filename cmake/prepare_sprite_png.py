#!/usr/bin/env python3
import argparse
import os
import struct
import sys
import zlib

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def read_chunks(path):
    with open(path, "rb") as handle:
        data = handle.read()

    if not data.startswith(PNG_SIGNATURE):
        raise ValueError("Input is not a PNG file")

    chunks = []
    offset = len(PNG_SIGNATURE)
    while offset < len(data):
        if offset + 8 > len(data):
            raise ValueError("Truncated PNG chunk header")
        length = struct.unpack(">I", data[offset:offset + 4])[0]
        chunk_type = data[offset + 4:offset + 8]
        chunk_data_start = offset + 8
        chunk_data_end = chunk_data_start + length
        crc_end = chunk_data_end + 4
        if crc_end > len(data):
            raise ValueError("Truncated PNG chunk data")
        chunks.append((chunk_type, data[chunk_data_start:chunk_data_end]))
        offset = crc_end
        if chunk_type == b"IEND":
            break

    return chunks


def write_chunk(handle, chunk_type, chunk_data):
    handle.write(struct.pack(">I", len(chunk_data)))
    handle.write(chunk_type)
    handle.write(chunk_data)
    handle.write(struct.pack(">I", zlib.crc32(chunk_type + chunk_data) & 0xFFFFFFFF))


def row_byte_count(width, bit_depth, color_type):
    samples_by_color_type = {
        0: 1,  # grayscale
        2: 3,  # RGB
        3: 1,  # indexed
        4: 2,  # grayscale + alpha
        6: 4,  # RGBA
    }
    if color_type not in samples_by_color_type:
        raise ValueError(f"Unsupported PNG color type: {color_type}")

    bits_per_pixel = samples_by_color_type[color_type] * bit_depth
    return (width * bits_per_pixel + 7) // 8


def write_gpl(path, palette_data, color_count):
    if len(palette_data) < color_count * 3:
        raise ValueError(f"PNG palette has fewer than {color_count} colors")

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="ascii") as handle:
        handle.write("GIMP Palette\n")
        handle.write("Name: aim\n")
        handle.write("Columns: 4\n")
        handle.write("#\n")
        for idx in range(color_count):
            r, g, b = palette_data[idx * 3:idx * 3 + 3]
            handle.write(f"{r:3d} {g:3d} {b:3d} Index {idx}\n")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output-png", required=True)
    parser.add_argument("--output-palette", required=True)
    parser.add_argument("--width", type=int, required=True)
    parser.add_argument("--height", type=int, required=True)
    args = parser.parse_args()

    chunks = read_chunks(args.input)
    ihdr = next((data for chunk_type, data in chunks if chunk_type == b"IHDR"), None)
    if ihdr is None:
        raise ValueError("PNG has no IHDR chunk")

    width, height, bit_depth, color_type, compression, filter_method, interlace = struct.unpack(">IIBBBBB", ihdr)
    if width != args.width or height != args.height:
        raise ValueError(f"Expected {args.width}x{args.height}, got {width}x{height}")
    if color_type != 3:
        raise ValueError("Sprite PNG must be indexed-color so its 2BPP palette is deterministic")
    if bit_depth not in (1, 2, 4, 8):
        raise ValueError(f"Unsupported indexed PNG bit depth: {bit_depth}")
    if compression != 0 or filter_method != 0 or interlace != 0:
        raise ValueError("Unsupported PNG compression, filter or interlace mode")

    palette_data = next((data for chunk_type, data in chunks if chunk_type == b"PLTE"), None)
    if palette_data is None:
        raise ValueError("Indexed PNG has no PLTE chunk")
    write_gpl(args.output_palette, palette_data, 4)

    idat_data = b"".join(data for chunk_type, data in chunks if chunk_type == b"IDAT")
    raw = zlib.decompress(idat_data)
    row_size = row_byte_count(width, bit_depth, color_type)
    expected_raw_size = (row_size + 1) * height
    if len(raw) != expected_raw_size:
        raise ValueError(f"Unexpected PNG data size: {len(raw)} != {expected_raw_size}")

    blank_row = b"\x00" + bytes(row_size)
    padded_raw = blank_row + raw + blank_row
    padded_idat = zlib.compress(padded_raw)
    padded_ihdr = struct.pack(">IIBBBBB", width, height + 2, bit_depth, color_type, compression, filter_method, interlace)

    pre_idat = []
    post_idat = []
    seen_idat = False
    for chunk_type, chunk_data in chunks:
        if chunk_type in (b"IHDR", b"IEND"):
            continue
        if chunk_type == b"IDAT":
            seen_idat = True
            continue
        if seen_idat:
            post_idat.append((chunk_type, chunk_data))
        else:
            pre_idat.append((chunk_type, chunk_data))

    os.makedirs(os.path.dirname(args.output_png), exist_ok=True)
    with open(args.output_png, "wb") as handle:
        handle.write(PNG_SIGNATURE)
        write_chunk(handle, b"IHDR", padded_ihdr)
        for chunk_type, chunk_data in pre_idat:
            write_chunk(handle, chunk_type, chunk_data)
        write_chunk(handle, b"IDAT", padded_idat)
        for chunk_type, chunk_data in post_idat:
            write_chunk(handle, chunk_type, chunk_data)
        write_chunk(handle, b"IEND", b"")


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(f"prepare_sprite_png.py: {error}", file=sys.stderr)
        sys.exit(1)
