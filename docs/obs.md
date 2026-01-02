# Config file of `osc`
- `oscrc` is at `~/.config/osc/oscrc` by default. Can be overwritten via `export OSC_CONFIG=~/.config/osc/oscrc_ANOTHER`, which can be used to create multiple accounts.
- To check the default values of `oscrc`, see `man oscrc`.
- My personal extra settings in `[general]` section of `~/.config/osc/oscrc`:
```conf
# The directory where downloaded packages are stored.
packagecachedir=/var/tmp/osbuild-packagecache

# QEMU/KVM img path
# build-root=/var/tmp/build-root%(dash_user)s/%(repo)s-%(arch)s
build-root=/tmp/build-root%(dash_user)s/%(repo)s-%(arch)s

# KVM, 16G memory, minimal swapfile (10M)
build-jobs=20
build-type=kvm
build-memory=16000
build-vmdisk-swapsize=10
build-shell-after-fail=1
```
# Local build
- [Use OBS to build your package](https://medium.com/@bilsted/use-obs-to-build-your-package-9afb81d3703d)
- [Local Building | User Guide](https://openbuildservice.org/help/manuals/obs-user-guide/cha-obs-local-building)

> To prevent `Ctrl+c` accidentally breaks `osc build --shell`, run
> ```
> stty intr `Ctrl+v Ctrl+] Enter`
> ```
> before running `osc build --shell`

```sh
cd ~/osc/home:kuanyui  # home:kuanyui/ is PROJECT directory (You MUST to create this PROJECT on OBS server via Web UI, then you can run `osc init` locally to create this PROJECT directory)
osc mkpac qkiss        # Create a PACKAGE named `qkiss` (only on local, until you commit.)
osc meta prj           # See what distros' repository are included in this PROJECT.
osc build --vm-type=kvm --vm-memory 16000 Debian_12 x86_64  # Build for Debian_12 (x86_64) in KVM with 16G RAM
```

# Delete build root / VM images
- The default paths are in `man oscrc`.
```conf
# Clean downloaded cache (not recommended because OBS download speed is so slow...)
# ocsrc :: packagecachedir : str = "/var/tmp/osbuild-packagecache"
rm -rf "/var/tmp/osbuild-packagecache/Debian:12"

# Clean QEMU/KVM image files (Typically 4G img + 1G swap file)
# oscrc :: build-root : str = "/var/tmp/build-root%(dash_user)s/%(repo)s-%(arch)s"
rm -rf "/var/tmp/build-root-${USER}/Debian_12-x86_64"
```

# Prefer depends via OSB (Not recommended)

> Recommended way is using `|` syntax in `debian/controls` directly.

Edit project meta file via `osc meta prjconf -e <project_name>`
```sh
osc meta prjconf -e home:kuanyui
```

```conf
# Resolve multiple choices problem for Debian + `qt6-multimedia-devel`
# unresolvable: have choice for libavcodec59 >= 7:5.0 needed by libqt6multimedia6: libavcodec-extra59 libavcodec59

# Debian 12 (libav5)
Prefer: libavcodec59 libavformat59

# Debian >= 13 (libav6)
Prefer: libavcodec60 libavformat60
Prefer: libavcodec61 libavformat61
Prefer: libavcodec62 libavformat62

# openSUSE (multiple python versions are available...)
Prefer: python311-jsonschema
Prefer: python311-PyYAML
```

