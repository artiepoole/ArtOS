//
// Created by artypoole on 22/11/24.
//

#ifndef SHELL_H
#define SHELL_H


class EventQueue;
class Terminal;

constexpr size_t cmd_buffer_size = 1024;

class Shell
{
public:
    explicit Shell(EventQueue* e);
    ~Shell();
    static Shell& get();

    static void run();

private:
    static int process_cmd();
};


inline void shell_run()
{
    Shell::get().run();
}


#endif //SHELL_H
