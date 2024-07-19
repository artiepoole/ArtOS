#include "Serial.h"

static Serial *instance{ nullptr };

Serial::Serial()
{
    instance = this;
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
        connected = false;
        return;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(PORT + 4, 0x0F);
}


Serial::~Serial(){
    instance = nullptr;
}

Serial &Serial::get(){
    return *instance;
}

void Serial::write(const unsigned char c)
{
    _sendChar(c);
}

void Serial::write(const char* data)
{
    _write(data, mystrlen(data));
}

void Serial::write(const char* data, const size_t len)
{
    _write(data, len);
}

void Serial::newLine()
{
    _sendChar('\n');
}


int Serial::_received()
{
    return inb(PORT + 5) & 1;
}

int Serial::_transmitEmpty()
{
    return inb(PORT + 5) & 0x20;
}

char Serial::_read()
{
    while (_received() == 0);

    return inb(PORT);
}

void Serial::_sendChar(unsigned char a)
{
    while (_transmitEmpty() == 0);

    outb(PORT, a);
}

void Serial::_write(const char* data, const size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        const char c = data[i];
        _sendChar(c);
    }
}

void Serial::time_stamp()
{
    auto&rtc= RTC::get();
    write(asctime(rtc.getTime()));
}