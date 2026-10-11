#!/usr/bin/env python3
"""
Assemble the QKiSS website, a single static page, as GitHub Pages and GitLab
Pages publish it. Both CI pipelines run this script, so both sites are always
identical.

    python3 homepage/build.py OUT_DIR --screenshots DIR         # what CI runs (`make homepage-build` too)
    python3 homepage/build.py OUT_DIR --no-screenshots          # `make homepage-build HOMEPAGE_SCREENSHOTS=`
    python3 homepage/build.py --serve [PORT] --screenshots DIR  # `make homepage-dev-server`

DIR is the `screenshots/` folder of the wiki (`qkiss.wiki.git`): the images
that `homepage/screenshots.json` lists. Everything is checked before anything
is written: on any error, nothing is built and the exit status is 1.

`--serve` builds the site into a temporary folder instead, and serves it on
PORT (8000 by default) on every network interface. It prints the URLs to
open, including this machine's LAN addresses, so that the host of a VM can
open the site too. When a source file changes, the script checks and
rebuilds the site; reload the page to see the result. If the check fails,
the server keeps serving the last good build and prints the errors. When
this script itself changes, the server restarts.

This script uses the Python standard library, and `rsvg-convert` (librsvg) for
the link-preview image (OG_IMAGE_SVG). See `homepage/README.md`.
"""

import argparse
import json
import os
import re
import shutil
import socket
import stat
import struct
import subprocess
import sys
import tempfile
import threading
import time
from html.parser import HTMLParser
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any, Callable

SCRIPT_PATH: Path = Path(__file__).resolve()
HOMEPAGE_DIR: Path = SCRIPT_PATH.parent
SRC_DIR: Path = HOMEPAGE_DIR / "src"
I18N_DIR: Path = SRC_DIR / "i18n"
REPO_ROOT_DIR: Path = HOMEPAGE_DIR.parent

# Icons of the app itself, copied at build time instead of being duplicated in
# `src/`. If you add files from elsewhere in the repository, also add them to
# the path filters in `.github/workflows/homepage.yaml` and `.gitlab-ci.yml`.
APP_ICONS: dict[str, Path] = {
    "images/qkiss.svg":           REPO_ROOT_DIR / "resources/appicon/apps/256/qkiss.svg",
    "images/qkiss-32.svg":        REPO_ROOT_DIR / "resources/appicon/apps/32/qkiss.svg",
    "images/qkiss-installer.svg": REPO_ROOT_DIR / "resources/appicon/apps/48/qkiss-installer.svg",
    "images/tux.svg":             REPO_ROOT_DIR / "resources/appicon/actions/22/tux.svg",
}

# The flags in `resources/flags/` come from lipis/flag-icons (MIT license). The `flag` field of a
# language names a file in `svg/`, without `.svg`. The build copies the flags it needs to
# `images/flags/`, together with their license notice (README.md).
FLAGS_DIR: Path = REPO_ROOT_DIR / "resources/flags"

SCREENSHOTS_SUBDIR: str = "screenshots"
SCREENSHOTS_MANIFEST: str = "screenshots.json"
# This file sets the order and the captions of the screenshots in the gallery. The images
# themselves come from the wiki, through the folder given with --screenshots.
SCREENSHOTS_MANIFEST_PATH: Path = HOMEPAGE_DIR / SCREENSHOTS_MANIFEST

# The link-preview image (`og:image` in index.html). Link previews do not show SVG, so the build uses rsvg-convert
# to convert this SVG into OG_IMAGE_PNG, with a white background. It also copies the SVG as is.
OG_IMAGE_SVG: Path = SRC_DIR / "images/og-image.svg"
OG_IMAGE_PNG: str = "images/og-image.png"
# Its width and height in pixels. These must match `og:image:width` and `og:image:height` in index.html.
OG_IMAGE_SIZE: tuple[int, int] = (1280, 640)

