#!/usr/bin/env sh

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
set -e

GRUB_SRC=".${CMAKE_SOURCE_DIR}/../grub.cfg"
BIN_SRC=".${CMAKE_BUILD_DIR}/ArtOS.bin"
BART_SRC=".${CMAKE_BUILD_DIR}/b.art"
HELLO_SRC=".${CMAKE_BUILD_DIR}/hello.art"
DOOMWAD_SRC=".${CMAKE_SOURCE_DIR}/../external_resources/doomwad/doom1.wad"

echo "Grub source: ${GRUB_SRC}"
echo "ArtOS bin loc: ${BIN_SRC}"
echo "DOOMWAD source: ${DOOMWAD_SRC}"
echo "b.art source: ${BART_SRC}"
echo "hello.art source: ${HELLO_SRC}"


if grub-file --is-x86-multiboot2 ArtOS.bin; then
  if test -f $GRUB_SRC; then
    echo multiboot2 confirmed
    mkdir -p isodir/boot/grub
    mkdir -p isodir/fs
    cp "${BIN_SRC}" isodir/boot/ArtOS.bin
    cp "${GRUB_SRC}" isodir/boot/grub/grub.cfg
    if test -f "${DOOMWAD_SRC}"; then
      cp "${DOOMWAD_SRC}" isodir/fs/doom1.wad
    else
      wget https://distro.ibiblio.org/slitaz/sources/packages/d/doom1.wad -O isodir/fs/doom1.wad
   fi
   cp "${BART_SRC}" isodir/fs/
    cp "${HELLO_SRC}" isodir/fs/
    grub-mkrescue -o ArtOS.iso isodir
#    rm -rf isodir
  else
    echo could not find grub config
    exit 1
  fi
else
  echo the file is not multiboot2
  exit 1
fi

exit 0
