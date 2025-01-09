//
// Created by artypoole on 09/01/25.
//

extern "C" int write(int fd, const char* buf, unsigned long count); // Declaration of syscall
extern "C" void exit(int status);

int main() {
    const char* message = "Hello, World!\n";
    write(1, message, 13); // Write to stdout
    exit(0); // Exit with status 0
    return 0; // Not reached
}