# The fields a language may have in `<script id="languages">` of index.html, and whether each one is required.
LANGUAGE_FIELDS: dict[str, bool] = {"id": True, "name": True, "lang": False, "fallback": False, "flag": False}

PATT_KEY = re.compile(r"[a-z][a-z0-9-]*(\.[a-z][a-z0-9-]*)*")
PATT_ATTRIBUTE = re.compile(r"[a-z][a-z0-9-]*")
PATT_LANGUAGE_ID = re.compile(r"[A-Za-z]{2,3}(-[A-Za-z0-9]+)*")
PATT_FLAG = re.compile(r"[a-z]{2}(-[a-z]+)*")
PATT_IMAGE_NAME = re.compile(r"[A-Za-z0-9][A-Za-z0-9._-]*\.(png|jpg|jpeg|webp|avif|gif|webm)")


class BuildError(Exception):
    """Something that must be fixed before the website can be built."""


def load_json(text: str, where: str) -> Any:
    """`json.loads()`, but a duplicated key is an error instead of silently keeping the last one."""
    def reject_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
        obj: dict[str, Any] = {}
        for key, value in pairs:
            if key in obj:
                raise BuildError(f"{where}: duplicated key {key!r}")
            obj[key] = value
        return obj
    try:
        return json.loads(text, object_pairs_hook=reject_duplicates)
    except json.JSONDecodeError as e:
        raise BuildError(f"{where}: invalid JSON: {e}") from None


def is_nonempty_string(value: Any) -> bool:
    return isinstance(value, str) and value.strip() != ""


# ==========================================
# index.html: translation keys and the language registry
# ==========================================

class PageParser(HTMLParser):
    """Collect the translation keys and the language registry of index.html."""

    def __init__(self) -> None:
        super().__init__()
        self.keys: dict[str, str] = {}       # key -> how it is used: "data-i18n", "data-i18n-html" or "data-i18n-attr"
        self.languages_json: str | None = None
        self.errors: list[str] = []
        self._in_languages_script = False

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        for name, value in attrs:
            if name in ("data-i18n", "data-i18n-html"):
                self._add_key(value, name)
            elif name == "data-i18n-attr":
                for item in (value or "").split(";"):
                    attribute, colon, key = item.partition(":")
                    if colon and PATT_ATTRIBUTE.fullmatch(attribute):
                        self._add_key(key, name)
                    else:
                        self._error(f"data-i18n-attr: invalid item {item!r}; expected `attribute:key`, items separated by `;` without spaces")
        if tag == "script" and ("id", "languages") in attrs:
            self._in_languages_script = True
            self.languages_json = ""

    def handle_endtag(self, tag: str) -> None:
        if tag == "script":
            self._in_languages_script = False

    def handle_data(self, data: str) -> None:
        if self._in_languages_script:
            self.languages_json = (self.languages_json or "") + data

    def _add_key(self, key: str | None, usage: str) -> None:
        if key is None or not PATT_KEY.fullmatch(key):
            self._error(f"{usage}: invalid key {key!r}; a key is lowercase words joined by `.`, e.g. `about.why.title`")
        elif self.keys.setdefault(key, usage) != usage:
            self._error(f"key {key!r} is used by both {self.keys[key]} and {usage}; use one key per usage")

    def _error(self, message: str) -> None:
        line, column = self.getpos()
        self.errors.append(f"index.html:{line}:{column + 1}: {message}")


