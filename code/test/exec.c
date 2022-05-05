#include "syscall.h"

int main()
{
	int a = 100;
	int b = 0;
	Exec("../test/halt.noff");
    Halt();
}
