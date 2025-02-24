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

//
// Created by artypoole on 14/07/24.
//

#ifndef CMOS_H
#define CMOS_H


#include "CPU.h"
#include "time.h"

class RTC
{
public:

	u32 hz;
    RTC();
    ~RTC();
    static RTC& get();

    // remove copy functionality
    RTC(RTC const& other) = delete;
    RTC& operator=(RTC const& other) = delete;

	tm read();
	void increment();
	void disableIRQs();
	void enableIRQs();

	u32 setDivider(u8 divider);

	tm *getTime();

	void toString(char* out_str) const;
	time_t epochTime();

private:
	tm current_time = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	static u8 readRegister(u8 reg_select);

	void writeRegister(u8 reg_select, u8 data_byte);
	static u8 checkUpdating();
};

void read_RTC();

void RTC_interrupts_enable();

u32 RTC_set_divider(u8 divider);

void rtc_handler();


#endif //CMOS_H
