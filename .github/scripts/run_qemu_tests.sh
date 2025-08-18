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

# https://gist.github.com/mvidner/8939289 - qemu characters
# TODO: implement the JSON version
# https://www.qemu.org/docs/master/devel/qapi-code-gen.html

qemu-system-i386 -cdrom bin/ArtOS.iso -serial file:serial.log -boot a -s -m 2G -no-reboot -nographic -monitor unix:qemu-monitor-socket,server,nowait&
sleep 5
cat .github/qemu_test_cmds/grub_enter_cmds | socat - unix-connect:qemu-monitor-socket && printf '\n'
sleep 30
cat .github/qemu_test_cmds/run_doom_cmds | socat - unix-connect:qemu-monitor-socket && printf '\n'
sleep 30
cat .github/qemu_test_cmds/quit_doom_cmds |  socat - unix-connect:qemu-monitor-socket && printf '\n'
sleep 5
pkill qemu-