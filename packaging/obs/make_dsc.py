#!/usr/bin/env python3
'''
Write the `.dsc` file that OBS uses to build the Debian package.

OBS does not use the native `.dsc` that `dpkg-source` makes. Instead, OBS gives this `.dsc`,
`debian.tar.gz` and the release tarball to its `debtransform` tool, which makes a
"3.0 (quilt)" source package out of them. So a broken Debian packaging can be fixed by
publishing a new `debian.tar.gz`, while the tarball stays the same.
(See `.github/workflows/obs.yaml`.)

The fields are copied from the configured `debian/control`, so the build dependencies are
written in one place only.

Example:
    python3 packaging/obs/make_dsc.py --control generated/debian/control --version 0.9.0 \\
        --tarball qkiss_0.9.0.tar.gz --debian-tarball debian.tar.gz --output generated/obs/qkiss.dsc
'''

from __future__ import annotations
import argparse
from pathlib import Path
from typing import Dict, List

# Fields that the source paragraph (the first paragraph) of `debian/control` must have.
REQUIRED_SOURCE_FIELDS = ["Source", "Maintainer", "Build-Depends"]
# Fields of the source paragraph that are copied into the `.dsc`, in this order, when they exist.
COPIED_SOURCE_FIELDS = ["Maintainer", "Homepage", "Standards-Version",
                        "Build-Depends", "Build-Depends-Arch", "Build-Depends-Indep",
                        "Build-Conflicts", "Build-Conflicts-Arch", "Build-Conflicts-Indep"]


def parse_paragraphs(text: str) -> List[Dict[str, str]]:
    '''Parse a file like `debian/control` into one {field name: value} dict per paragraph.'''
    paragraphs: List[Dict[str, str]] = []
    fields: Dict[str, str] = {}
    name = ""
    for line in text.splitlines():
        if line.startswith("#"):
            continue
        if not line.strip():
            # An empty line ends the paragraph.
            if fields:
                paragraphs.append(fields)
            fields, name = {}, ""
        elif line[0] in " \t":
            # A line that starts with a space continues the value of the field above it.
            if not name:
                raise ValueError(f"This line continues no field: {line!r}")
            fields[name] += "\n" + line
        else:
            name, colon, value = line.partition(":")
            name = name.strip()
            if not colon or not name:
                raise ValueError(f"This line is not `Name: value`: {line!r}")
            if name in fields:
                raise ValueError(f"The field `{name}` appears twice in one paragraph.")
            fields[name] = value.strip()
    if fields:
        paragraphs.append(fields)
    for paragraph in paragraphs:
        for field_name, field_value in paragraph.items():
            if not field_value.strip():
                raise ValueError(f"The field `{field_name}` is empty.")
    return paragraphs


def format_field(name: str, value: str) -> str:
    # A value can start on the next line, like `Build-Depends:` in `debian/control`.
    return f"{name}:{value}" if value.startswith("\n") else f"{name}: {value}"


def make_dsc(control_text: str, version: str, tarball: str, debian_tarball: str) -> str:
    if "-" in version:
        raise ValueError(f"The version must not contain '-', because Debian reads the part after it as the packaging revision: {version}")
    paragraphs = parse_paragraphs(control_text)
    if len(paragraphs) < 2:
        raise ValueError("debian/control needs a source paragraph and at least one binary paragraph.")
    source, binaries = paragraphs[0], paragraphs[1:]
    for field in REQUIRED_SOURCE_FIELDS:
        if not source.get(field, "").strip():
            raise ValueError(f"The source paragraph of debian/control has no `{field}` field, or it is empty.")
    for binary in binaries:
        if not binary.get("Package", "").strip() or not binary.get("Architecture", "").strip():
            raise ValueError("Each binary paragraph of debian/control needs `Package` and `Architecture`.")
    # `dict.fromkeys()` removes the duplicates and keeps the order.
    architectures = " ".join(dict.fromkeys(binary["Architecture"] for binary in binaries))

    lines = [
        "Format: 3.0 (quilt)",
        format_field("Source", source["Source"]),
        "Binary: " + ", ".join(binary["Package"] for binary in binaries),
        "Architecture: " + architectures,
        "Version: " + version,
    ]
    lines += [format_field(field, source[field]) for field in COPIED_SOURCE_FIELDS if field in source]
    lines += [
        # The fields below are for `debtransform` only.
        "DEBTRANSFORM-TAR: " + tarball,
        "DEBTRANSFORM-FILES-TAR: " + debian_tarball,
        # Appends the release number of OBS to the version (like `0.9.0-1+3.1`).
        # OBS raises this number after every change of the package files, so a fixed packaging
        # gets a higher version, and the users get it as an update.
        "DEBTRANSFORM-RELEASE: 1",
    ]
    return "\n".join(lines) + "\n"


def main() -> None:
    ap = argparse.ArgumentParser(description="Write the .dsc file for OBS's debtransform from debian/control.")
    ap.add_argument("--control", required=True, type=Path, help="The configured debian/control file")
    ap.add_argument("--version", required=True, help="Version of the program, without a packaging revision (ex: 0.9.0)")
    ap.add_argument("--tarball", required=True, help="File name of the release tarball (ex: qkiss_0.9.0.tar.gz)")
    ap.add_argument("--debian-tarball", required=True, help="File name of the tarball of debian/ (ex: debian.tar.gz)")
    ap.add_argument("--output", required=True, type=Path, help="Where to write the .dsc file")
    args = ap.parse_args()
    try:
        dsc = make_dsc(args.control.read_text(encoding="utf-8"), args.version, args.tarball, args.debian_tarball)
    except ValueError as err:
        raise SystemExit(f"[make_dsc.py] Error: {err}")
    args.output.write_text(dsc, encoding="utf-8")


if __name__ == "__main__":
    main()
