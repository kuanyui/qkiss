#!/bin/env python3
import os
import sys
import re
import argparse
from datetime import datetime

from pathlib import Path

SCRIPT_PATH: Path = Path(__file__).resolve()
SCRIPT_DIR: Path = SCRIPT_PATH.parent
REPO_ROOT_DIR: Path = SCRIPT_DIR.parent
CPP_DIR: Path = REPO_ROOT_DIR / 'src'
YEAR: int = datetime.now().year

LICENSE_TEMPLATE: str = f"""\
/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) {YEAR} ono ono (kuanyui) <azazabc123@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/
"""

def has_license(path: Path) -> bool:
    content: str = path.read_text(encoding='utf-8')
    return re.search(r"Copyright \(C\) ", content) is not None

def insert_license(path: Path) -> None:
    original: str = path.read_text(encoding='utf-8')
    new_content: str = LICENSE_TEMPLATE + '\n' + original
    path.write_text(new_content, encoding='utf-8')

def main(dry_run: bool = False) -> int:
    files_missing_license: list[Path] = []

    for subdir, _, files in os.walk(CPP_DIR):
        for filename in files:
            if filename.endswith(('.cpp', '.h')):
                path: Path = Path(subdir) / filename
                if not has_license(path):
                    files_missing_license.append(path)

    if dry_run:
        if files_missing_license:
            print("❌ Files missing license header:")
            for path in files_missing_license:
                print(f"  - {path.relative_to(REPO_ROOT_DIR)}")
            return 1
        else:
            print("✅ All files have license headers.")
            return 0
    else:
        if len(files_missing_license) == 0:
            print("✅ All files have license headers. Skip.")
        else:
            for path in files_missing_license:
                insert_license(path)
            print(f"✅ Applied license to {len(files_missing_license)} files.")
        return 0

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true", help="Check only, do not modify files. If some files have no license header, exit code is 1.")
    args = parser.parse_args()

    sys.exit(main(dry_run=args.dry_run))
