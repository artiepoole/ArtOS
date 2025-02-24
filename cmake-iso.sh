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

GRUB_SRC="../grub.cfg"
DOOMWAD_SRC="../external_resources/doomwad/doom1.wad"

if grub-file --is-x86-multiboot2 ArtOS.bin; then
  if test -f $GRUB_SRC; then
    echo multiboot2 confirmed
    mkdir -p isodir/boot/grub
    mkdir -p isodir/fs
    cp ArtOS.bin isodir/boot/ArtOS.bin
    cp $GRUB_SRC isodir/boot/grub/grub.cfg
    cp $DOOMWAD_SRC isodir/fs/doom1.wad
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