def check_languages(text: str | None) -> list[dict[str, str]]:
    """Validate the language registry (`<script type="application/json" id="languages">` in index.html)."""
    where = 'index.html <script id="languages">'
    if text is None:
        raise BuildError(f"{where}: not found")
    languages = load_json(text, where)
    if not isinstance(languages, list) or not languages:
        raise BuildError(f"{where}: must be a non-empty array of languages")

    errors: list[str] = []
    for i, language in enumerate(languages):
        if not isinstance(language, dict):
            raise BuildError(f"{where}: [{i}] must be an object")
        for field in sorted(language.keys() - LANGUAGE_FIELDS.keys()):
            errors.append(f"{where}: [{i}]: unknown field {field!r}; allowed fields: {', '.join(LANGUAGE_FIELDS)}")
        for field, required in LANGUAGE_FIELDS.items():
            if field in language and not is_nonempty_string(language[field]):
                errors.append(f"{where}: [{i}]: {field!r} must be a non-empty string")
            elif required and field not in language:
                errors.append(f"{where}: [{i}]: {field!r} is required")
    if errors:
        raise BuildError("\n".join(errors))

    ids = [language["id"] for language in languages]
    for i, language in enumerate(languages):
        if not PATT_LANGUAGE_ID.fullmatch(language["id"]):
            errors.append(f"{where}: [{i}]: id {language['id']!r} must look like a BCP 47 tag, e.g. `ja`, `zh-TW`, `zh-HK`")
        elif ids.count(language["id"]) > 1:
            errors.append(f"{where}: [{i}]: id {language['id']!r} is duplicated")
        if "fallback" in language and (language["fallback"] not in ids or language["fallback"] == language["id"]):
            errors.append(f"{where}: [{i}]: fallback {language['fallback']!r} must be the id of another language")
        if "flag" in language and not (PATT_FLAG.fullmatch(language["flag"]) and (FLAGS_DIR / "svg" / f"{language['flag']}.svg").is_file()):
            available = ", ".join(sorted(path.stem for path in (FLAGS_DIR / "svg").glob("*.svg")))
            errors.append(f"{where}: [{i}]: flag {language['flag']!r} is not in {FLAGS_DIR / 'svg'}; available: {available}")
    if errors:
        raise BuildError("\n".join(errors))

    # A fallback chain must end, otherwise some strings would never resolve.
    fallbacks = {language["id"]: language.get("fallback") for language in languages}
    for start in ids:
        seen = [start]
        while (nxt := fallbacks[seen[-1]]) is not None:
            if nxt in seen:
                raise BuildError(f"{where}: the fallbacks form a loop: {' -> '.join(seen + [nxt])}")
            seen.append(nxt)
    return languages


# ==========================================
# i18n/*.json
# ==========================================

def check_translations(languages: list[dict[str, str]], keys: dict[str, str]) -> list[str]:
    """Validate every `i18n/<id>.json` against index.html. Returns warnings about untranslated keys."""
    source_id = languages[0]["id"]
    expected = {f"{language['id']}.json" for language in languages[1:]}
    present = {path.name for path in I18N_DIR.iterdir()} if I18N_DIR.is_dir() else set()

    errors: list[str] = []
    for name in sorted(present - expected):
        errors.append(f"i18n/{name}: not a registered language (the strings of the source language {source_id!r} live in index.html)")
    for name in sorted(expected - present):
        errors.append(f"i18n/{name}: missing; every registered language except {source_id!r} needs one")

    warnings: list[str] = []
    for name in sorted(expected & present):
        path = I18N_DIR / name
        translations = load_json(path.read_text(encoding="utf-8"), f"i18n/{name}")
        if not isinstance(translations, dict):
            errors.append(f'i18n/{name}: must be an object of `"key": "translation"`')
            continue
        for key, value in translations.items():
            if key not in keys:
                errors.append(f"i18n/{name}: unknown key {key!r}; it is not used in index.html")
            elif not is_nonempty_string(value):
                errors.append(f"i18n/{name}: {key!r}: the translation must be a non-empty string")
        untranslated = [key for key in keys if key not in translations]
        if untranslated:
            warnings.append(f"i18n/{name}: {len(untranslated)} untranslated key(s), shown in the fallback language: {', '.join(untranslated)}")
    if errors:
        raise BuildError("\n".join(errors))
    return warnings


# ==========================================
# screenshots.json
# ==========================================

