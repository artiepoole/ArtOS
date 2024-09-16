//
// Created by artypoole on 05/09/24.
//

#ifndef IDE_HANDLER_H
#define IDE_HANDLER_H
#include "IDEStorageContainer.h"
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
