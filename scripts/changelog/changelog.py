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
from enum import Enum
from typing import List, Optional, Literal, Dict, Any, Iterator, Tuple, get_args
import email.utils  # for RFC 2822 date formatting

import yaml  # PyYAML
import jsonschema  # jsonschema
from dataclasses import dataclass, field, asdict

MARKDOWN_TIMESTAMP_FORMAT = "%Y-%m-%d (%a) %H:%M %Z"

# ==========================================
# Data Models (@dataclass instead of dict for type safety)
# ==========================================

class ChangeCategory (Enum):
    added       = ("Added"      , "➕")  # ✨
    fixed       = ("Fixed"      , "🐞")  # 🐛🐝
    removed     = ("Removed"    , "➖")  # 🗑️
    style       = ("Style"      , "🎨")
    performance = ("Performance", "🚀")    # ⚡
    build       = ("Build"      , "⚒️")    # 🏗️
    security    = ("Security"   , "🛡")    # 🔒
    def __init__(self, label: str, emoji: str):
        self.label = label
        self.emoji = emoji

ALL_CHANGE_CATEGORIES = get_args(ChangeCategory)

@dataclass
class Body:
    added: List[str] = field(default_factory=list)  # type: ignore
    fixed: List[str] = field(default_factory=list)  # type: ignore
    removed: List[str] = field(default_factory=list)  # type: ignore
    style: List[str] = field(default_factory=list)  # type: ignore
    performance: List[str] = field(default_factory=list)  # type: ignore
    build: List[str] = field(default_factory=list)  # type: ignore
    security: List[str] = field(default_factory=list)  # type: ignore

    def __iter__(self) -> Iterator[Tuple[ChangeCategory, list[str]]]:
        for cat in ChangeCategory:
            values = getattr(self, cat.name)
            yield cat, values


@dataclass
class Release:
    """Changelogs of a release"""
    srcpkgname: str
    version: str
    date: datetime
    maintainer: str
    deb_urgency: Literal["low", "medium", "high"]
    body: Body


@dataclass
class RootData:
    releases: List[Release]

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

        releases: List[Release] = []
        for i, raw_release in enumerate(raw_root["releases"]):
            try:
                # Accept Z or offset; normalize Z to +00:00 for fromisoformat
                date_str = raw_release["date"]
                dt = datetime.fromisoformat(date_str.replace("Z", "+00:00"))
                # body
                raw_body = raw_release["body"]
                body = Body()
                for cate, logs in body:
                    if cate.name not in raw_body: continue
                    raw_logs: List[str] = raw_body[cate.name]
                    logs.extend(raw_logs)
                release = Release(
                    srcpkgname=raw_release["srcpkgname"],
                    version=raw_release["version"],
                    date=dt,
                    maintainer=raw_release["maintainer"],
                    deb_urgency=raw_release["deb"]["urgency"],
                    body=body,
                )
            except Exception as e:
                sys.exit(f"Invalid release at index {i}: {e}")
            releases.append(release)

        return RootData(releases=releases)


# ==========================================
# Converters
# ==========================================

def to_rpm(changelog: RootData) -> str:
    """
    Convert changelog entries to RPM %changelog format.
    """
    lines: List[str] = []
    for release in changelog.releases:
        # RPM header: "* Wed Dec 10 2025 Maintainer 1.2.3"
        date_str = release.date.strftime("%a %b %d %Y")
        header = f"* {date_str} {release.maintainer} {release.version}"
        lines.append(header)
        for cate, logs in release.body:
            for log in logs:
                lines.append(f"- {cate.label}: {log}")
        lines.append("")  # blank line between entries
    return "\n".join(lines).strip() + "\n"


