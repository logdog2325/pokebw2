#!/usr/bin/env python3
"""PearlB2: link our new code (src/pearlb2/*_itcm.c) after the game's own ITCM code.

    link_itcm.py LCF OBJECTS OBJ... OUT_LCF OUT_OBJECTS

PearlB2 also edits the game's code with binary patches at fixed addresses, so its C changes must not move anything
(include/pearlb2.h). Code that doesn't fit where it replaces the game's goes to ITCM, the last module the game's
memory layout doesn't chain anything after: this adds each object's sections to the end of the linker script's .itcm
section (before ITCM_TEXT_END, so the SDK's ITCM arena starts after it) and the objects to main's part of the linker's
object list (before the first -overlay option). With PEARLB2 off, the files compile to empty objects and the link is
the game's.
"""
import re
import sys
from pathlib import Path

ITCM_END = re.compile(r"^(\s*)\. = ALIGN\(4\);\n\s*ITCM_TEXT_END = \.;$", re.M)
SECTIONS = (".text", ".rodata", ".data")


def main():
    lcf, objects, *objs, out_lcf, out_objects = sys.argv[1:]
    script = Path(lcf).read_text()
    match = ITCM_END.search(script)
    if not match:
        sys.exit(f"{lcf}: no ITCM_TEXT_END in the .itcm section")
    indent = match.group(1)
    lines = "".join(f"{indent}{Path(o).name}({s})\n" for o in objs for s in SECTIONS)
    Path(out_lcf).write_text(script[:match.start()] + lines + script[match.start():])

    listed = Path(objects).read_text().splitlines()
    first_option = next((i for i, line in enumerate(listed) if line.startswith("-")), len(listed))
    listed[first_option:first_option] = [f'"{o}"' for o in objs]
    Path(out_objects).write_text("\n".join(listed) + "\n")


if __name__ == "__main__":
    main()
