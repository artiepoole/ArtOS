# ArtOS
ArtOS is a very basic operating system which started from [osdev](https://wiki.osdev.org/Bare_Bones) tutorials but which quickly outgrew them.

The aim of this project is simply to learn. If it becomes something I want others to be able to use I will add the typical sections to this README.

## Progress markers

**Running DOOM (1993) IN USERSPACE! (+ user space apps and shell):**

[artosdemo-low.webm](https://github.com/user-attachments/assets/02c30762-726b-494d-93da-90d1ec0ef382)


**Running DOOM (1993):**

https://github.com/user-attachments/assets/c0464620-f6a0-44e3-b5c9-f59bd1c55018

p.s. it runs smoothly (easily achiving the hardcoded 35 fps) but the video recording and compression makes it look clunky

**Loading Screen and Keyboard:**

![keyboard_support](https://github.com/user-attachments/assets/8fea2d96-1281-46d2-94d4-97fe90898c3f)

**Splash Screen:**

![Splash Screen](https://github.com/user-attachments/assets/7e11bf7d-7b6a-4f3f-b169-63956f5d99e4)


**Pixel Mode:**

![Pixel Mode Graphics](https://github.com/user-attachments/assets/7409c5e2-a6e2-4e1f-a4d6-326ad56a3bef)


**Colours:**

![VGA Text Mode Colours](https://github.com/user-attachments/assets/185d925c-64bc-4986-af92-3fdbf05f513e)


**Hello World:**

![Hello World](https://github.com/user-attachments/assets/27ce4931-914b-4c4e-bdbf-f9c89d69ac8a)

## My Goals
- [x] **Basics**
  - [x] Boot into hello world
  - [x] Text mode terminal newline Character
  - [x] Text mode terminal Scrolling
  - [x] Colour Art
  - [x] Serial PORT1 output from kernel
  - [x] Pixel mode text support
  - [x] Draw "art" in pixel mode
  - [x] Replace grub and multiboot with grub2 and multiboot2
- [ ] **Interrupts and time**
  - [x] Interrupt Service Routines (ISR) are targeted on interrupt using a partially filled Interrupt Descriptor Table
  - [x] Interrupt Requests (IRQ) can be handled
  - [x] IRQ0 targets a timer decrementor to allow for scheduling and waiting.
  - [x] Kernel sleep (block until n ticks occur)
  - [x] Replace PIC with APIC
  - [x] Real time clock implemented
  - [x] Configure LAPIC timer for scheduling
- [ ] **User IO**
  - [x] Keyboard
  - [ ] Mouse
  - [x] Shell environment
- [ ] **Hardware detection and support**
  - [x] SMBIOS detection
  - [x] PCI device detection
  - [x] IDE detection
  - [ ] IDE initialisation (currently supports CD ROM only, see storage)
  - [x] Initialise frame buffer etc using the multiboot2 data instead of hard coded
  - [ ] Detection and selection of video modes i.e. reconfiguring the VGA hardware
- [ ] **Storage**
  - [x] Attach a RW storage device in Qemu
  - [x] Read data from CD ROM using PCI IDE DMA BusMastering
  - [x] Use data from CD ROM i.e. parse .iso file system
  - [ ] Support virtual attached RW storage
  - [ ] Support path strings or similar filename access (WIP)
  - [ ] Detect and mount/unmount storage devices on real hardware
  - [ ] USB storage support
- [ ] **Filesystem**
  - [x] Create a directory tree from iso fs data
  - [x] Implement a rudimentary filesystem
  - [ ] implement proper pdclib compliant filehandling
- [ ] **PDCLIB**
  - [x] Implement C standard library for kernel space
  - [x] Load a file using pdclib
  - [ ] User space functionality (e.g. exit, raise, signal)
- [ ] **DOOM**
  - [x] Play DOOM for ArtOS!
  - [x] Play DOOM for ArtOS on real hardware
  - [x] Loading from CD ROM instead of baked into the binary
  - [ ] Loading from boot USB
  - [x] Run in user space
  - [ ] Add sound
  - [ ] Add save game support
- [ ] **Optimisations**
  - [x] Basic SIMD for memcpy, memmove and memset
  - [ ] Advanced SIMD usage (string operations and vector maths)
  - [ ] There are many other optimisations to consider
- [ ] **Path to User Space**
  - [x] Use Paging/virtual memory
  - [x] Proper handling of a scheduler to allow for sleeping of a task
  - [x] Implement a standard library for user space
  - [x] Run an executable
  - [ ] Self hosting compiler
- [ ] **Misc**
  - [x] Remove resolution specific baked in splash screen with centered graphic (can be scaled) and programatically drawn borders. Bring back the loading bar?
- [ ] **Stretch goals**
  - [x] higher half
  - [ ] 64-bit support
  - [ ] ARM support (raspberry pi zero?)
  - [ ] Multithreading
  - [ ] Networking
  - [ ] USB support
  - [ ] Window manager and windows
  - [ ] Graphics driver(s)


## Known issues:

- [ ] I need to create some useful tools for the shell
  - such as "ls", "cd", "run" etc. This means I need to create the idea of a path/path string within my OS. This requires path lookup support in the kernel.

## Tools
- i686-elf gcc cross-compiler
- QEMU
- GRUB2
- Multiboot2
- CMake
- Jetbrains CLion
- xorriso
- ventoy
- [Public Domain C Library](https://github.com/DevSolar/pdclib)
- [DOOM Generic](https://github.com/ozkl/doomgeneric)

## Qemu Run Command:

```
qemu-system-i386
-cdrom
bin/ArtOS.iso
-serial
file:serial.log
-boot
d
-s
-S
-device
VGA,vgamem_mb=32
-m
2G
-no-reboot
-smbios
type=0
-drive
id=disk,file=external_resources/ArtOS_HDD.img,format=raw,if=none
-device
ide-hd,drive=disk,bus=ide.0
```

```
qemu-system-i386 -cdrom bin/ArtOS.iso -serial file:serial.log -boot a -s -S -device VGA,vgamem_mb=32 -m 2G -no-reboot
-smbios type=0 -drive id=disk,file=external_resources/ArtOS_HDD.img,format=raw,if=none -device
ide-hd,drive=disk,bus=ide.0
```

## Dependencies

```
sudo apt-get install
gcc-multilib
g++-multilib
grub-pc-bin
xorriso
mtools
```

make a disk image using:
`qemu-img create external_resources/ArtOS_HDD.img 512M` and change 512M to any size you like.


# Build and Run ArtOS

## Building ArtOS in schroot

**NOTE** package names/instructions are for ubuntu noble (24.04) - your mileage may vary!

I suggest using a chroot, container or some other separate environment for building to avoid polluting your host system with x86_32 architecture packages.

For schroot approach:
```
sudo apt install debootstrap schroot
sudo mkdir -p /srv/chroot/artos
sudo debootstrap noble /srv/chroot/artos http://archive.ubuntu.com/ubuntu/
sudo nano /etc/schroot/chroot.d/artos # or your preferred editor
```

and populate it with
```
[artos]
description=ubuntu noble for building artos
directory=/srv/chroot/artos
users=<your current username>
root-users=root,<your current username>
type=directory
profile=desktop
personality=linux
```

then enter the schroot environment
```
[sudo] schroot -c artos
```
you should see your bash prompt change.

Now set up build dependencies inside the schroot
```
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install -y cmake wget gcc-multilib g++-multilib libc6:i386 libstdc++6:i386 mtools grub-common grub-pc-bin socat xorriso
```

and then to build (still inside schroot, at root dir of project)

```
cmake -S ./ -B cmake-build-artos -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake -DENABLE_SERIAL_LOGGING:BOOL=ON -DENABLE_TERMINAL_LOGGING:BOOL=ON -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-artos -t ArtOS
```
**NOTE** you can change the flags as you see fit. e.g. -DCMAKE_BUILD_TYPE=Debug

To exit schroot run
```
exit
```

## Running ArtOS in qemu

In order to run ArtOS, you will need the proper qemu. On ubuntu the package is called `qemu-system-x86`, but the executable we need is called `qemu-system-i386`.

**NOTE** schroot will not have access to your display manager so you can only run it with no-monitor. To actually interact with ArtOS, please run it directly from your host.

To install qemu on (ubuntu) run
```
sudo apt install -y qemu-system-x86
```

and to run ArtOS, you can use this command
```
qemu-system-i386 -cdrom bin/ArtOS.iso -serial file:serial.log -boot d -s -m 2G -no-reboot
```
and additional options can be used to, e.g., attach a HDD (not implemented at time of writing) based on the [qemu docs](https://www.qemu.org/docs/master/system/qemu-manpage.html#hxtool-0)

e.g. further above is an example, and here it is broken down:
```
qemu-system-i386
# specify a cd-rom drive and file
-cdrom
bin/ArtOS.iso
# log serial IO to file
-serial
file:serial.log
# boot from cd-rom first
-boot
d
# `-s` is shorthand for `-gdb tcp::1234` - enabled gdb connections
-s
# Hang on start, allows you to connect gdb before even the first stage boot loader runs.
-S
# add video and specify video memory
-device
VGA,vgamem_mb=32
# specify max system RAM
-m
2G
# don't reboot the system on a cpu panic
-no-reboot
# this is supposed to specify the smbios type, but seems to do nothing :D
-smbios
type=0
# add a .img file as RW storage
-drive
id=disk,file=external_resources/ArtOS_HDD.img,format=raw,if=none
-device
ide-hd,drive=disk,bus=ide.0
# enable CPU register printouts for every interrupt
-d int
```

# Clion setup steps (mostly for Artie)

## use the toolchain:

After installing multilib, set up the following for each build profile (`edit configurations`):

example CMake options for File -> settings -> Build, Execution, Deployment -> CMake -> <profile>

```
-DENABLE_SERIAL_LOGGING:BOOL=ON
-DASYNC_READ=ON
-DENABLE_TERMINAL_LOGGING:BOOL=ON
-DSIMD:BOOL=ON
-DCMAKE_TOOLCHAIN_FILE="./toolchain.cmake"
```

## build the iso

Target is always `ArtOS` (no extension)

add a new build target: `CMake application`

Target: `ArtOS`

Executable: `<path/to/>qemu-system-i386` (usually `/usr/bin/qemu-system-i386`)

Program Arguments (example):

```
-cdrom
bin/ArtOS.iso
-serial
file:serial.log
-boot
a
-s
-S
-m
2G
-no-reboot
```

also append this to add a virtual HDD:

```
-drive
id=disk,file=external_resources/ArtOS_HDD.img,format=raw,if=none
-device
ide-hd,drive=disk,bus=ide.0
```

Working Directory (optional): `<Path/To/Project/Root>`

Before launch: `Build`

## Remote debugging

Add a new target in `edit configurations`:

`+` -> `Remote Debug`

Give it a name like `remote debug ArtOS`

Debugger: `Bundled GDB (multiarch)`

'target remote' args: `localhost:1234`

Symbol File: `<path/to/project/root>/bin/ArtOS.bin` (or path to, e.g., `doom.art`)

## Creating a blank ext4 image for RW storage (WIP)

Make a blank file with `dd if=/dev/zero of=external_resources/ArtOS_HDD.img bs=1M count=64` (this is 64MB because it's `bs` times `count`)

Give it an ext4 format: `mkfs.ext4 -F external_resources/ArtOS_HDD.img [-L ArtOS_RW]` where the stuff in square brackets labels it with a name