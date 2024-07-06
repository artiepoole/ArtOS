//
// Created by artypoole on 04/07/24.
//

#include "pic.h"

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define PIC_EOI		0x20


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

void pic_irq_remap()
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

void pic_disable()
{
    serial_write_string("Disabled PIC");
    mask1 = inb(PIC1_DATA); // save masks
    mask2 = inb(PIC2_DATA);
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

void pic_enable()
{
    serial_write_string("Renabled PIC");
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_enable_irq0()
{
    u8 old_mask1 = inb(PIC1_DATA);
    u8 old_mask2 = inb(PIC2_DATA);
    mask1 = old_mask1 & 0xFE;
    mask2 = old_mask2 & 0xFE;

    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_enable_all()
{
    mask1 = inb(PIC1_DATA);
    mask2 = inb(PIC2_DATA);
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}


volatile u32 ticks = 0;
u32 rate =0;


// extern "C"
void configure_pit(u32 hz)
{

    u32 divisor = 1193180 / hz; /* Calculate our divisor */
    rate = hz;
    serial_write_string("Configured PIT. Divisor: ");
    serial_write_int(divisor);
    serial_new_line();
    outb(0x43, 0x36); /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF); /* Set low byte of divisor */
    outb(0x40, divisor >> 8); /* Set high byte of divisor */
}

void sleep(u32 ms)
{
    if (rate==0)
    {
        serial_write_string("Tried to sleep when timer is not initiated.");
        return;
    }

    ticks = ms * rate / 1000;  // rate is in hz, time is in ms

    serial_write_string("Sleeping for ");
    serial_write_int(ms);
    serial_write_string("ms. Ticks: ");
    serial_write_int(ticks);
    serial_write_string(" Rate: ");
    serial_write_int(rate);
    serial_new_line();
    while (ticks > 0);
    // serial_write_string("Exited while loop. ");
    // serial_write_string("Remaining ticks: ");
    // serial_write_int(ticks);
    // serial_new_line();
}

void timer_handler()
{
    /* Increment our 'tick count' */
    if (ticks==0) return;

    ticks--;

    /* Every 18 clocks (approximately 1 second), we will
    *  display a message on the screen */
    // if (ticks % rate == 0)
    // {
    //     serial_write_string("One second has passed\n");
    // }
}
