# ArtOS
ArtOS is a very basic operating system being developed with help from [osdev](https://wiki.osdev.org/Bare_Bones) tutorials. 

The aim of this project is simply to learn. If it becomes something I want others to be able to use I will add the typical sections to this README.

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
- [ ] DOOM: run in user space
- [ ] DOOM: add sound
- [ ] DOOM: add save games
- [ ] DOOM: add loading from CDROM instead of from memory
- [ ] User IO - mouse
- [ ] Read from CDRom
- [ ] Attach a RW storage device in Qemu
- [ ] Implement a filesystem
- [ ] Detect and mount/unmount storage devices on real hardware
- [ ] Replace PIT with HPET
- [ ] Proper handling of a scheduler to allow for sleeping of a task
- [ ] Detection and selection of video modes i.e. reconfiguring the VGA hardware
- [ ] Implement a standard library for user space
- [ ] Run an executable
- [ ] Self hosting compiler


## Progress markers

Running DOOM (1993):

https://github.com/user-attachments/assets/c0464620-f6a0-44e3-b5c9-f59bd1c55018

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



## Tools
- i686-elf gcc cross-compiler
- qemu emulator
- grub
- CMake
- CLion

