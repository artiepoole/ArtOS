// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

#include "Serial.h"

#include "ArtFile.h"
#include "Files.h"

#include "ports.h"
#include "RTC.h"
#include "logging.h"
#include "Terminal.h"

// static Serial* instance{nullptr};
static ArtFile* file_wrapper{nullptr};

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
    // instance = this;
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

    Terminal::get().write("Sun Jan  0 00:00:00 1900\tSerial connected\n");
    write("Sun Jan  0 00:00:00 1900\tSerial connected\n");
}

void Serial::link_file()
{
    char name[] = "/dev/com1";
    file_wrapper = new ArtFile{this, name};
}


// Serial::~Serial()
// {
//     instance = nullptr;
// }

// Serial& Serial::get()
// {
//     static Serial instance;
//     return instance;
// }

ArtFile*& Serial::get_file()
{
    return file_wrapper;
}


void Serial::register_device()
{
    register_storage_device(this);
}

void Serial::write(const char c)
{
    _send_one_byte(c);
}

void Serial::write(const char* data)
{
    _write_buffer(data, art_string::strlen(data));
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

    outb(PORT + SEND_OFFSET, a);
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
    tm time{};
    RTC::get().getTime(&time);
    write(asctime(&time));
}

u32 Serial::com_read(char* dest, const u32 count)
{
    for (u32 i = 0; i < count; i++)
    {
        dest[i] = _read_one_byte();
    }
    return count;
}

u32 Serial::com_write(const char* data, const u32 count)
{
    _write_buffer(data, count);
    return count;
}

Serial& get_serial()
{
    static Serial instance;
    return instance;
}
