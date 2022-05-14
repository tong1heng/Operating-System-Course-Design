#include "syscall.h"

int main() {
    // SpaceId pid = Exec("../test/exit.noff");
    SpaceId pid = Exec("../test/yield.noff");
    Join(pid);
    Exit(0);
}
