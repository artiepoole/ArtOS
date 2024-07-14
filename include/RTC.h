//
// Created by artypoole on 14/07/24.
//

#ifndef CMOS_H
#define CMOS_H

#include "ports.h"
#include "Timers.h"
#include "Serial.h"
#include "system.h"


class RTC
{
public:
    RTC();
    ~RTC();
    static RTC& get();

    // remove copy functionality
    RTC(RTC const& other) = delete;
    RTC& operator=(RTC const& other) = delete;

	RTC_t read();
	void increment();
	void disableInterrupts();
	void enableInterrupts();

	u32 setDivider(u8 divider);

	RTC_t getTime();

	void toString(char* out_str) const;


private:
	RTC_t current_time{};
	u32 hz;
    static u8 readRegister(u8 reg_select);
	static u8 checkUpdating();
};

void read_RTC();

void RTC_interrupts_enable();

u32 RTC_set_divider(u8 divider);


#endif //CMOS_H
