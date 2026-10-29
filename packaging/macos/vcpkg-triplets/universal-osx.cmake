# A vcpkg triplet like vcpkg's own `arm64-osx`, but each static library has the code for both
# Apple silicon (arm64) and Intel (x86_64), like `qkiss.app` (see `CMAKE_OSX_ARCHITECTURES` in macos.yaml).
# vcpkg has no such triplet, so macos.yaml gives this folder to vcpkg (`--overlay-triplets`, `VCPKG_OVERLAY_TRIPLETS`).
set(VCPKG_TARGET_ARCHITECTURE arm64)  # Required by vcpkg. The code for both CPUs comes from VCPKG_OSX_ARCHITECTURES below.
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES "arm64;x86_64")