def check_screenshots(screenshots_dir: Path | None, languages: list[dict[str, str]]) -> list[str]:
    """Validate `homepage/screenshots.json`. If screenshots_dir is given, also check that it
    contains every image the manifest lists. Return the image file names the manifest uses."""
    where = str(SCREENSHOTS_MANIFEST_PATH.relative_to(REPO_ROOT_DIR))
    if not SCREENSHOTS_MANIFEST_PATH.is_file():
        raise BuildError(f"{where}: not found")
    if screenshots_dir is not None and not screenshots_dir.is_dir():
        raise BuildError(f"{screenshots_dir}: not a directory (--screenshots)")
    manifest = load_json(SCREENSHOTS_MANIFEST_PATH.read_text(encoding="utf-8"), where)
    if not isinstance(manifest, dict) or manifest.keys() != {"screenshots"}:
        raise BuildError(f'{where}: must be an object with exactly one key, "screenshots"')
    entries = manifest["screenshots"]
    if not isinstance(entries, list) or len(entries) < 2:
        raise BuildError(f'{where}: "screenshots" must be an array of at least 2 screenshots: the main screenshot, then at least one for the Screenshots section')

    source_id = languages[0]["id"]
    language_ids = {language["id"] for language in languages}
    errors: list[str] = []

    def localized_values(value: Any, field_where: str) -> list[str]:
        """A localizable value is a string, or an object `{language id: string}` including the source language."""
        if isinstance(value, str):
            values = [value]
        elif isinstance(value, dict):
            for language_id in sorted(value.keys() - language_ids):
                errors.append(f"{field_where}: unknown language {language_id!r}; registered: {', '.join(sorted(language_ids))}")
            if source_id not in value:
                errors.append(f"{field_where}: must include the source language {source_id!r}")
            values = list(value.values())
        else:
            errors.append(f'{field_where}: must be a string, or an object like {{"{source_id}": "..."}}')
            return []
        if not all(is_nonempty_string(v) for v in values):
            errors.append(f"{field_where}: must not be empty")
            return []
        return values

    images: list[str] = []
    for i, entry in enumerate(entries):
        entry_where = f"{where}: screenshots[{i}]"
        if not isinstance(entry, dict) or entry.keys() != {"image", "caption"}:
            errors.append(f'{entry_where}: must be an object with exactly the keys "image" and "caption"')
            continue
        localized_values(entry["caption"], f"{entry_where}.caption")
        for name in localized_values(entry["image"], f"{entry_where}.image"):
            if not PATT_IMAGE_NAME.fullmatch(name):
                errors.append(f"{entry_where}.image: invalid file name {name!r}; use letters, digits, `.`, `_`, `-` and one of the extensions png, jpg, jpeg, webp, avif, gif, webm")
            elif screenshots_dir is not None and ((screenshots_dir / name).is_symlink() or not (screenshots_dir / name).is_file()):
                errors.append(f"{entry_where}.image: {name!r} is not a regular file in {screenshots_dir}; push the image to the wiki first")
            else:
                images.append(name)
    if errors:
        raise BuildError("\n".join(errors))
    return sorted(set(images))


# ==========================================
# The image of link previews
# ==========================================

def og_image_png() -> bytes:
    """Convert OG_IMAGE_SVG into a PNG with a white background. The result must have the size OG_IMAGE_SIZE."""
    if (SRC_DIR / OG_IMAGE_PNG).exists():
        raise BuildError(f"{SRC_DIR / OG_IMAGE_PNG}: must not exist; the build writes it from {OG_IMAGE_SVG.name}")
    if shutil.which("rsvg-convert") is None:
        raise BuildError(f"rsvg-convert was not found. It converts {OG_IMAGE_SVG.relative_to(REPO_ROOT_DIR)} into the link-preview PNG. "
                         "Install it: librsvg2-bin (Debian, Ubuntu), rsvg-convert (openSUSE), librsvg2-tools (Fedora), librsvg (Arch Linux).")
    result = subprocess.run(["rsvg-convert", "--format=png", "--background-color=white", str(OG_IMAGE_SVG)], capture_output=True)
    if result.returncode != 0 or not result.stdout.startswith(b"\x89PNG"):
        raise BuildError(f"{OG_IMAGE_SVG}: rsvg-convert failed: {result.stderr.decode(errors='replace').strip()}")
    size = struct.unpack(">II", result.stdout[16:24])   # the width and height in the PNG header (IHDR)
    if size != OG_IMAGE_SIZE:
        raise BuildError(f"{OG_IMAGE_SVG}: the rendered image is {size[0]}x{size[1]} pixels, not {OG_IMAGE_SIZE[0]}x{OG_IMAGE_SIZE[1]}. "
                         "For another size, change OG_IMAGE_SIZE in build.py, and og:image:width and og:image:height in index.html.")
    return result.stdout


