#!/usr/bin/env python3

# ==========================================
# Warning and Acknowedgement
#
# This script heavily relys on M$ Copilot, which wrote > 90% code.
# Human reviewed.
# ==========================================

from __future__ import annotations
import argparse
import sys
import json
from pathlib import Path
from datetime import datetime
from typing import List, Optional, Literal, Dict, Any
import email.utils  # for RFC 2822 date formatting

import yaml  # PyYAML
import jsonschema  # jsonschema
from dataclasses import dataclass


# ==========================================
# Data Models
# ==========================================

@dataclass
class ChangelogEntry:
    srcpkgname: str
    version: str
    date: datetime
    maintainer: str
    deb_urgency: Literal["low", "medium", "high"]
    changes: List[str]


@dataclass
class RootData:
    changelogs: List[ChangelogEntry]

@dataclass
class Options:
    input_yaml: Path
    json_schema: Path
    output_rpm: Optional[Path] = None
    output_deb: Optional[Path] = None
    output_markdown: Optional[Path] = None
    output_latest_markdown: Optional[Path] = None

# ==========================================
# Parser
# ==========================================

class Parser:
    def __init__(self, options: Options) -> None:
        self.options: Options = options

    def load_and_validate(self) -> RootData:
        """
        Load YAML, validate against JSON schema.
        If invalid, raise SystemExit with error.
        """
        # Step 0: explicit existence checks
        if not self.options.input_yaml.exists():
            sys.exit(f"Input YAML not found: {self.options.input_yaml}")
        if not self.options.json_schema.exists():
            sys.exit(f"JSON schema not found: {self.options.json_schema}")

        # Step 1: Load YAML to raw dict
        try:
            with self.options.input_yaml.open("r", encoding="utf-8") as f:
                raw_root: Dict[str, Any] = yaml.safe_load(f)  # type: ignore[assignment]
        except Exception as e:
            sys.exit(f"Error loading YAML: {e}")

        # Step 2: Load JSON Schema
        try:
            with self.options.json_schema.open("r", encoding="utf-8") as f:
                schema: Dict[str, Any] = json.load(f)
        except Exception as e:
            sys.exit(f"Error loading JSON schema: {e}")

        # Step 3: Validate raw dict against schema
        try:
            jsonschema.validate(instance=raw_root, schema=schema)
        except jsonschema.ValidationError as e:
            sys.exit(f"Schema validation failed: {e.message}")

        # Step 4: Convert raw dict -> dataclass instances

        entries: List[ChangelogEntry] = []
        for i, entry in enumerate(raw_root["changelogs"]):
            try:
                # Accept Z or offset; normalize Z to +00:00 for fromisoformat
                date_str = entry["date"]
                dt = datetime.fromisoformat(date_str.replace("Z", "+00:00"))

                ce = ChangelogEntry(
                    srcpkgname=entry["srcpkgname"],
                    version=entry["version"],
                    date=dt,
                    maintainer=entry["maintainer"],
                    deb_urgency=entry["deb"]["urgency"],
                    changes=entry["changes"],
                )
            except Exception as e:
                sys.exit(f"Invalid entry at index {i}: {e}")
            entries.append(ce)

        return RootData(changelogs=entries)


# ==========================================
# Converters
# ==========================================

def to_rpm(changelog: RootData) -> str:
    """
    Convert changelog entries to RPM %changelog format.
    """
    lines: List[str] = []
    for entry in changelog.changelogs:
        # RPM header: "* Wed Dec 10 2025 Maintainer 1.2.3"
        date_str = entry.date.strftime("%a %b %d %Y")
        header = f"* {date_str} {entry.maintainer} {entry.version}"
        lines.append(header)
        for change in entry.changes:
            lines.append(f"- {change}")
        lines.append("")  # blank line between entries
    return "\n".join(lines).strip() + "\n"


def to_deb(changelog: RootData) -> str:
    """
    Convert changelog entries to Debian changelog format.
    """
    lines: List[str] = []
    for entry in changelog.changelogs:
        # Debian header: "name (version-1) unstable; urgency=medium"
        header = f"{entry.srcpkgname} ({entry.version}-1) unstable; urgency={entry.deb_urgency}"
        lines.append(header)
        lines.append("")
        for change in entry.changes:
            lines.append(f"  * {change}")
        lines.append("")
        # Debian footer: " -- Maintainer  Wed, 10 Dec 2025 12:00:00 +0800"
        date_str = email.utils.format_datetime(entry.date)
        footer = f" -- {entry.maintainer}  {date_str}"
        lines.append(footer)
        lines.append("")  # blank line between entries
    return "\n".join(lines).strip() + "\n"


def to_markdown_full(changelog: RootData) -> str:
    """
    Convert all changelog entries to Markdown.
    """
    lines: List[str] = []
    for entry in changelog.changelogs:
        lines.append(f"# {entry.srcpkgname} {entry.version}")
        lines.append(f"Date: {entry.date.isoformat()}")
        lines.append("")
        for change in entry.changes:
            lines.append(f"- {change}")
        lines.append("")  # blank line between entries
    return "\n".join(lines).strip() + "\n"


def to_markdown_latest(changelog: RootData) -> str:
    """
    Convert only the latest changelog entry to Markdown.
    """
    if not changelog.changelogs:
        return ""
    entry = changelog.changelogs[-1]
    lines: List[str] = []
    lines.append(f"# {entry.srcpkgname} {entry.version}")
    lines.append(f"Date: {entry.date.isoformat()}")
    lines.append("")
    for change in entry.changes:
        lines.append(f"- {change}")
    lines.append("")
    return "\n".join(lines).strip() + "\n"



# ==========================================
# Main
# ==========================================

class Runner:
    def __init__(self, options: Options) -> None:
        self.options: Options = options

    def run(self) -> None:
        parser = Parser(self.options)
        changelog = parser.load_and_validate()

        if self.options.output_rpm:
            rpm_text = to_rpm(changelog)
            self.options.output_rpm.write_text(rpm_text, encoding="utf-8")

        if self.options.output_deb:
            deb_text = to_deb(changelog)
            self.options.output_deb.write_text(deb_text, encoding="utf-8")

        if self.options.output_markdown:
            md_text = to_markdown_full(changelog)
            self.options.output_markdown.write_text(md_text, encoding="utf-8")

        if self.options.output_latest_markdown:
            latest_md = to_markdown_latest(changelog)
            self.options.output_latest_markdown.write_text(latest_md, encoding="utf-8")


def main(argv: List[str]) -> None:
    ap = argparse.ArgumentParser(description="Universal changelog parser and converter")
    ap.add_argument("--input-yaml", "-i", required=True, type=Path)
    ap.add_argument("--json-schema", "-j", required=True, type=Path)
    ap.add_argument("--output-rpm", "-r", type=Path)
    ap.add_argument("--output-deb", "-d", type=Path)
    ap.add_argument("--output-markdown", "-m", type=Path)
    ap.add_argument("--output-latest-markdown", "-l", type=Path)

    args = ap.parse_args(argv)

    options = Options(
        input_yaml=args.input_yaml,
        json_schema=args.json_schema,
        output_rpm=args.output_rpm,
        output_deb=args.output_deb,
        output_markdown=args.output_markdown,
        output_latest_markdown=args.output_latest_markdown,
    )

    runner = Runner(options)
    runner.run()


if __name__ == "__main__":
    main(sys.argv[1:])
