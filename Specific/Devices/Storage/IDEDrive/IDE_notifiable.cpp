//
// Created by artypoole on 22/09/24.
//

#include "IDE_notifiable.h"
#include "IDE_handler.h"

bool IDE_notifiable::add_to_list(){return IDE_add_device(this);}
bool IDE_notifiable::remove_from_list(){return IDE_remove_device(this);}