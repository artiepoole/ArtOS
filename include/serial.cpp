#include "serial.h"


int serial_received()
{
    return inb(PORT + 5) & 1;
}

char serial_read()
{
    while (serial_received() == 0);

    return inb(PORT);
}

int serial_transmit_empty()
{
    return inb(PORT + 5) & 0x20;
}

void serial_send_char(char a)
{
    while (serial_transmit_empty() == 0);

    outb(PORT, a);
}

void serial_write(const char* data, const size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        const char c = data[i];
        serial_send_char(c);
    }
}

void serial_write_string(const char* data)
{
    serial_write(data, strlen(data));
}



void serial_new_line()
{
    serial_send_char('\n');
}

// extern "C"
int serial_initialise()
{
    outb(PORT + 1, 0x00); // Disable all interrupts
    outb(PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT + 1, 0x00); //                  (hi byte)
    outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
    outb(PORT + 4, 0x1E); // Set in loopback mode, test the serial chip
    outb(PORT + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(PORT + 0) != 0xAE)
    {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(PORT + 4, 0x0F);
    return 0;
}
