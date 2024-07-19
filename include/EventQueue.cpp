//
// Created by artypoole on 06/07/24.
//

#include "EventQueue.h"

static EventQueue* instance{nullptr};


/* US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. */
char key_map[128] =
{
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
    '9', '0', '-', '=', '\b', /* Backspace */
    '\t', /* Tab */
    'q', 'w', 'e', 'r', /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    '^', /* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
    '\'', '`', '*', /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', /* 49 */
    'm', ',', '.', '/', '+', /* Right shift */
    '*',
    '!', /* Alt */
    ' ', /* Space bar */
    'C', /* Caps lock */
    0, /* 59 - F1 key ... > */
    0, 0, 0, 0, 0, 0, 0, 0,
    0, /* < ... F10 */
    0, /* 69 - Num lock*/
    0, /* Scroll Lock */
    'H', /* Home key */
    'U', /* Up Arrow */
    0, /* Page Up */
    '-',
    '<', /* Left Arrow */
    0,
    '>', /* Right Arrow */
    '+',
    'E', /* 79 - End key*/
    'D', /* Down Arrow */
    0, /* Page Down */
    0, /* Insert Key */
    0, /* Delete Key */
    0, 0, 0,
    0, /* F11 Key */
    0, /* F12 Key */
    0, /* All other keys are undefined */
};
char shift_map[128] =
{
    0, 27, '!', '"', '#', '$', '%', '^', '&', '*', /* 9 */
    '(', ')', '_', '+', '\b', /* Backspace */
    0, /* Tab */
    'Q', 'W', 'E', 'R', /* 19 */
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0, /* ENTER KEY */
    0, /* 29   - CONTROL */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
    '@', '~', 0, /* LEFT SHIFT */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', /* 49 */
    'M', '<', '>', '?', 0, /* Right shift */
    0,
    0, /* Alt */
    ' ', /* Space bar */
    0, /* Caps lock */
    0, /* 59 - F1 key ... > */
    0, 0, 0, 0, 0, 0, 0, 0,
    0, /* < ... F10 */
    0, /* 69 - Num lock*/
    0, /* Scroll Lock */
    0, /* Home key */
    0, /* Up Arrow */
    0, /* Page Up */
    0,
    0, /* Left Arrow */
    0,
    0, /* Right Arrow */
    0,
    0, /* 79 - End key*/
    0, /* Down Arrow */
    0, /* Page Down */
    0, /* Insert Key */
    0, /* Delete Key */
    0, 0, 0,
    0, /* F11 Key */
    0, /* F12 Key */
    0, /* All other keys are undefined */
};

/* Handles the keyboard interrupt */
void keyboardHandler()
{
    [[maybe_unused]] auto& log = Serial::get();

    auto& queue = EventQueue::getInstance();

    /* Read from the keyboard's data buffer */


    if (const u32 scancode = inb(KEYB_DATA); scancode & 0x80) // key down event
    {
        const auto ku = event_t{KEY_UP, event_data_t{scancode-0x80, 0}};
        queue.addEvent(ku);
        /* You can use this one to see if the user released the
        *  shift, alt, or control keys... */
    }
    else // key release event
    {
        const auto kd = event_t{KEY_DOWN, event_data_t{scancode, 0}};
        queue.addEvent(kd);
    }
}

EventQueue::EventQueue()
{
    auto& log = Serial::get();
    log.log("Initialising EventQueue");
    instance = this;
    _unread_counter = 0;
    _write_index = 0;
    _read_index = 0;

    for (auto & i : _event_queue)
    {
        i = event_t{NULL_EVENT, event_data_t{0, 0}};
    }
    log.log("EventQueue initialised");
}

EventQueue::~EventQueue()
{
    instance = nullptr;
    _write_index = 0;
}

EventQueue& EventQueue::getInstance()
{
    return *instance;
}

void EventQueue::addEvent(const event_t& event)
{
    [[maybe_unused]] auto & log = Serial::get();


    // log.write("Adding event.\n");
    // log.write("type: ");
    // log.write(static_cast<int>(event.type));
    // log.write(" lower: ");
    // log.write(event.data.lower_data true);
    // log.write(" upper: ");
    // log.write(event.data.upper_data, true);
    // log.newLine();

    _event_queue[_write_index] = event;
    _write_index = (_write_index+1)%max_len;
    _unread_counter++;

}



bool EventQueue::pendingEvents() const
{
    return _unread_counter > 0;
}


event_t EventQueue::getEvent()
{
    if (_unread_counter == 0)
    {
        auto& log = Serial::get();
        log.write("Tried to get read event ahead of event queue. Returning NONE event");
        return event_t{NULL_EVENT, event_data_t{0, 0}};
    }

    const auto event_out = _event_queue[_read_index];
    _unread_counter--;
    _read_index = (_read_index+1)%max_len;
    return event_out;
}