# ==========================================
# The site
# ==========================================

def site_files(screenshots_dir: Path | None, images: list[str], languages: list[dict[str, str]]) -> dict[str, Path]:
    """Return every file of the site, as a mapping from its path in the site to the file it is copied from."""
    files: dict[str, Path] = {path.relative_to(SRC_DIR).as_posix(): path for path in sorted(SRC_DIR.rglob("*")) if path.is_file()}
    copied: dict[str, Path] = dict(APP_ICONS)
    flags = sorted({language["flag"] for language in languages if "flag" in language})
    for flag in flags:
        copied[f"images/flags/{flag}.svg"] = FLAGS_DIR / "svg" / f"{flag}.svg"
    if flags:
        copied["images/flags/README.md"] = FLAGS_DIR / "README.md"   # MIT: the notice goes with the copies
    errors: list[str] = []
    for dest, src in copied.items():
        if dest in files:
            errors.append(f"{files[dest]}: conflicts with {src}, copied to the same place at build time")
        elif not src.is_file():
            errors.append(f"{src}: not found")
        files[dest] = src
    if any(path.startswith(f"{SCREENSHOTS_SUBDIR}/") for path in files):
        errors.append(f"{SRC_DIR / SCREENSHOTS_SUBDIR}: must not exist; screenshots come from --screenshots")
    if errors:
        raise BuildError("\n".join(errors))
    if screenshots_dir is not None:
        files[f"{SCREENSHOTS_SUBDIR}/{SCREENSHOTS_MANIFEST}"] = SCREENSHOTS_MANIFEST_PATH
        for name in images:
            files[f"{SCREENSHOTS_SUBDIR}/{name}"] = screenshots_dir / name
    return files


def check_site(screenshots_dir: Path | None) -> tuple[dict[str, Path | bytes], list[str]]:
    """Check everything the site is made from. Return its files and the warnings to print. Each file is either
    the path to copy it from (from site_files), or its content (the PNG from og_image_png)."""
    page = PageParser()
    page.feed((SRC_DIR / "index.html").read_text(encoding="utf-8"))
    page.close()
    if page.errors:
        raise BuildError("\n".join(page.errors))
    languages = check_languages(page.languages_json)
    warnings = check_translations(languages, page.keys)
    images = check_screenshots(screenshots_dir, languages)   # screenshots.json is checked even without --screenshots
    if screenshots_dir is None:
        warnings.append("Without screenshots (--no-screenshots): fine for a preview, never publish it.")
    files: dict[str, Path | bytes] = {**site_files(screenshots_dir, images, languages), OG_IMAGE_PNG: og_image_png()}
    return files, warnings


def check_out_dir(out_dir: Path) -> None:
    if out_dir.is_relative_to(SRC_DIR):
        raise BuildError(f"{out_dir}: the output directory must not be inside {SRC_DIR}")
    if out_dir.exists() and (not out_dir.is_dir() or any(out_dir.iterdir())):
        raise BuildError(f"{out_dir}: already exists and is not an empty directory; remove it first")


