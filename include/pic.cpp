//
// Created by artypoole on 04/07/24.
//

#include "pic.h"


#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

u8 mask1;
u8 mask2;

PIC::PIC()
{
    /* Normally, IRQs 0 to 7 are mapped to entries 8 to 15. This
    *  is a problem in protected mode, because IDT entry 8 is a
    *  Double Fault! Without remapping, every time IRQ0 fires,
    *  you get a Double Fault Exception, which is NOT actually
    *  what's happening. We send commands to the Programmable
    *  Interrupt Controller (PICs - also called the 8259's) in
    *  order to make IRQ0 to 15 be remapped to IDT entries 32 to
    *  47 */
    mask1 = inb(PIC1_DATA); // save masks
    mask2 = inb(PIC2_DATA);

    outb(PIC1, 0x11); // initialisation sequence
    outb(PIC2, 0x11);
    outb(PIC1_DATA, 0x20); // offset = 32
    outb(PIC2_DATA, 0x28); // offset = 32 + 8
    outb(PIC1_DATA, 0x04); // tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(PIC2_DATA, 0x02); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    outb(PIC1_DATA, 0x01); // 8086 mode
    outb(PIC2_DATA, 0x01); // 8086 mode
    outb(PIC1_DATA, 0x0); //
    outb(PIC2_DATA, 0x0);
    outb(PIC1_DATA, 0xFF); // Output mask - disable pic
    outb(PIC2_DATA, 0xFF); // Output mask - disable pic
}

void PIC::disable()
{
    auto &log = Serial::get();
    log.writeString("Disabled PIC");
    mask1 = inb(PIC1_DATA); // save masks
    mask2 = inb(PIC2_DATA);
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

void PIC::enable()
{
    auto &log = Serial::get();
    log.writeString("Renabled PIC");
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void PIC::enableIRQ(const u8 i)
{
    auto &log = Serial::get();
    log.writeString("Enabling IRQ");
    log.writeInt(i);
    log.newLine();


    if (i < 8)
    {
        const u8 old_mask1 = inb(PIC1_DATA);
        const u8 byte = 0x1 << i;
        mask1 = old_mask1 & byte;
        log.writeHex(byte);
        log.newLine();
    }
    else
    {
        const u8 old_mask2 = inb(PIC2_DATA);
        const u8 byte = 0x1 << (i-8);
        mask2 = old_mask2 & byte;
        log.writeHex(byte);
        log.newLine();
    }

    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void PIC::enableAll()
{
    mask1 = inb(PIC1_DATA);
    mask2 = inb(PIC2_DATA);
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}


volatile u32 ticks = 0;
u32 rate =0;


// extern "C"
void configurePit(const u32 hz)
{
    auto &log = Serial::get();
    const u32 divisor = 1193180 / hz; /* Calculate our divisor */
    rate = hz;
    log.writeString("Configured PIT. Divisor: ");
    log.writeInt(divisor);
    log.newLine();
    outb(0x43, 0x36); /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF); /* Set low byte of divisor */
    outb(0x40, divisor >> 8); /* Set high byte of divisor */
}

void sleep(const u32 ms)
{
    auto &log = Serial::get();
    if (rate==0)
    {
        log.writeString("Tried to sleep when timer is not initiated.");
        return;
    }

    ticks = ms * rate / 1000;  // rate is in hz, time is in ms

    log.writeString("Sleeping for ");
    log.writeInt(ms);
    log.writeString("ms. Ticks: ");
    log.writeInt(ticks);
    log.writeString(" Rate: ");
    log.writeInt(rate);
    log.newLine();
    while (ticks > 0);
    //log.writeString("Exited while loop. ");
    //log.writeString("Remaining ticks: ");
    //log.write_int(ticks);
    // log.new_line();
}

void timerHandler()
{
    /* Increment our 'tick count' */
    if (ticks==0) return;

    ticks--;

    /* Every 18 clocks (approximately 1 second), we will
    *  display a message on the screen */
    // if (ticks % rate == 0)
    // {
    //    log.writeString("One second has passed\n");
    // }
}
