#include "syscall.h"

int main()
{
    OpenFileId Fp;
    char buffer[50];
    int size;
    Create("Ftest");
    Fp = Open("Ftest");
    Write("Hello!", 6, Fp);
    Close(Fp);
    
    Fp = Open("Ftest");
    size = Read(buffer, 6, Fp);
    Close(Fp);
    Exit(0);
}
