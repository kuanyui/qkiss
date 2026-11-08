#!/usr/bin/env python3
"""
Checks the AppDir of the AppImage, and writes its license files. See `.github/workflows/linux-appimage.yaml`.

    python3 -I packaging/linux/appdir_tool.py licenses APPDIR --qt-dir QT_DIR --sources SOURCES_DIR ...
    python3 -I packaging/linux/appdir_tool.py set-runpath APPDIR
    python3 -I packaging/linux/appdir_tool.py check APPDIR --max-glibc 2.39 --max-glibcxx 3.4.33

Run the three commands in this order. `licenses` finds the package of each library of Ubuntu by comparing it with
the files of the system, so it must run before `set-runpath` changes the libraries.

Each command prints all the problems that it finds, then fails if it found any.
"""

import argparse
import filecmp
import os
import re
import shutil
import subprocess
import sys
import tarfile
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
HOST_LIBRARIES_FILE = SCRIPT_DIR / "appimage-host-libraries.txt"
QKISS_LICENSE_FILE = SCRIPT_DIR.parent.parent / "LICENSE"

# The program loader of x86_64 Linux. A program that asks for another one does not start on other systems.
PROGRAM_LOADER = "/lib64/ld-linux-x86-64.so.2"
# lha and unar come with their own license files and source code, and their own libraries (see CMakeLists.txt).
HELPERS_DIR = "usr/lib/qkiss/helpers"

# The license of each package of Ubuntu that the AppImage takes libraries from, from the `License:` fields of the
# copyright file of the package. qkiss is GPL-3.0-or-later, so none of them can be GPL-2.0-only.
# A package that is not here fails `licenses`, which then prints the fields to add it.
UBUNTU_LICENSES = {
}

problems = []


def problem(message):
    problems.append(message)
    # The escapes of a GitHub annotation: `%0A` is a line end.
    escaped = message.replace("%", "%25").replace("\r", "%0D").replace("\n", "%0A")
    print(f"::error::{escaped}", flush=True)


def finish():
    if problems:
        sys.stdout.flush()
        print(f"{len(problems)} problem(s), see above.", file=sys.stderr)
        sys.exit(1)


def run(*command):
    # LC_ALL=C: the output of readelf and dpkg-query is parsed below.
    env = dict(os.environ, LC_ALL="C")
    return subprocess.run(command, check=True, capture_output=True, text=True, env=env).stdout


def is_elf(path):
    with open(path, "rb") as f:
        return f.read(4) == b"\x7fELF"


def elf_files(root):
    """The ELF files under `root`, but not the symbolic links to them."""
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames.sort()
        for name in sorted(filenames):
            path = Path(dirpath, name)
            if not path.is_symlink() and path.is_file() and is_elf(path):
                yield path


def is_inside(path, root):
    return Path(os.path.realpath(path)).is_relative_to(Path(os.path.realpath(root)))


def read_host_libraries():
    lines = HOST_LIBRARIES_FILE.read_text(encoding="utf-8").splitlines()
    return {line.strip() for line in lines if line.strip() and not line.lstrip().startswith("#")}


def dynamic_entries(path):
    """For example {"NEEDED": ["libQt6Core.so.6", ...], "RUNPATH": ["$ORIGIN/../lib"]}."""
    entries = {}
    # A line looks like ` 0x0000000000000001 (NEEDED)             Shared library: [libQt6Core.so.6]`.
    for match in re.finditer(r"^\s*0x[0-9a-f]+ \((\w+)\)\s+[^\[\n]*\[(.*)\]$", run("readelf", "-W", "-d", str(path)), re.M):
        tag, value = match.groups()
        if tag in ("RUNPATH", "RPATH"):
            entries.setdefault(tag, []).extend(value.split(":"))
        else:
            entries.setdefault(tag, []).append(value)
    return entries


