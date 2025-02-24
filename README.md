# ArtOS
ArtOS is a very basic operating system which started from [osdev](https://wiki.osdev.org/Bare_Bones) tutorials but which quickly outgrew them.

The aim of this project is simply to learn. If it becomes something I want others to be able to use I will add the typical sections to this README.

## Progress markers

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
  - [ ] Shell environment
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
  - [ ] Support path strings or similar filename access
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
  - [ ] Run in user space
  - [ ] Add sound
  - [ ] Add save game support
- [ ] **Optimisations**
  - [x] Basic SIMD for memcpy, memmove and memset
  - [ ] Advanced SIMD usage (string operations and vector maths)
  - [ ] Ther are many other optimisations to consider
- [ ] **Path to User Space**
  - [x] Use Paging/virtual memory
  - [x] Proper handling of a scheduler to allow for sleeping of a task
  - [ ] Implement a standard library for user space
  - [ ] Run an executable
  - [ ] Self hosting compiler
- [ ] **Misc**
  - [ ] Remove resolution specific baked in splash screen with centered graphic (can be scaled) and programatically drawn borders. Bring back the loading bar?
- [ ] **Stretch goals**
  - [ ] higher half and 64-bit support
  - [ ] ARM support (raspberry pi zero?)
  - [ ] Multithreading
  - [ ] Networking
  - [ ] USB support
  - [ ] Window manager and windows
  - [ ] Graphics driver(s)


## Known issues:

- [ ] I have not made an internal memory allocator.
  - I have relied on the pdclib malloc implementation too heavily, and now it is supposed to use syscalls so that user space processes can use it, and I have no way to internally allocate while staying within the relevant context.
- [ ] Paging is only ever kernel level and global. This means that user processes cannot have their own memory map.
  - Each process should have its own paging table, but I mustn't multi-allocate physical addresses. This probably just needs to use the physical address bitmap globally.
  - Should probably make a PageTable class which handles the virtual address mapping
- [ ] The IDT files are trash.
  - It's difficult to change the interrupt vectors.
  - If the vector is not sequential it is not assigned properly e.g. even if I call the syscall isr 0x80, the actual interrupt is 0x50. This is a problem for the spurious interrupt as well because it HAS to be >240 or similar.
- [ ] My interrupt process/Scheduler/IDE driver components do not work together
  - The IDE driver needs to receive interrupts but the process requesting a read triggers an interrupt which disables interrupts until it returns. This means the IDE interrupt is lost.
  - I need to make it so that a read syscall causes the calling task to sleep
  - I need to implement concurrency handling in the driver (i.e. spinlocks or whatever)
  - I need to keep track of what process was requesting the read so that it can be resumed when the read finishes.
  - Should the IDE exist as a process/daemon so that it can have its own stack etc?
- [ ] malloc has no concept of user or alignment. Internally, I should implement a new version of this or just use mmap instead of malloc? I think if I fix the paging tables issue, that will be handled better.
- [ ] I need to create some useful tools for the shell
  - such as "ls", "cd", "run" etc. This means I need to create the idea of a path/path string within my OS.

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
-serial[doom1.wad](external_resources/doomwad/doom1.wad)
file:serial.log
-boot
a
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

qemu-system-i386 -cdrom bin/ArtOS.iso -serial file:serial.log -boot a -s -S -device VGA,vgamem_mb=32 -m 2G -no-reboot
-smbios type=0 -drive id=disk,file=external_resources/ArtOS_HDD.img,format=raw,if=none -device
ide-hd,drive=disk,bus=ide.0

## Dependencies

```
sudo apt-get install 
grub-pc-bin
xorriso
mtools
```

make a disk image using:
`qemu-img create external_resources/ArtOS_HDD.img 512M` and change 512M to any size you like.