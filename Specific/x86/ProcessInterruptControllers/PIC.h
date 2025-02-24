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