def write_site(out_dir: Path, files: dict[str, Path | bytes]) -> None:
    for path, source in files.items():
        (out_dir / path).parent.mkdir(parents=True, exist_ok=True)
        if isinstance(source, bytes):
            (out_dir / path).write_bytes(source)
        else:
            shutil.copyfile(source, out_dir / path)


def build_into_temp_dir(screenshots_dir: Path | None) -> tuple[Path, list[str]]:
    """Check the site with check_site(), then write it into a new temporary folder. Return that folder and the warnings."""
    files, warnings = check_site(screenshots_dir)
    site_dir = Path(tempfile.mkdtemp(prefix="qkiss-homepage-"))
    try:
        write_site(site_dir, files)
    except BaseException:
        shutil.rmtree(site_dir, ignore_errors=True)
        raise
    return site_dir, warnings


# ==========================================
# The dev server (--serve)
# ==========================================

def watched_sources(screenshots_dir: Path | None) -> dict[Path, tuple[int, int]]:
    """The files the site is made from, each with its (mtime, size): everything under
    `src/`, `screenshots.json`, the APP_ICONS, the flags, the files of the screenshots
    folder, and this script."""
    paths = [*SRC_DIR.rglob("*"), SCREENSHOTS_MANIFEST_PATH, *APP_ICONS.values(), *FLAGS_DIR.rglob("*"), SCRIPT_PATH]
    if screenshots_dir is not None and screenshots_dir.is_dir():
        paths += screenshots_dir.iterdir()
    found: dict[Path, tuple[int, int]] = {}
    for path in paths:
        try:
            st = path.stat()
        except OSError:   # gone between the listing and here
            continue
        if stat.S_ISREG(st.st_mode):
            found[path] = (st.st_mtime_ns, st.st_size)
    return found


def watch(screenshots_dir: Path | None, rebuild: Callable[[list[Path]], None],
          before: dict[Path, tuple[int, int]], interval: float = 1.0) -> None:
    """Poll watched_sources() every `interval` seconds, forever, starting from `before`.
    When something changes, keep polling until one poll finds no new change, because
    editors often save a file in several steps. Then call rebuild() with the files that
    were modified, added or removed."""
    while True:
        time.sleep(interval)
        now = watched_sources(screenshots_dir)
        if now == before:
            continue
        while True:
            time.sleep(interval)
            again = watched_sources(screenshots_dir)
            if again == now:
                break
            now = again
        rebuild(sorted(path for path in before.keys() | now.keys() if before.get(path) != now.get(path)))
        before = now


def lan_ips() -> list[str]:
    """Addresses of the interfaces this machine would use towards each private range
    and the internet, without duplicates (as tango-remixer's viewer/server.py finds
    them). Nothing is sent: connect() on a UDP socket only picks the source address.
    Empty when offline."""
    found: list[str] = []
    for probe in ("10.255.255.255", "192.168.255.255", "172.31.255.255", "1.1.1.1"):
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            try:
                s.connect((probe, 1))
            except OSError:
                continue
            ip: str = s.getsockname()[0]
            if ip not in found and ip != "127.0.0.1":
                found.append(ip)
    return found


class _PreviewServer(ThreadingHTTPServer):
    site_dir: Path   # the build being served; replaced by each rebuild


class _PreviewHandler(SimpleHTTPRequestHandler):
    """Serve the files of the current build. The browser must check for a newer version on every load, and no request is logged."""

    def __init__(self, request: Any, client_address: Any, server: _PreviewServer) -> None:
        super().__init__(request, client_address, server, directory=str(server.site_dir))

    def end_headers(self) -> None:
        self.send_header("Cache-Control", "no-cache")
        super().end_headers()

    def log_message(self, format: str, *args: Any) -> None:
        pass


