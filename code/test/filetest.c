#include "syscall.h"

int main()
{
    OpenFileId Fp;
    char buffer[50];
    int size;
    Create("Ftest");
    Fp = Open("Ftest");
    Write("Hello world!", 10, Fp);
    size = Read(buffer, 4, Fp);
    Close(Fp);
    Exit(0);
}
