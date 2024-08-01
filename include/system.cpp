#include "Serial.h"

void disable_interrupts(){
    LOG("\tInterrupts disabled");
    __asm__ volatile ("cli"); // clear the interrupt flag
}

void enable_interrupts(){
    LOG("\tInterrupts enabled");
    __asm__ volatile ("sti"); // set the interrupt flag
}