def serve(port: int, screenshots_dir: Path | None) -> None:
    """Build the site, serve it, and rebuild it when its sources change (details in the module docstring). Return when Ctrl-C is pressed."""
    sources = watched_sources(screenshots_dir)   # before building: a change made while it builds is not missed
    site_dir, warnings = build_into_temp_dir(screenshots_dir)
    print_warnings(warnings)
    try:
        server = _PreviewServer(("0.0.0.0", port), _PreviewHandler)
    except OSError as e:
        shutil.rmtree(site_dir, ignore_errors=True)
        sys.exit(f"Cannot listen on port {port} ({e.strerror}). Is another server still running? Stop it first.")
    server.site_dir = site_dir
    restart = threading.Event()

    def rebuild(changed: list[Path]) -> None:
        names = ", ".join(str(path.relative_to(REPO_ROOT_DIR)) if path.is_relative_to(REPO_ROOT_DIR) else str(path)
                          for path in changed[:3]) + (", …" if len(changed) > 3 else "")
        if SCRIPT_PATH in changed:   # this process runs the code it started with: start over with the new one
            print(f"{names} changed: restarting to run the new code.", flush=True)
            restart.set()
            server.shutdown()   # serve_forever() returns; the main thread cleans up, then restarts
            return
        try:
            new_dir, warnings = build_into_temp_dir(screenshots_dir)
        except (BuildError, OSError) as e:
            print(f"❌ {names} changed, but the website was not rebuilt; still serving the last good build:\n{e}", file=sys.stderr, flush=True)
            return
        old_dir, server.site_dir = server.site_dir, new_dir
        shutil.rmtree(old_dir, ignore_errors=True)
        print_warnings(warnings)
        print(f"✅ {names} changed: rebuilt (reload the page).", flush=True)

    threading.Thread(target=watch, args=(screenshots_dir, rebuild, sources), daemon=True).start()
    urls = [f"http://{host}:{port}/" for host in ["127.0.0.1", *lan_ips()]]
    print("Website listening on all interfaces (Ctrl-C to stop). Open one of:\n  " + "\n  ".join(urls)
          + "\nWatching homepage/src/, homepage/screenshots.json, the app icons, the flags" + (" and the screenshot images" if screenshots_dir else "")
          + ": a change rebuilds the site (reload the page); a change to build.py restarts this server.", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
        shutil.rmtree(server.site_dir, ignore_errors=True)
    if restart.is_set():
        os.execv(sys.executable, [sys.executable, *sys.argv])


def print_warnings(warnings: list[str]) -> None:
    for warning in warnings:
        print(f"⚠️  {warning}", flush=True)


def main() -> int:
    parser = argparse.ArgumentParser(description="Check and assemble the QKiSS website (see the docstring of homepage/build.py).")
    parser.add_argument("out_dir", metavar="OUT_DIR", nargs="?", type=Path, help="Write the site here. It must not exist yet, or be empty.")
    parser.add_argument("--serve", metavar="PORT", nargs="?", const=8000, type=int, help="Serve the site on PORT (default: 8000) on every network interface, instead of writing it to OUT_DIR, and rebuild it when its sources change.")
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--screenshots", metavar="DIR", type=Path, help="Directory of the images listed in homepage/screenshots.json (the screenshots/ folder of a clone of the wiki).")
    group.add_argument("--no-screenshots", action="store_true", help="Build without screenshots. For a preview only; never publish it.")
    args = parser.parse_args()
    if (args.out_dir is None) == (args.serve is None):
        parser.error("give either OUT_DIR, or --serve [PORT]")

    screenshots_dir: Path | None = None if args.no_screenshots else args.screenshots.resolve()
    try:
        if args.serve is not None:
            serve(args.serve, screenshots_dir)
            return 0
        out_dir: Path = args.out_dir.resolve()
        files, warnings = check_site(screenshots_dir)
        check_out_dir(out_dir)
        write_site(out_dir, files)
    except BuildError as e:
        print(f"❌ The website was not built:\n{e}", file=sys.stderr)
        return 1
    print_warnings(warnings)
    print(f"✅ Website built into {out_dir}: {len(files)} files" + (f", screenshots from {screenshots_dir}" if screenshots_dir else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
