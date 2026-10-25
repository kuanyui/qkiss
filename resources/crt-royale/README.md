# CRT-Royale phosphor mask

The CRT filter (`src/crtfilter.cpp`) can copy the look of CRT-Royale. For that, it uses this picture of a slot mask
from CRT-Royale: `TileableLinearSlotMaskTall15Wide9And4d5Horizontal9d14VerticalSpacingResizeTo64.png`.

- Author: TroggleMonkey <trogglemonkey@gmx.com>, 2014
- License: GNU General Public License, version 2 or (at your option) any later version. See `LICENSE.TXT`, which
  comes from CRT-Royale too.
- Source: https://github.com/libretro/slang-shaders/tree/master/crt/shaders/crt-royale
  (the file is the same as in commit 901441a496121a9c12a9d72d30860e8be3fe4cf6)

The file is not changed. qkiss resizes the mask when it loads it, in the same way as CRT-Royale.
