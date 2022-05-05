#include "syscall.h"

int main()
{
    OpenFileId Fp;
    char buffer[50];
    int size;
    Fp = Open("Ftest");
    size = Read(buffer, 6, Fp);
    Close(Fp);
    Exit(0);
}