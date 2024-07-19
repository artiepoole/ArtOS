#include <Serial.h>

void disable_interrupts(){
    auto&log = Serial::get();
    log.log("\tInterrupts disabled");
    __asm__ volatile ("cli"); // clear the interrupt flag
}

void enable_interrupts(){
    auto&log = Serial::get();
    log.log("\tInterrupts enabled");
    __asm__ volatile ("sti"); // set the interrupt flag
}