//
// Created by artypoole on 14/07/24.
//

#ifndef CMOS_H
#define CMOS_H

#include "ports.h"
#include "Serial.h"
#include "system.h"
#include "time.h"

// struct tm // using u16 for ease of printing.
// {
// 	u8 second, minute, hour, day, month, weekday;
// 	u16 year; //  4 digit year
// 	u8 regB;
// };

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
	void disableInterrupts();
	void enableInterrupts();

	u32 setDivider(u8 divider);

	tm *getTime();

	void toString(char* out_str) const;
	time_t epochTime();

private:
	tm current_time{};

    static u8 readRegister(u8 reg_select);
	static u8 checkUpdating();
};

void read_RTC();

void RTC_interrupts_enable();

u32 RTC_set_divider(u8 divider);

void rtc_handler();


#endif //CMOS_H
