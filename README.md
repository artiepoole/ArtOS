# ArtOS
ArtOS is a very basic operating system which started from [osdev](https://wiki.osdev.org/Bare_Bones) tutorials but which quickly outgrew them. 

The aim of this project is simply to learn. If it becomes something I want others to be able to use I will add the typical sections to this README.

## Progress markers

Running DOOM (1993):

https://github.com/user-attachments/assets/c0464620-f6a0-44e3-b5c9-f59bd1c55018

p.s. it runs smoothly (easily achiving the hardcoded 35 fps) but the video recording and compression makes it look clunky 

Loading Screen and Keyboard:

![keyboard_support](https://github.com/user-attachments/assets/8fea2d96-1281-46d2-94d4-97fe90898c3f)

Splash Screen:

![Splash Screen](https://github.com/user-attachments/assets/7e11bf7d-7b6a-4f3f-b169-63956f5d99e4)


Pixel Mode:

![Pixel Mode Graphics](https://github.com/user-attachments/assets/7409c5e2-a6e2-4e1f-a4d6-326ad56a3bef)


Colours:

![VGA Text Mode Colours](https://github.com/user-attachments/assets/185d925c-64bc-4986-af92-3fdbf05f513e)


Hello World:

![Hello World](https://github.com/user-attachments/assets/27ce4931-914b-4c4e-bdbf-f9c89d69ac8a)

## My Goals
- [x] Boot into hello world
- [x] Text mode terminal newline Character
- [x] Text mode terminal Scrolling
- [x] Colour Art
- [x] Serial PORT1 output from kernel
- [x] Pixel mode text support
- [x] Draw "art" in pixel mode
- [x] Interrupt Service Routines (ISR) are targeted on interrupt using a partially filled Interrupt Descriptor Table
- [x] Interrupt Requests (IRQ) can be handled
- [x] IRQ0 targets a timer decrementor to allow for scheduling and waiting.
- [x] Kernel sleep (block until n ticks occur)
- [x] User IO - keyboard
- [x] SMBIOS detection
- [x] PCI device detection
- [x] Real time clock implemented
- [x] Use grub2 and multiboot2
- [x] Replace PIC with APIC
- [x] Initialise frame buffer etc using the multiboot2 data instead of hard coded
- [x] Implement c standard library for kernel space
- [x] Play DOOM for ArtOS!
- [x] Play DOOM for ArtOS on real hardware
- [x] Read data from CD ROM using PCI IDE DMA BusMastering
- [x] Use data from CD ROM i.e. parse .iso file system
- [x] Create a directory tree from iso fs data
- [x] Implement a rudimentary filesystem
- [x] Load a file using pdclib
- [x] DOOM: Loading from CD ROM instead of baked into the binary
- [ ] DOOM: run in user space
- [ ] DOOM: add sound
- [ ] Attach a RW storage device in Qemu
- [ ] DOOM: add save games
- [ ] User IO - mouse
- [ ] implement proper pdclib compliant filehandling
- [ ] Detect and mount/unmount storage devices on real hardware
- [ ] Replace PIT with HPET
- [ ] Proper handling of a scheduler to allow for sleeping of a task
- [ ] Detection and selection of video modes i.e. reconfiguring the VGA hardware
- [ ] Implement a standard library for user space
- [ ] Run an executable
- [ ] Self hosting compiler

## Tools
- i686-elf gcc cross-compiler
- QEMU
- GRUB2
- Multiboot2
- CMake
- Jetbrains CLion
- xorriso
- [Public Domain C Library](https://github.com/DevSolar/pdclib)
- [DOOM Generic](https://github.com/ozkl/doomgeneric)
