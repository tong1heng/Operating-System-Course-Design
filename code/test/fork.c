#include "syscall.h"

void func() {
    // Create("a.txt");
    int a = 123;
    int b = 456;
    Exit(0);
}

int main() {
    Fork(func);
    Exit(100);
    //func();
}
