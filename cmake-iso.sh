#!/usr/bin/env sh

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
