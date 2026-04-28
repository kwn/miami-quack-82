#!/usr/bin/env python3
import argparse
import os
import struct
import sys
import zlib

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def paeth(a, b, c):
    p = a + b - c
    pa = abs(p - a)
    pb = abs(p - b)
    pc = abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    if pb <= pc:
        return b
    return c


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


def unfilter_rows(raw, width, height):
    rows = []
    prev = bytearray(width)
    offset = 0

    for _ in range(height):
        filter_type = raw[offset]
        offset += 1
        src = bytearray(raw[offset:offset + width])
        offset += width
        dst = bytearray(width)

        for x in range(width):
            left = dst[x - 1] if x else 0
            up = prev[x]
            up_left = prev[x - 1] if x else 0

            if filter_type == 0:
                value = src[x]
            elif filter_type == 1:
                value = src[x] + left
            elif filter_type == 2:
                value = src[x] + up
            elif filter_type == 3:
                value = src[x] + ((left + up) >> 1)
            elif filter_type == 4:
                value = src[x] + paeth(left, up, up_left)
            else:
                raise ValueError(f"Unsupported PNG filter: {filter_type}")

            dst[x] = value & 0xFF

        rows.append(dst)
        prev = dst

    return rows


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--tile-width", type=int, required=True)
    parser.add_argument("--tile-height", type=int, required=True)
    parser.add_argument("--columns", type=int, required=True)
    parser.add_argument("--rows", type=int, required=True)
    args = parser.parse_args()

    chunks = read_chunks(args.input)
    ihdr = next((data for chunk_type, data in chunks if chunk_type == b"IHDR"), None)
    if ihdr is None:
        raise ValueError("PNG has no IHDR chunk")

    width, height, bit_depth, color_type, compression, filter_method, interlace = struct.unpack(">IIBBBBB", ihdr)
    expected_width = args.tile_width * args.columns
    expected_height = args.tile_height * args.rows

    if width != expected_width or height != expected_height:
        raise ValueError(f"Expected {expected_width}x{expected_height}, got {width}x{height}")
    if color_type != 3 or bit_depth != 8:
        raise ValueError("Expected 8-bit indexed PNG")
    if compression != 0 or filter_method != 0 or interlace != 0:
        raise ValueError("Unsupported PNG compression, filter or interlace mode")

    raw = zlib.decompress(b"".join(data for chunk_type, data in chunks if chunk_type == b"IDAT"))
    source_rows = unfilter_rows(raw, width, height)
    output_rows = []

    for tile_y in range(args.rows):
        for tile_x in range(args.columns):
            src_x = tile_x * args.tile_width
            src_y = tile_y * args.tile_height
            for row in range(args.tile_height):
                output_rows.append(source_rows[src_y + row][src_x:src_x + args.tile_width])

    out_raw = bytearray()
    for row in output_rows:
        out_raw.append(0)
        out_raw.extend(row)

    out_height = args.tile_height * args.columns * args.rows
    os.makedirs(os.path.dirname(args.output), exist_ok=True)
    with open(args.output, "wb") as handle:
        handle.write(PNG_SIGNATURE)
        write_chunk(handle, b"IHDR", struct.pack(">IIBBBBB", args.tile_width, out_height, bit_depth, color_type, 0, 0, 0))
        for chunk_type, chunk_data in chunks:
            if chunk_type in (b"IHDR", b"IDAT", b"IEND"):
                continue
            write_chunk(handle, chunk_type, chunk_data)
        write_chunk(handle, b"IDAT", zlib.compress(bytes(out_raw)))
        write_chunk(handle, b"IEND", b"")


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(f"prepare_tileset_column.py: {error}", file=sys.stderr)
        sys.exit(1)
