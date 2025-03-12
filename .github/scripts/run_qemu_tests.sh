#!/usr/bin/env sh

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