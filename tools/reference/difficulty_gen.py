#!/usr/bin/env python3

import re
import sys

PARAM_COUNT = 14
SRC = "game/zGlobal_DATA.c"


def s16(token):
    m = re.search(r"0x[0-9a-fA-F]+", token)
    if not m:
        raise ValueError(f"no hex literal in {token!r}")
    value = int(m.group(0), 16) & 0xFFFF
    return value - 0x10000 if value >= 0x8000 else value


def parse_table(text, field_name):
    start = text.index(f".{field_name} =")
    entries = {}
    index = None
    current_set = None
    values = []
    for line in text[start:].splitlines()[1:]:
        stripped = line.strip()
        entry = re.match(r"\[(\d+)\]\s*=", stripped)
        if entry:
            index = int(entry.group(1))
            entries.setdefault(index, {})
            continue
        if stripped.startswith(".params1"):
            current_set, values = 1, []
            continue
        if stripped.startswith(".params2"):
            current_set, values = 2, []
            continue
        if current_set is not None and "0x" in stripped:
            for token in stripped.split(","):
                if "0x" in token:
                    values.append(s16(token))
            if len(values) >= PARAM_COUNT:
                entries[index][current_set] = values[:PARAM_COUNT]
                current_set = None
            continue
        if entries and re.match(r"\.\w+ =", stripped) and not stripped.startswith(
            (".params1", ".params2")
        ):
            break
    return entries


def main():
    out_dir = sys.argv[1] if len(sys.argv) > 1 else "."
    with open(SRC, encoding="utf-8") as handle:
        text = handle.read()

    arcade = parse_table(text, "ArcadeDifficulty")
    boss = parse_table(text, "BossDifficulty")

    header = "table,index,set," + ",".join(f"p{i}" for i in range(PARAM_COUNT))
    lines = [header]
    for name, table, count in (("arcade", arcade, 18), ("boss", boss, 6)):
        for index in range(count):
            for set_id in (1, 2):
                row = table[index][set_id]
                lines.append(f"{name},{index},{set_id}," + ",".join(str(v) for v in row))

    path = f"{out_dir}/difficulty.csv"
    with open(path, "wb") as handle:
        handle.write(("\n".join(lines) + "\n").encode("ascii"))
    print(f"difficulty_gen: wrote {path} ({len(lines) - 1} rows)")


if __name__ == "__main__":
    main()
