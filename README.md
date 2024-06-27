# ArtOS
ArtOS is a very basic operating system being developed following [osdev](https://wiki.osdev.org/Bare_Bones) tutorials. 

The aim of this project is simply to learn. If it becomes something I want others to be able to use I will add the typical sections to this README.

## My Goals
- [x] Boot into hello world
- [x] Text mode terminal newline Character
- [x] Text mode terminal Scrolling
- [x] Colour Art
- [x] Serial PORT1 output from kernel
- [x] Pixel mode text support
- [ ] Detection and selection of video modes
- [ ] Initialise frame buffer etc using the multiboot data instead of hard coded
- [ ] User IO - keyboard
- [ ] User IO - mouse?
- [ ] Implement a standard library
- [ ] Print filesystem structure
- [ ] Run an executable so that a compiler can be used
- [ ] Play DOOM for ArtOS!

## Progress markers
HelloWorld:
![Hello World in qemu](https://github.com/stupoole/ArtOS/blob/master/res/img/HelloWorld.png?raw=true)
Colours:
![A printed with different FG and BG colours across the whole screen](https://github.
com/stupoole/ArtOS/blob/master/res/img/Colours.png?raw=true)
![Welcome to ArtOS! written in the centre of a 1024x768x32 graphics mode screen.](https://github.
com/stupoole/ArtOS/blob/master/res/img/PixelMode.png?raw=true)
![Splash image drawn in 1024x768x32 graphics mode screen.](https://github.
com/stupoole/ArtOS/blob/master/res/img/Splash.png?raw=true)


## Tools
- i686-elf gcc cross-compiler
- qemu emulator
- grub
- CMake
- CLion
- 