def needed_symbol_versions(path):
    """The versions of the symbols that `path` needs from other libraries, for example {"GLIBC_2.34", "GLIBCXX_3.4.29"}."""
    versions = set()
    in_needs = False
    for line in run("readelf", "-W", "-V", str(path)).splitlines():
        if line.startswith("Version "):
            in_needs = line.startswith("Version needs section")
        elif in_needs:
            # A line looks like `  0x0010:   Name: GLIBC_2.34  Flags: none  Version: 3`.
            match = re.match(r"^\s*0x[0-9a-f]+:\s+Name: (\S+)", line)
            if match:
                versions.add(match.group(1))
    return versions


def version_tuple(text):
    return tuple(int(part) for part in text.split("."))


# ======================================================================
# check
# ======================================================================

def check(args):
    appdir = Path(args.appdir).resolve()
    host_libraries = read_host_libraries()
    limits = {"GLIBC": args.max_glibc, "GLIBCXX": args.max_glibcxx}
    newest = {}
    used_host_libraries = set()
    count = 0

    for dirpath, dirnames, filenames in os.walk(appdir):
        for name in filenames + dirnames:
            path = Path(dirpath, name)
            if path.is_symlink() and not (path.exists() and is_inside(path, appdir)):
                problem(f"{path.relative_to(appdir)} is a symbolic link to {os.readlink(path)}, which is not in the AppDir")

    for path in elf_files(appdir):
        count += 1
        relative = path.relative_to(appdir)
        header = run("readelf", "-W", "-h", str(path))
        if not re.search(r"^\s*Class:\s+ELF64$", header, re.M) or not re.search(r"^\s*Machine:\s+Advanced Micro Devices X86-64$", header, re.M):
            problem(f"{relative} is not x86_64 code")
            continue

        match = re.search(r"Requesting program interpreter: (.+)\]$", run("readelf", "-W", "-l", str(path)), re.M)
        if match and match.group(1) != PROGRAM_LOADER:
            problem(f"{relative} asks for the program loader {match.group(1)}, not {PROGRAM_LOADER}")

        entries = dynamic_entries(path)
        # The users have their own versions of these libraries. A copy in the AppDir would be loaded instead.
        names = {path.name, *entries.get("SONAME", [])}
        if names & host_libraries and not relative.is_relative_to(HELPERS_DIR):
            problem(f"{relative} must not be in the AppDir: it is in {HOST_LIBRARIES_FILE.name}")
        # RPATH also applies to the libraries that the libraries load, which RUNPATH does not.
        if "RPATH" in entries:
            problem(f"{relative} has an RPATH: {':'.join(entries['RPATH'])}. Only RUNPATH is allowed")
        search_dirs = []
        for entry in entries.get("RUNPATH", []):
            directory = Path(os.path.normpath(re.sub(r"\$(ORIGIN|\{ORIGIN\})", str(path.parent), entry)))
            if not entry.startswith(("$ORIGIN", "${ORIGIN}")) or not is_inside(directory, appdir):
                problem(f"{relative} has the RUNPATH {entry}, which is not in the AppDir")
            else:
                search_dirs.append(directory)
        for needed in entries.get("NEEDED", []):
            # The loader takes a name with `/` as a path, which RUNPATH does not limit.
            if "/" in needed:
                problem(f"{relative} needs the path {needed}, not a library name")
                continue
            if any((directory / needed).is_file() for directory in search_dirs):
                continue
            if needed in host_libraries:
                used_host_libraries.add(needed)
            else:
                problem(f"{relative} needs {needed}, which is neither in its RUNPATH ({':'.join(entries.get('RUNPATH', [])) or 'none'}) nor in {HOST_LIBRARIES_FILE.name}")

        for version in needed_symbol_versions(path):
            prefix, _, number = version.partition("_")
            if prefix not in limits:
                continue
            if not re.fullmatch(r"\d+(\.\d+)+", number):
                # For example GLIBC_PRIVATE, which only the glibc that it was built with has.
                problem(f"{relative} needs {version}")
            elif version_tuple(number) > version_tuple(limits[prefix]):
                problem(f"{relative} needs {version}, newer than {prefix} {limits[prefix]}")
            elif prefix not in newest or version_tuple(number) > version_tuple(newest[prefix]):
                newest[prefix] = number

    print(f"Checked {count} ELF files.")
    for prefix in limits:
        print(f"The newest {prefix} that the ELF files need: {newest.get(prefix, 'none')} (at most {limits[prefix]})")
    print("Libraries of the system that the ELF files need:", " ".join(sorted(used_host_libraries)))
    if count < 3:
        problem(f"Only {count} ELF files in {appdir}. Did `cmake --install` run?")
    finish()


