//
// Created by artypoole on 04/07/24.
//

#ifndef PIC_H
#define PIC_H
#include "types.h"


class PIC
{
public:
    PIC();
    void pause_PIC();
    void resume_PIC();
    static void disable_entirely();
    void enableIRQ(u8 irq_id);
    void disableIRQ(u8 irq_id);
    void enableAll();
};



#endif //PIC_H
