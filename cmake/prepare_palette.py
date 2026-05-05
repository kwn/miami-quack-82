#!/usr/bin/env python3
import argparse
import os
import re
import shutil


COLOR_LINE_RE = re.compile(r"^(\s*)(\d+)(\s+)(\d+)(\s+)(\d+)(.*)$")


def to_ocs_channel(value):
    value = max(0, min(255, value))
    return ((value * 15 + 127) // 255) * 17


def convert_gpl_to_ocs(input_path, output_path):
    with open(input_path, "r", encoding="utf-8") as handle:
        lines = handle.readlines()

    converted_lines = []
    for line in lines:
        newline = "\n" if line.endswith("\n") else ""
        line_body = line[:-1] if newline else line

        match = COLOR_LINE_RE.match(line_body)
        if not match:
            converted_lines.append(line)
            continue

        prefix, r_sep, g_sep, suffix = match.group(1), match.group(3), match.group(5), match.group(7)
        r = to_ocs_channel(int(match.group(2)))
        g = to_ocs_channel(int(match.group(4)))
        b = to_ocs_channel(int(match.group(6)))
        converted_lines.append(f"{prefix}{r}{r_sep}{g}{g_sep}{b}{suffix}{newline}")

    with open(output_path, "w", encoding="utf-8", newline="") as handle:
        handle.writelines(converted_lines)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--mode", choices=("AGA", "OCS"), required=True)
    args = parser.parse_args()

    os.makedirs(os.path.dirname(args.output), exist_ok=True)

    if args.mode == "AGA":
        shutil.copyfile(args.input, args.output)
    else:
        convert_gpl_to_ocs(args.input, args.output)


if __name__ == "__main__":
    main()