def to_deb(changelog: RootData) -> str:
    """
    Convert changelog entries to Debian changelog format.
    """
    lines: List[str] = []
    for release in changelog.releases:
        # Debian header:
        # When debian/source/format is 3.0 (quilt):  "name (version-1) unstable; urgency=medium"
        # When debian/source/format is 3.0 (native): "name (version) unstable; urgency=medium"
        header = f"{release.srcpkgname} ({release.version}) unstable; urgency={release.deb_urgency}"
        lines.append(header)
        lines.append("")
        for cate, logs in release.body:
            for log in logs:
                lines.append(f"  * {cate.label}: {log}")
        lines.append("")
        # Debian footer: " -- Maintainer  Wed, 10 Dec 2025 12:00:00 +0800"
        date_str = email.utils.format_datetime(release.date)
        footer = f" -- {release.maintainer}  {date_str}"
        lines.append(footer)
        lines.append("")  # blank line between entries
    return "\n".join(lines).strip() + "\n"


def to_markdown_full(changelog: RootData) -> str:
    """
    Convert all changelog entries to Markdown.
    """
    lines: List[str] = []
    for release in changelog.releases:
        lines.append(f"# {release.srcpkgname} {release.version}")
        lines.append(f"Date: {release.date.strftime(MARKDOWN_TIMESTAMP_FORMAT)}")
        lines.append("")
        for cate, logs in release.body:
            if len(logs) == 0: continue
            lines.append(f"- {cate.emoji} **{cate.label}**")
            for log in logs:
                lines.append(f"  - {log}")
        lines.append("")  # blank line between entries
    return "\n".join(lines).strip() + "\n"


def to_markdown_latest(changelog: RootData) -> str:
    """
    Convert only the latest changelog entry to Markdown.
    """
    if not changelog.releases:
        return ""
    release = changelog.releases[-1]
    lines: List[str] = []
    lines.append(f"# {release.srcpkgname} {release.version}")
    lines.append(f"Date: {release.date.strftime(MARKDOWN_TIMESTAMP_FORMAT)}")
    lines.append("")

    for cate, logs in release.body:
        if len(logs) == 0: continue
        lines.append(f"- {cate.emoji} **{cate.label}**")
        for log in logs:
            lines.append(f"  - {log}")
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
            self.options.output_rpm.parent.mkdir(parents=True, exist_ok=True)
            self.options.output_rpm.write_text(rpm_text, encoding="utf-8")

        if self.options.output_deb:
            deb_text = to_deb(changelog)
            self.options.output_deb.parent.mkdir(parents=True, exist_ok=True)
            self.options.output_deb.write_text(deb_text, encoding="utf-8")

        if self.options.output_markdown:
            md_text = to_markdown_full(changelog)
            self.options.output_markdown.parent.mkdir(parents=True, exist_ok=True)
            self.options.output_markdown.write_text(md_text, encoding="utf-8")

        if self.options.output_latest_markdown:
            latest_md = to_markdown_latest(changelog)
            self.options.output_latest_markdown.parent.mkdir(parents=True, exist_ok=True)
            self.options.output_latest_markdown.write_text(latest_md, encoding="utf-8")


def main(argv: List[str]) -> None:
    ap = argparse.ArgumentParser(description="Universal changelog parser and converter (for RPM / DEB / Markdown)")
    ap.add_argument("--input-yaml", "-i", required=True, type=Path,
                    help="Path to the input changelog YAML file (default: ./schema.json)")
    ap.add_argument("--json-schema", "-j", required=False, type=Path, default=Path(__file__).parent / 'schema.json',
                    help="Path to JSON Schema file used to validate the YAML")
    ap.add_argument("--output-rpm", "-r", type=Path,
                    help="""Output a plain text file containing changelog in RPM .spec format at this path,
                    you need to insert the contents of the outputted file  after `changelog` in .spec file manually (e.g. via CMake's `configure_file()`)""")
    ap.add_argument("--output-deb", "-d", type=Path,
                    help="Output a plain text file containing changelog in Debian changelog format at this path")
    ap.add_argument("--output-markdown", "-m", type=Path,
                    help="Write full changelog into Markdown file at this path")
    ap.add_argument("--output-latest-markdown", "-l", type=Path,
                    help="Write only the latest release entry into Markdown file at this path")

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
