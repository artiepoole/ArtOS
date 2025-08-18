#!/bin/bash

#
# ArtOS - hobby operating system by Artie Poole
# Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
#
#     This program is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with this program.  If not, see <https://www.gnu.org/licenses/>
#

INCLUDED_DIRS=("executable_src/bart" "executable_src/helloworld" "Generic" "Specific" "ArtOS_lib" "ArtOSTypes")
EXTENSIONS=("*.cpp" "*.h" "*.c" "*.S" "*.s" "*.cmake" "*.txt")

total=0

for dir in "${INCLUDED_DIRS[@]}"; do
  for ext in "${EXTENSIONS[@]}"; do
    # Find matching files and count lines
    while IFS= read -r -d '' file; do
      lines=$(wc -l < "$file")
      total=$((total + lines))
      printf "%6d  %s\n" "$lines" "$file"
    done < <(find "$dir" -type f -name "$ext" -print0)
  done
done

echo "total=$total"