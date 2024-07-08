#include "terminal.h"

static Terminal* instance{nullptr};

Terminal::Terminal()
{
    instance = this;
}

Terminal::~Terminal()
{
    instance = nullptr;
}

Terminal& Terminal::get()
{
    return *instance;
}
