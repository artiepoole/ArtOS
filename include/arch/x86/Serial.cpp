#include "Serial.h"

#include <ArtFile.h>

#include "ports.h"
#include "RTC.h"
#include "logging.h"

static Serial *instance{ nullptr };
static ArtFile *file_wrapper{ nullptr };

#define RECEIVE_OFFSET 0x0
#define SEND_OFFSET 0x0
#define INTERRUPT_REG_OFFSET 0x1
#define INTERRUPT_ID_OFFSET 0x2
#define FIFO_OFFSET 0x2
#define LINE_CONTROL_OFFSET 0x3
#define MODEM_CONTROL_OFFSET 0x4
#define STATUS_OFFSET 0x5
#define MODEM_STATUS_OFFSET 0x6
#define SCRATCH_OFFSET

Serial::Serial()
{
    instance = this;
    outb(PORT + INTERRUPT_REG_OFFSET, 0x00); // Disable all interrupts
    outb(PORT + LINE_CONTROL_OFFSET, 0x80); // Enable DLAB (set baud rate divisor)
    outb(PORT + SEND_OFFSET, 0x03); // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT + INTERRUPT_REG_OFFSET, 0x00); //                  (hi byte)
    outb(PORT + LINE_CONTROL_OFFSET, 0x03); // 8 bits, no parity, one stop bit
    outb(PORT + FIFO_OFFSET, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + MODEM_CONTROL_OFFSET, 0x0B); // IRQs enabled, RTS/DSR set
    outb(PORT + MODEM_CONTROL_OFFSET, 0x1E); // Set in loopback mode, test the serial chip
    outb(PORT + SEND_OFFSET, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(PORT + RECEIVE_OFFSET) != 0xAE)
    {
        connected = false;
        return;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(PORT + MODEM_CONTROL_OFFSET, 0x0F);
    connected = true;

    WRITE("DAY MON DD HH:MM:SS YYYY\tSerial connected\n");
    char name[] = "/dev/com1";
    file_wrapper = new ArtFile{this, name};
}


Serial::~Serial(){
    instance = nullptr;
}

Serial &Serial::get(){
    return *instance;
}

ArtFile*& Serial::get_file(){
    return file_wrapper;
}


void Serial::write(const char c)
{
    _send_one_byte(c);
}

void Serial::write(const char* data)
{
    _write_buffer(data, mystrlen(data));
}

void Serial::write(const char* data, const size_t len)
{
    _write_buffer(data, len);
}

void Serial::newLine()
{
    _send_one_byte('\n');
}


int Serial::_get_read_ready_status()
{
    return inb(PORT + STATUS_OFFSET) & 1;
}

int Serial::_get_transmit_empty()
{
    return inb(PORT + STATUS_OFFSET) & 0x20;
}

char Serial::_read_one_byte()
{
    while (_get_read_ready_status() == 0);

    return inb(PORT + RECEIVE_OFFSET);
}

void Serial::_send_one_byte(unsigned char a)
{
    while (_get_transmit_empty() == 0);

    outb(PORT+ SEND_OFFSET, a);
}

void Serial::write(bool b)
{
    if (b == true)
    {
        _write_buffer("True", 4);
    }
    else
    {
        _write_buffer("False", 5);
    }
}


void Serial::_write_buffer(const char* data, const size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        const char c = data[i];
        _send_one_byte(c);
    }
}

void Serial::time_stamp()
{
    write(asctime(RTC::get().getTime()));
}

u32 Serial::com_read(char* dest, const u32 count)
{
    for(u32 i = 0; i < count; i++) {
        dest[i] = _read_one_byte();
    }
    return count;
}
u32 Serial::com_write(const char* data, const u32 count)
{
    _write_buffer(data, count);
    return count;
}