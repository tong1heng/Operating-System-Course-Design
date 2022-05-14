#include "syscall.h"

void func() {
    int a = 123;
    int b = 456;
    Exec("../test/halt.noff");
    Exit(100);
}

int main() {
    Fork(func);
    Exit(0);
}
