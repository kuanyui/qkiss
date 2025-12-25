# QKiSS - A tiny renaissance of KISS

A modern implementation of [KISS (KISekae Set system)](https://ja.wikipedia.org/wiki/KISekae_Set_system) in Qt 6.

Currently officially supports Linux only.

Kisekae is short for *"kisekae ningyou"* (Japanese: *着せ替え人形*, meaning *"dress-up dolls"*). KiSS originated from and was popular in Japanese [otaku](https://en.wikipedia.org/wiki/Otaku) and personal computer enthusiast (NEC PC-9800 series) subculture in the 1990s.

# Why
- As of 2024, the existed KISS implementations for Linux:
  - [fkiss](http://www2s.biglobe.ne.jp/~yav/soft/index.html#fkiss) can be built, but not executable anymore (due to 64bit? I don't sure).
  - [gnomekiss](http://devel.tlrmx.org/kiss/) depends too many out-dated, about to be deprecated GNOME 2 packages (e.g. `libgnomeui`, `libgnomecanvas`) and no longer available on most of distros.
- It is very fun and romantic to read and implement some specifications comes from 30 years ago (and nearly as old as myself).
- Nostalgia and pixel-art from the childhood using Windows 95 / 98 (though I never heard KISS at that time).
- Real life is so painful. Wellness is in manga and anime.


# Development
> This project use `CMakePresets.json` to define build presets (`Release`, `Debug`...etc), for unify the build behavior between QtCreator and VSCode.

## QtCreator
Just `File -> Open File or Project` and select the `CMakeLists.txt` to open project, it will ask you which presets to be configured.

## VSCode
1. Open **the root folder of this project**. (if `CMake Tools` extension is not installed, please install it and restart VSCode, then open the folder again.)
2. `cmake.selectConfigurePreset` to select configure preset (`configurePresets`).
3. `F7` (`cmake.build`) to build.
4. `F5` to run the built binary.

## Manual
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