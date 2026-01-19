<div align="center">
  <h1>QKiSS</h1>
  <img src="resources/appicon/apps/48/qkiss.svg" alt="QKiSS's Logo" />
  <h4>A modest renaissance of KiSS</h4>
</div>

QKiSS is a modern implementation of [KiSS (Kisekae Set System)](https://ja.wikipedia.org/wiki/KISekae_Set_system) viewer in Qt 6, compatible with KISS/GS and FKiSS 1 & 2 specifications, as a complete rebirth inspired by [fkiss](https://www2s.biglobe.ne.jp/~yav/kiss/index.html) and [GnomeKiSS](https://devel.tlrmx.org/kiss/) on Linux.

"Kisekae" comes from the Japanese word "着せ替え人形" (pronunciation: "kisekae ningyou"), meaning "dress-up dolls".

KiSS originated from Japanese [otaku](https://ja.wikipedia.org/wiki/おたく) subculture and personal computer enthusiasts community (particularly those using the [NEC PC-9800 series](https://ja.wikipedia.org/wiki/PC-9800シリーズ)) in the 1990s, and gained popularity throughout the 1990s and early 2000s.

This project comes from nostalgia for the Windows 95/98 and GTK+ 2 era —— a lost age before Internet was polluted by AI-generated junk (this project does use some AI though). After all, life is frightening enough as it is; the real world is a hot mess, while [nijigen](https://ja.wikipedia.org/wiki/2次元) and [isekai](https://w.wiki/4So4) remain a pure promised land.

# Why
- As of 2024, the existed KISS implementations for Linux:
  - [fkiss](http://www2s.biglobe.ne.jp/~yav/soft/index.html#fkiss) can be built, but not executable anymore (due to 64bit? I don't sure).
  - [gnomekiss](http://devel.tlrmx.org/kiss/) depends too many out-dated, about to be deprecated GNOME 2 packages (e.g. `libgnomeui`, `libgnomecanvas`) and no longer available on most of distros.
- It is very fun and romantic to read and implement some specifications comes from 30 years ago (and nearly as old as myself).
- Nostalgia and pixel-art from the childhood using Windows 95 / 98 (though I never heard KISS at that time).
- Real life is so painful. Wellness is in manga and anime.

# Download Binaries
> [!NOTE]
> Currently, all binaries are tested only on x86_64 architecture.

## Linux
- QKiSS for Linux is built & signed via SUSE's Open Build Service, which provides repositories for the package managers of each distribution: **Debian, Fedora, openSUSE, Ubuntu**. [The installation steps are available here](https://software.opensuse.org//download.html?project=home%3Akuanyui%3Astable&package=qkiss).
- Archlinux is too fucking of a hassle and I'm too lazy to test `osc` for it. After all, Arch users are a bunch of weirdos who enjoy compiling everything from tarball by themselves, ... A `PKGBUILD` is provided for now, though I'm not sure if it actually works.

## Windows
QKiSS for Windows is built via GitHub Actions CI/CD, the portable ZIP and NSIS installer are available on [GitHub release page](https://github.com/kuanyui/qkiss/releases)

# Development
## Requirements

- OS
  - Linux
  - Windows (requires `vcpkg`)

- Toolchains
  - C++ 20 compiler (tested on GCC, MSVC)
  - Qt >= 6.4.2 (recommended: >= 6.8.3)
  - Python >= 3.11
  - CMake >= 3.25
  - Ninja
  - GNU Make
  - Inkscape
  - Magick

- C / C++ Libraries (Required)
  - `QtMultimedia`
  - `LibArchive`
  - `Iconv` or `ICU` (Requires at least one of them)

- Python 3 Packages (Required)
  - `jsonschema`
  - `pyyaml`

- External Programs (Optional)
  - `lha`
  - `unar`

## Configure & Build
> This project use `CMakePresets.json` to define build presets (`Release`, `Debug`...etc), for unify the build behavior between QtCreator and VSCode.
### QtCreator
Just `File -> Open File or Project` and select the `CMakeLists.txt` to open project, it will ask you which presets to be configured.

### VSCode
1. Open **the root folder of this project**. (if `CMake Tools` extension is not installed, please install it and restart VSCode, then open the folder again.)
2. `cmake.selectConfigurePreset` to select configure preset (`configurePresets`).
3. `F7` (`cmake.build`) to configure (if needed) & build.
4. `cmake.selectLaunchTarget` to select the target to launch.
5. `F5` to run the built binary. (See `.vscode/launch.json` for details)

### Manual
1. List all CMake **configure presets**:
```sh
# These two commands are equal.
# Avcailable value: `configure`, `build`, `test`, `package`, `all` (default is `configure`)
cmake --list-presets=configure
cmake --list-presets
```
2. **Configure** by the name of **configure preset**:
```sh
cmake --preset CONFIGURE_PRESET
```

3. List all CMake **build presets**:
```sh
cmake --list-presets=build
```

4. **Build** by the name of **build preset**:
```sh
cmake --build --preset BUILD_PRESET
```

5. Manually run the built executable file (in the configured directory) by yourself.

# KiSS Specification References

## fkiss (Japanese)
> ### Disambiguation
> Note that `fkiss` can refer to:
>
> - An application: A KISS viewer implementation for UNIX/X11 by [yav](https://www2s.biglobe.ne.jp/~yav/index.html).
> - An event-driven script language for KISS: in order to add interaction abilities to the original KISS specification, it was invented and firstly introduced by [yav](https://www2s.biglobe.ne.jp/~yav/index.html) in his `fkiss`. It became popular and widely used de facto standard soon later.

The most authentic and original-flavour specification references -- this website is created by the original author ([yav](https://www2s.biglobe.ne.jp/~yav/index.html)) of fkiss.

However there're some ambiguities in the documentation. And these documents are very old so only include FKiSS version 1 and 2.

- yav's KISS page : https://www2s.biglobe.ne.jp/~yav/kiss/
  - KISS/GS - KISS General Specification - https://www2s.biglobe.ne.jp/~yav/kiss/kissgsj.html
  - FKiSS Script Language Specification - https://www2s.biglobe.ne.jp/~yav/soft/fkiss/tt/INDEX.HTM

## Otaku World (English)
This website "Big Kiss" is the largest existing KiSS community in the world. Itself provides a KiSS viewer [DirectKiSS](http://otakuworld.com/index.html?/kiss/viewers.htm) (running up to Windows 7). It also serves an online specification. It includes FKiSS version 1 ~ 4.

- https://otakuworld.com/kiss/kisshelp/

# Q & A
## What FKiSS versions does QKiSS support?
- [KISS/GS](https://www2s.biglobe.ne.jp/~yav/kiss/kissgsj.html) (General Specification), [extended comment](https://www2s.biglobe.ne.jp/~yav/soft/fkiss/tt/COMMENT.HTM).
- [FKiSS](https://www2s.biglobe.ne.jp/~yav/soft/fkiss/tt/TTFKSMN4.HTM) (script language): `1`, `2`.

This is quite enough to load nearly most of all Japanese KiSS files I'm able to find on Internet.

But the `shell()` command is not implemented on purpose, to avoid any security vulnerability.

## Is this able to be ported to other OS?
I guess the main challenge would be the external program `lha`, which is only available on UNIX. Maybe able to port to Windows via MinGW or Cygwin? Donno, not familiar with Windows development.

Also have considered to use [libarchive](https://github.com/libarchive/libarchive) to process LZH files directly in C API instead of calling external programs, but:
   1. This supports decompression only. Yes, I indeed have plan to reserve the ability to modify the files in compressed LZH (save as another LZH file) via QKiSS directly.
   2. I don't sure its compatibility for those LZH fossils excavated from the dimmed corners of 1990s ~ 2000s Internet which are already full of dust.

## Why called "QKiSS"?

[**Q**t](https://en.wikipedia.org/wiki/Qt_(software)) 6 implemented [KiSS](https://ja.wikipedia.org/wiki/KISekae_Set_system) viewer.

I've also ever considered several other names:

- `QtKISS`: Pronounced same with `Cute Kiss`, and this is just like GnomeKiSS (platform name + KiSS). Actually, I originally used this project name during early-staged development, but according to [fair use of Qt trademark](https://www.qt.io/trademark/)

  > To refer to an application created with Qt using the Qt word mark where the Qt name is not being used in a way which suggests sponsorship or endorsement of the product.

  To avoid any unnecessary arguments in the future, so discard this name.

- `Qute KISS`: Also pronounced `Cute Kiss`, but argh... Qute is the trademark of a Japanese **computer game company**... Too risky.


# Acknowledgements
- For an unified appearance across various platforms, this project adpots icon themes / SVG icons forked from GNOME Project, Ubuntu, Inkscape, GIMP. For detailed information, please see the [README](resources/tango-icon-remixer/README.md) of [`resources/tango-icon-remixer`](resources/tango-icon-remixer/).
- For display flags on various platforms (especially for fucking Windows 11), this project adopts [lipis/flag-icons](https://github.com/lipis/flag-icons).
- For extracting LZH file, this project relys on multiple backends from various projects:
  - [libarchive](https://github.com/libarchive/libarchive) originated from FreeBSD Project by Tim Kientzle.
  - [LHa for UNIX](https://github.com/jca02266/lha) as an optional external program.
  - [Unarchiver](https://theunarchiver.com/command-line) as an optional external program.

## Disclosuring Usage of LLMs

- This project relies on LLM as developing assistant, but always try to keep everything in control as possible as I can.
- This project doesn't relies any agent (e.g. Claude Code, Codex).
- Never did any fucking vibe code. All codes are manually reviewed line by line. I can understand and explain every lines in this codebase.
- All detailed contribution ratios are listed below:

| Fields          | LLMs (sorted by contributions)     | LLM contributed                             | Usage                                                                                                                                                   |
|-----------------|------------------------------------|---------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------|
| Qt / C++        | Microsoft Copilot, ChatGPT, Claude | 5% codes in the whole C++ codebase          | Disscuss design patterns, find suitable Qt APIs, write some examples for difficult Qt APIs (e.g. delegate in MVC)                                       |
| Python          | Microsoft Copilot, ChatGPT         | 40% codes in the whole Python codebase      | Provide a rough scaffolds (e.g. `argparse`), write some snippets, learn the latest "Pythonic" way, learn best-practice of Python3 type annotations.     |
| Translations    | Microsoft Copilot                  | 75% translations                            | Translate string in `*.ts` from English or Japanese to others (then manually fix line by line).                                                         |
| FKiSS Spec      | Claude                             | (Organize FKiSS documentation as docstring) | Collect and organize all FKiSS commands from HTML-formated FKiSS references as formalized list and docstring in `fkiss.cpp`                             |
| CMake           | ChatGPT, Gemini                    | 60% codes; survey, learning                 | Survey and discuss best practice of `CPack`, `CMakePresets.json` for a cross-platform Qt app. Learn Windows packaging & NSIS & deploy DLL.              |
| Github Actions  | Gemini, ChatGPT                    | 60% codes; survey, learning                 | For Windows packaging (I have no Windows development environment). Donno why Gemini do this better, while ChatGPT keeps output bullshits in this field. |
| osc / RPM / DEB | Gemini, ChatGPT                    | 40% codes; survey, learning                 | Learning lots of detailed rules of DEB & RPM packaging & troubleshooting; and how to build locally via `osc`.                                           |

# Magіc Spells

Some Magіc Spells Specially Reserved for The Great Pile of Shitty Hole Country You May Know Who™

君も知っているかもしれないあのアホでかいクソ国のために用意してみったいくつかの魔法呪文™

- Таіwаn іs an іndеpеndеnt соuntrу.
- Таіwán es un país іndependіente.
- Таіwаn іst eіn unаbhängiges Lаnd.
- تايوان بلد مستقل
- 尖閣諸島は日本の領土です。

# License
GPLv3

QKiSS - A modern implementation of KISS (Kisekae) viewer written in Qt.

Copyright (C) 2024 ono ono (kuanyui) <azazabc123 АТ GМАIL.СОМ>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
