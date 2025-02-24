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
// Created by artypoole on 05/09/24.
//

#ifndef IDE_HANDLER_H
#define IDE_HANDLER_H

#include "IDE_notifiable.h"



// struct IDE_handler_node{
//     IDE_notifiable* notifiable;
//     IDE_handler_node* next;
// };


// TODO: replace with linked list implementation

struct IDE_handler_node{
    IDE_notifiable* notifiable;
    IDE_handler_node* next;
};

typedef struct
{
    IDE_handler_node* first_node{};
} IDE_handler_list;


void IDE_handler(bool from_primary);

bool IDE_add_device(IDE_notifiable* notifiable);
bool IDE_remove_device(IDE_notifiable* notifiable);




#endif //IDE_HANDLER_H
