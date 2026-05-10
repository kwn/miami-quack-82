#!/usr/bin/env python3
import argparse
import os
import struct
import sys
import zlib

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"
MASK_RGB = (255, 0, 255)


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
    row_size = width
    rows = []
    prev = bytearray(row_size)
    offset = 0

    for _ in range(height):
        filter_type = raw[offset]
        offset += 1
        src = bytearray(raw[offset:offset + row_size])
        offset += row_size
        dst = bytearray(row_size)

        for x in range(row_size):
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


def read_gpl_palette(path):
    colors = []
    with open(path, "r", encoding="utf-8") as handle:
        for line in handle:
            stripped = line.strip()
            if not stripped or stripped.startswith("#"):
                continue
            parts = stripped.split()
            if len(parts) < 3:
                continue
            try:
                colors.append(bytes((int(parts[0]), int(parts[1]), int(parts[2]))))
            except ValueError:
                continue
    return colors


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--width", type=int, required=True)
    parser.add_argument("--height", type=int, required=True)
    parser.add_argument("--game-palette")
    args = parser.parse_args()

    chunks = read_chunks(args.input)
    ihdr = next((data for chunk_type, data in chunks if chunk_type == b"IHDR"), None)
    if ihdr is None:
        raise ValueError("PNG has no IHDR chunk")

    width, height, bit_depth, color_type, compression, filter_method, interlace = struct.unpack(">IIBBBBB", ihdr)
    if width != args.width or height != args.height:
        raise ValueError(f"Expected {args.width}x{args.height}, got {width}x{height}")
    if color_type != 3 or bit_depth != 8:
        raise ValueError("Expected 8-bit indexed PNG")
    if compression != 0 or filter_method != 0 or interlace != 0:
        raise ValueError("Unsupported PNG compression, filter or interlace mode")

    palette = next((data for chunk_type, data in chunks if chunk_type == b"PLTE"), None)
    transparency = next((data for chunk_type, data in chunks if chunk_type == b"tRNS"), b"")
    if palette is None:
        raise ValueError("Indexed PNG has no PLTE chunk")

    raw = zlib.decompress(b"".join(data for chunk_type, data in chunks if chunk_type == b"IDAT"))
    rows = unfilter_rows(raw, width, height)
    game_palette = read_gpl_palette(args.game_palette) if args.game_palette else None

    out_raw = bytearray()
    for row in rows:
        out_raw.append(0)
        for color_idx in row:
            alpha = transparency[color_idx] if color_idx < len(transparency) else 255
            palette_offset = color_idx * 3
            source_rgb = palette[palette_offset:palette_offset + 3]
            if alpha == 0 or source_rgb == MASK_RGB:
                out_raw.extend(MASK_RGB)
            else:
                if game_palette:
                    if color_idx >= len(game_palette):
                        raise ValueError(f"Color index {color_idx} missing from {args.game_palette}")
                    out_raw.extend(game_palette[color_idx])
                else:
                    out_raw.extend(source_rgb)

    os.makedirs(os.path.dirname(args.output), exist_ok=True)
    with open(args.output, "wb") as handle:
        handle.write(PNG_SIGNATURE)
        write_chunk(handle, b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
        write_chunk(handle, b"IDAT", zlib.compress(bytes(out_raw)))
        write_chunk(handle, b"IEND", b"")


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(f"prepare_masked_png.py: {error}", file=sys.stderr)
        sys.exit(1)
