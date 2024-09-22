//
// Created by artypoole on 05/09/24.
//
#include "stdlib.h"
#include "IDE_handler.h"
#include "logging.h"


IDE_handler_list ide_handler_list{};

void IDE_handler([[maybe_unused]] bool from_primary)
{
    //    // set command start/stop bit correctly
    // if (from_primary)
    //     LOG("IRQ 14 FIRED");
    // else
    //     LOG("IRQ 15 FIRED");
    // For each
    IDE_handler_node* current = ide_handler_list.first_node;
    while (current)
    {
        current->notifiable->notify();
        current = current->next;
    }
}


bool IDE_add_device(IDE_notifiable* notifiable)
{
    auto new_node = new IDE_handler_node;
    if (!new_node) return false;
    new_node->notifiable = notifiable;
    new_node->next = NULL;

    if (!ide_handler_list.first_node) // empty event list so put this as first.
    {
        ide_handler_list.first_node = new_node;
        return true;
    }

    IDE_handler_node* current = ide_handler_list.first_node;
    while (current->next)
    {
        current = current->next;
    }
    current->next = new_node;
    return true;
}

bool IDE_remove_device(IDE_notifiable* notifiable)
{
    IDE_handler_node* current = ide_handler_list.first_node;
    IDE_handler_node* prev = NULL;
    while (current)
    {
        if (current->notifiable == notifiable) // If found the handler to remove
        {
            if (prev) // not found at first node
            {
                prev->next = current->next;
            }
            else // currently at the first node.
            {
                // remake the link but the first node is replaced with the second
                ide_handler_list.first_node = current->next;
            }
            delete current; // free the memory
            return true; // success
        }
        // Not yet found handler so go to the next node
        prev = current;
        current = current->next;
    }
    return false;
}
