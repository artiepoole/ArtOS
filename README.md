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
- [x] Kernel sleep (block until n ticks)
- [ ] Proper handling of a scheduler to allow for sleeping of a task.
- [x] User IO - keyboard
- [ ] User IO - mouse?
- [ ] Detection and selection of video modes
- [ ] Initialise frame buffer etc using the multiboot data instead of hard coded
- [ ] Implement a standard library - WIP BRANCH: stdlib
- [ ] Implement filesystem
- [ ] Run an executable
- [ ] Self hosting compiler
- [ ] Play DOOM for ArtOS! - WIP BRANCH: doom (requires several stdlib features)

## Progress markers
Loading Screen and Keyboard
![Loading Screen and Keyboard demo.]([https://github.com/stupoole/ArtOS/blob/master/res/img/Splash.png?raw=true](https://github.com/stupoole/ArtOS/blob/main/res/img/keyboard_support.gif?raw=true))

Splash Screen
![Splash image drawn in 1024x768x32 graphics mode screen.](https://github.com/stupoole/ArtOS/blob/master/res/img/Splash.png?raw=true)

Pixel Mode
![Welcome to ArtOS! written in the centre of a 1024x768x32 graphics mode screen.](https://github.com/stupoole/ArtOS/blob/master/res/img/PixelMode.png?raw=true)

Colours:
![A printed with different FG and BG colours across the whole screen](https://github.com/stupoole/ArtOS/blob/master/res/img/Colours.png?raw=true)

HelloWorld:
![Hello World in qemu](https://github.com/stupoole/ArtOS/blob/master/res/img/HelloWorld.png?raw=true)





## Tools
- i686-elf gcc cross-compiler
- qemu emulator
- grub
- CMake
- CLion
- 