# ======================================================================
# set-runpath
# ======================================================================

def set_runpath(args):
    """Adds the RUNPATH `$ORIGIN` to the copied libraries of Ubuntu that have none. Without it, such a library would load
    the libraries that it needs from the system, not from `usr/lib/`."""
    lib_dir = Path(args.appdir).resolve() / "usr/lib"
    for path in elf_files(lib_dir):
        if path.parent != lib_dir:
            continue
        entries = dynamic_entries(path)
        if "RUNPATH" in entries or "RPATH" in entries:
            continue
        if any("/" not in needed and (lib_dir / needed).is_file() for needed in entries.get("NEEDED", [])):
            print(f"RUNPATH $ORIGIN: usr/lib/{path.name}")
            run("patchelf", "--set-rpath", "$ORIGIN", str(path))
    finish()


# ======================================================================
# licenses
# ======================================================================

def safe_member_path(name):
    """The path of a file in a source archive, without the top folder (for example `qtbase-everywhere-src-6.10.3/`)."""
    parts = Path(name).parts
    if len(parts) < 2 or Path(name).is_absolute() or ".." in parts:
        return None
    return Path(*parts[1:])


def copy_from_archive(archive, wanted, destination):
    """Copies the files of the archive for which `wanted(path)` is true. Returns their paths."""
    copied = []
    with tarfile.open(archive) as tar:
        for member in tar:
            path = safe_member_path(member.name)
            if member.isfile() and path is not None and wanted(path):
                target = destination / path
                target.parent.mkdir(parents=True, exist_ok=True)
                with tar.extractfile(member) as source, open(target, "wb") as f:
                    shutil.copyfileobj(source, f)
                copied.append(path)
    return copied


def find_ubuntu_package(path):
    """The package of Ubuntu that has the same file as `path`: (system file, package, version, source package, source version)."""
    for directory in ("/usr/lib/x86_64-linux-gnu", "/lib/x86_64-linux-gnu", "/usr/lib", "/lib"):
        system_file = Path(directory, path.name)
        if not system_file.is_file() or not filecmp.cmp(system_file, path, shallow=False):
            continue
        # Ubuntu moved the files from /lib to /usr/lib, so dpkg may know the file by either path.
        for name in {str(system_file), str(system_file).removeprefix("/usr"), "/usr" + str(system_file).removeprefix("/usr")}:
            try:
                output = run("dpkg-query", "-S", name)
            except subprocess.CalledProcessError:
                continue
            # A line looks like `libxkbcommon0:amd64: /usr/lib/x86_64-linux-gnu/libxkbcommon.so.0.0.0`.
            owners = [line.split(": ", 1)[0] for line in output.splitlines() if not line.startswith("diversion ")]
            if len(owners) != 1 or "," in owners[0]:
                problem(f"{name} is in more than one package: {output.strip()}")
                return None
            package = owners[0]
            fields = run("dpkg-query", "-W", "-f", "${Package}\t${Version}\t${source:Package}\t${source:Version}", package).split("\t")
            return (system_file, *fields)
    return None


