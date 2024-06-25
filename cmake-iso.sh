#!/usr/bin/env sh

if grub-file --is-x86-multiboot ArtOS.bin; then
  echo multiboot confirmed
  mkdir -p isodir/boot/grub
  cp ArtOS.bin isodir/boot/ArtOS.bin
  cp ../grub.cfg isodir/boot/grub/grub.cfg
  grub-mkrescue -o ArtOS.iso isodir
  rm -rf isodir
else
  echo the file is not multiboot
fi
