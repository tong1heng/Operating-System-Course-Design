// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void AdvancePC() {
    machine->WriteRegister(PCReg, machine->ReadRegister(PCReg) + 4);
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
}

void
StartProcess(int spaceId) {
    currentThread->space->InitRegisters();   // set the initial register values
    currentThread->space->RestoreState();   // load page table register
    machine->Run();                         // jump to the user progam
    ASSERT(FALSE);                          // machine->Run never returns;
                    // the address space exits by doing the syscall "exit"
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if (which == SyscallException) {
        switch(type) {
            case SC_Halt: {
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            }
            case SC_Exec: {
                printf("Syscall execption type: SC_Exec, CurrentThreadId: %d\n",currentThread->GetSpaceId());
                // read argument
                char filename[50];
                int addr = machine->ReadRegister(4);
                int i = 0;
                do {    // read filename from main memory
                    machine->ReadMem(addr+i, 1, (int *)&filename[i]);
                } while(filename[i++] != '\0');
                printf("Exec(%s):\n", filename);

                OpenFile *executable = fileSystem->Open(filename);
                if(executable == NULL) {
                    printf("Unable to open file %s\n",filename);
                    return ;
                }
                //
                AddrSpace *space = new AddrSpace(executable);
                delete executable;

                Thread *thread = new Thread(filename);
                thread->space = space;
                thread->Fork(StartProcess,space->GetSpaceId());

                // return SpaceId
                machine->WriteRegister(2, space->GetSpaceId());
                AdvancePC();
                break;
            }
            case SC_Join: {
                printf("Syscall execption type: SC_Join, CurrentThreadId: %d\n",currentThread->GetSpaceId());
                int spaceId = machine->ReadRegister(4);     // i.e. ThreadId or SpaceId
                // printf("before join******************************\n");
                currentThread->Join(spaceId);
                // printf("after join*******************************\n");
                //返回 Joinee 的退出码 waitProcessExitCode
                machine->WriteRegister(2, currentThread->GetWaitProcessExitCode());
                AdvancePC();
                break;
            }
            case SC_Exit:{
                printf("Syscall execption type: SC_Exit, CurrentThreadId: %d\n",currentThread->GetSpaceId());
	            int exitCode = machine->ReadRegister(4);
	            machine->WriteRegister(2,exitCode);
	            currentThread->SetExitCode(exitCode);
	            // 父进程的退出码特殊标记，由 Join 的实现方式决定
	            if(exitCode == 99)
		            scheduler->EmptyList(scheduler->GetTerminatedList());
	            delete currentThread->space;
	            currentThread->Finish();
                AdvancePC();
	            break;
            }
            default: {
                printf("Unexpected syscall %d %d\n", which, type);
                ASSERT(FALSE);
            }
        }
    } else {
	    printf("Unexpected user mode exception %d %d\n", which, type);
	    ASSERT(FALSE);
    }
}