def licenses(args):
    appdir = Path(args.appdir).resolve()
    usr = appdir / "usr"
    qt_dir = Path(args.qt_dir).resolve()
    sources = Path(args.sources).resolve()
    doc_dir = usr / "share/doc/qkiss"
    out = doc_dir / "third-party"
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    shutil.copyfile(QKISS_LICENSE_FILE, doc_dir / "LICENSE")

    # Qt: the license texts (`LICENSES/`) and the notices of the code of others in Qt (`qt_attribution.json`).
    qt_archives = sorted((sources / "qt").glob("*-everywhere-src-*.tar.xz"))
    qt_versions = {archive.name.split("-everywhere-src-")[1].removesuffix(".tar.xz") for archive in qt_archives}
    if len(qt_versions) != 1:
        problem(f"Expected the source code of one version of Qt in {sources / 'qt'}, found: {[a.name for a in qt_archives]}")
        finish()
    qt_version = qt_versions.pop()
    for archive in qt_archives:
        module = archive.name.split("-everywhere-src-")[0]
        copied = copy_from_archive(archive, lambda p: p.parts[0] == "LICENSES" or p.name == "qt_attribution.json", out / "qt" / module)
        if not any(p.parts[0] == "LICENSES" for p in copied):
            problem(f"{archive.name} has no LICENSES/")
    if not (usr / f"lib/libQt6Core.so.{qt_version}").is_file():
        problem(f"usr/lib/ has no libQt6Core.so.{qt_version}: the Qt in the AppDir is not the version of the source code in {sources / 'qt'}")

    # FFmpeg: its libraries come with Qt Multimedia.
    ffmpeg_archives = sorted((sources / "ffmpeg").glob("FFmpeg-*.tar.gz"))
    match = re.fullmatch(r"FFmpeg-n?(\d+(?:\.\d+)+)\.tar\.gz", ffmpeg_archives[0].name) if len(ffmpeg_archives) == 1 else None
    if not match:
        problem(f"Expected one FFmpeg-VERSION.tar.gz in {sources / 'ffmpeg'}, found: {[a.name for a in ffmpeg_archives]}")
        finish()
    ffmpeg_version = match.group(1)
    copied = copy_from_archive(ffmpeg_archives[0], lambda p: len(p.parts) == 1 and (p.name == "LICENSE.md" or p.name.startswith("COPYING.LGPL")), out / "ffmpeg")
    if not {Path("LICENSE.md"), Path("COPYING.LGPLv2.1")} <= set(copied):
        problem(f"{ffmpeg_archives[0].name} has no LICENSE.md or COPYING.LGPLv2.1")

    (out / "icu").mkdir()
    shutil.copyfile(args.icu_license, out / "icu/LICENSE")

    rows = []
    ubuntu_sources = set()
    unknown_packages = {}
    for path in elf_files(appdir):
        relative = path.relative_to(usr) if path.is_relative_to(usr) else None
        if relative is None:
            problem(f"{path.relative_to(appdir)} is not in usr/")
            continue
        if relative == Path("bin/qkiss") or path.is_relative_to(appdir / HELPERS_DIR):
            continue

        if relative.parts[0] == "plugins":
            # The deployment changes the RUNPATH of the plugins, so only the name is compared.
            if (qt_dir / relative).is_file():
                rows.append((relative, "Qt", qt_version, "LGPL-3.0-only", "third-party/qt/"))
            else:
                problem(f"usr/{relative} is not in {qt_dir}")
            continue
        if relative.parent != Path("lib"):
            problem(f"usr/{relative}: an ELF file in an unexpected folder")
            continue

        qt_file = qt_dir / "lib" / path.name
        if qt_file.is_file():
            if not filecmp.cmp(qt_file, path, shallow=False):
                problem(f"usr/{relative} is not the same as {qt_file}")
            elif path.name.startswith("libQt6"):
                rows.append((relative, "Qt", qt_version, "LGPL-3.0-only", "third-party/qt/"))
            elif re.fullmatch(r"libicu\w+\.so\.(\d+(?:\.\d+)*)", path.name):
                # The LICENSE file is the one of this version of ICU (see the workflow).
                if not path.name.endswith(f".so.{args.icu_version}"):
                    problem(f"usr/{relative} is not ICU {args.icu_version}")
                rows.append((relative, "ICU", args.icu_version, "ICU", "third-party/icu/"))
            elif re.match(r"lib(av|sw)\w+\.so\.", path.name):
                data = path.read_bytes()
                # Each library of FFmpeg has the strings `FFmpeg version VERSION` and the license of the build.
                # `--enable-gpl` would make it GPL, and `--enable-nonfree` not distributable at all.
                if not re.search(rb"FFmpeg version n?" + re.escape(ffmpeg_version.encode()) + rb"(?![.\d])", data):
                    problem(f"usr/{relative} is not FFmpeg {ffmpeg_version}, the version of the source code in {sources / 'ffmpeg'}")
                if b"LGPL version 2.1 or later" not in data or re.search(rb"(?<!L)GPL version [23] or later|nonfree and unredistributable", data):
                    problem(f"usr/{relative} is not a build of FFmpeg under LGPL-2.1-or-later")
                rows.append((relative, "FFmpeg", ffmpeg_version, "LGPL-2.1-or-later", "third-party/ffmpeg/"))
            else:
                problem(f"usr/{relative} is from Qt, but it is not a library of Qt, ICU or FFmpeg")
            continue

        found = find_ubuntu_package(path)
        if found is None:
            problem(f"usr/{relative} is neither in {qt_dir / 'lib'} nor the same as a file of a package of Ubuntu")
            continue
        system_file, package, version, source_package, source_version = found
        copyright_file = Path("/usr/share/doc", package, "copyright")
        if not copyright_file.is_file():
            problem(f"{package} (the package of {system_file}) has no {copyright_file}")
            continue
        target = out / "ubuntu" / package / "copyright"
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(copyright_file, target)
        text = copyright_file.read_text(encoding="utf-8", errors="replace")
        # Many copyright files only name the license texts in /usr/share/common-licenses/.
        for name in sorted(set(re.findall(r"/usr/share/common-licenses/([A-Za-z0-9][\w.+-]*[\w+])", text))):
            license_file = Path("/usr/share/common-licenses", name)
            if license_file.is_file():
                (out / "ubuntu/common-licenses").mkdir(parents=True, exist_ok=True)
                shutil.copyfile(license_file, out / "ubuntu/common-licenses" / name)
            else:
                problem(f"The copyright file of {package} names {license_file}, which does not exist")
        license_name = UBUNTU_LICENSES.get(package)
        if license_name is None:
            fields = sorted(set(re.findall(r"^License:\s*(.+?)\s*$", text, re.M)))
            unknown_packages[package] = f"{package} (source {source_package}): License: fields: {' | '.join(fields) or 'none'}"
            license_name = "?"
        rows.append((relative, package, version, license_name, f"third-party/ubuntu/{package}/"))
        ubuntu_sources.add(f"{source_package}={source_version}")

    if unknown_packages:
        # One annotation with all the packages.
        problem(f"Add these packages to UBUNTU_LICENSES in {Path(__file__).name}:\n" + "\n".join(unknown_packages.values()))

    # The libraries of vcpkg are static: they are in bin/qkiss.
    vcpkg_rows = []
    vcpkg_installed = Path(args.vcpkg_installed).resolve()
    triplet = vcpkg_installed.name
    for listing in sorted((vcpkg_installed.parent / "vcpkg/info").glob(f"*_{triplet}.list")):
        port, _, version = listing.name.removesuffix(f"_{triplet}.list").partition("_")
        has_library = any(re.fullmatch(rf"{re.escape(triplet)}/lib/[^/]+\.a", line.strip()) for line in listing.read_text().splitlines())
        copyright_file = vcpkg_installed / "share" / port / "copyright"
        if has_library:
            if not copyright_file.is_file():
                problem(f"The vcpkg port {port} has no {copyright_file}")
                continue
            (out / "vcpkg" / port).mkdir(parents=True)
            shutil.copyfile(copyright_file, out / "vcpkg" / port / "copyright")
            vcpkg_rows.append((Path("bin/qkiss"), port, version, "see the file", f"third-party/vcpkg/{port}/"))

    Path(args.ubuntu_sources_list).write_text("".join(f"{s}\n" for s in sorted(ubuntu_sources)), encoding="utf-8")

    table = [("File in usr/", "Component", "Version", "License", "License texts and notices")] + rows + vcpkg_rows
    widths = [max(len(str(row[i])) for row in table) for i in range(4)]
    lines = ["  ".join(str(cell).ljust(width) for cell, width in zip(row, widths + [0])).rstrip() for row in table]
    (doc_dir / "THIRD-PARTY.txt").write_text(f"""\
qkiss is under the GNU General Public License, version 3 or later: see LICENSE next to this file.
This AppImage also has the programs and libraries of others, listed below.

- The last column is the folder with the license texts and the notices of each component, next to this file.
- The source code of these components is in {args.source_archive}, on the release page of this
  version of qkiss: https://github.com/kuanyui/qkiss/releases
- Qt and FFmpeg are linked dynamically. To use your own build of Qt or FFmpeg, extract the AppImage with
  `--appimage-extract`. Then replace the files in usr/lib/ and usr/plugins/.
- lha and unar are in usr/lib/qkiss/helpers/. Each of the two folders has the license files of its program.
  The folder of lha also has the source code of lha.
  The source code of unar is unar-VERSION-src.tar.xz, on the same release page.
- The libraries of vcpkg are linked statically into bin/qkiss.

{chr(10).join(lines)}
""", encoding="utf-8")
    print((doc_dir / "THIRD-PARTY.txt").read_text(encoding="utf-8"))
    finish()


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)

    p = commands.add_parser("check", help="Check every ELF file of the AppDir")
    p.add_argument("appdir")
    p.add_argument("--max-glibc", required=True, help="The glibc of the oldest supported system, for example 2.39")
    p.add_argument("--max-glibcxx", required=True, help="The GLIBCXX of the libstdc++ of that system, for example 3.4.33")
    p.set_defaults(function=check)

    p = commands.add_parser("set-runpath", help="Set the RUNPATH of the libraries of Ubuntu in usr/lib/ to $ORIGIN")
    p.add_argument("appdir")
    p.set_defaults(function=set_runpath)

    p = commands.add_parser("licenses", help="Write usr/share/doc/qkiss/: LICENSE, THIRD-PARTY.txt and third-party/")
    p.add_argument("appdir")
    p.add_argument("--qt-dir", required=True, help="The Qt that the AppDir has its Qt from, for example Qt/6.10.3/gcc_64")
    p.add_argument("--sources", required=True, help="Folder with qt/*-everywhere-src-VERSION.tar.xz and ffmpeg/FFmpeg-VERSION.tar.gz")
    p.add_argument("--icu-license", required=True, help="The LICENSE file of ICU")
    p.add_argument("--icu-version", required=True, help="The version of ICU of that LICENSE file, for example 73.2")
    p.add_argument("--vcpkg-installed", required=True, help="For example vcpkg_installed/x64-linux")
    p.add_argument("--source-archive", required=True, help="The file name of the source code of the third-party components on the release page")
    p.add_argument("--ubuntu-sources-list", required=True, help="Output: the source packages of Ubuntu, one `NAME=VERSION` per line, for `apt-get source`")
    p.set_defaults(function=licenses)

    args = parser.parse_args()
    args.function(args)


if __name__ == "__main__":
    main()
