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
StartThread(int functionPC) {
    currentThread->space->InitRegisters();   // set the initial register values
    machine->WriteRegister(PCReg, functionPC);
    machine->WriteRegister(NextPCReg, functionPC+4);
    currentThread->space->RestoreState();   // load page table register
    machine->Run();                         // jump to the user progam
    ASSERT(FALSE);                          // machine->Run never returns;
                    // the address space exits by doing the syscall "exit"
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    // currentThread->space->Print();
    // printf("current thread->spaceId = %d\n\n", currentThread->GetSpaceId());

    if (which == SyscallException) {
        switch(type) {
            case SC_Halt: {
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            }
            case SC_Exit:{
                printf("Syscall exception type: SC_Exit, CurrentThreadId: %d\n", currentThread->GetSpaceId());
	            int exitCode = machine->ReadRegister(4);
	            // machine->WriteRegister(2, exitCode);
	            currentThread->SetExitCode(exitCode);
	            // 父进程的退出码特殊标记，由 Join 的实现方式决定
	            // if(exitCode == 99)
		        //     scheduler->EmptyList(scheduler->GetTerminatedList());

	            //delete currentThread->space;
	            currentThread->Finish();
                AdvancePC();
	            break;
            }
            case SC_Exec: {
                printf("Syscall exception type: SC_Exec, CurrentThreadId: %d\n", currentThread->GetSpaceId());
                
                if(currentThread->childProcessNumber == MaxChildProcess) {
                    printf("Current thread is full of children. Exec failed!\n");
                    AdvancePC();
                    break;
                }
                
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
                    printf("Unable to open file %s\n", filename);
                    return ;
                }
                // new address space
                AddrSpace *space = new AddrSpace(executable);
                // space->Print();
                delete executable;

                Thread *thread = new Thread(filename);
                thread->space = space;
                thread->Fork(StartProcess,space->GetSpaceId());

                // set childProcessSpaceId and fatherProcessSpaceId
                for(int j = 0; j < MaxChildProcess; j++) {
                    if(currentThread->childProcessSpaceId[j] == 0) {
                        currentThread->childProcessSpaceId[j] = space->GetSpaceId();
                        currentThread->childProcessNumber++;
                        break;
                    }
                }
                thread->fatherProcessSpaceId = currentThread->GetSpaceId();

                // return SpaceId
                machine->WriteRegister(2, space->GetSpaceId());
                AdvancePC();
                break;
            }
            case SC_Join: {
                printf("Syscall exception type: SC_Join, CurrentThreadId: %d\n", currentThread->GetSpaceId());
                int spaceId = machine->ReadRegister(4);     // i.e. ThreadId or SpaceId
                currentThread->Join(spaceId);
                //返回 Joinee 的退出码 waitProcessExitCode
                machine->WriteRegister(2, currentThread->GetWaitProcessExitCode());
                AdvancePC();
                break;
            }
            case SC_Create:{
                printf("Syscall exception type: SC_Create, CurrentThreadId: %d\n",currentThread->GetSpaceId());
                // read argument
                char filename[50];
                int addr = machine->ReadRegister(4);
                int i = 0;
                do {    // read filename from main memory
                    machine->ReadMem(addr+i, 1, (int *)&filename[i]);
                } while(filename[i++] != '\0');

                int fileDescriptor = OpenForWrite(filename);
                if(fileDescriptor == -1) {
                    printf("create file %s failed!\n", filename);
                }
                else {
                    printf("create file %s succeed, the file id is %d\n", filename, fileDescriptor);
                }
                Close(fileDescriptor);
                // machine->WriteRegister(2,fileDescriptor);
                AdvancePC();
                break;
            }
            case SC_Open:{
                printf("Syscall exception type: SC_Open, CurrentThreadId: %d\n",currentThread->GetSpaceId());
                // read argument
                char filename[50];
                int addr = machine->ReadRegister(4);
                int i = 0;
                do {    // read filename from main memory
                    machine->ReadMem(addr+i, 1, (int *)&filename[i]);
                } while(filename[i++] != '\0');

                int fileDescriptor = OpenForWrite(filename);
                if(fileDescriptor == -1) {
                    printf("Open file %s failed!\n",filename);
                }
                else {
                    printf("Open file %s succeed, the file id is %d\n", filename, fileDescriptor);
                }       
                machine->WriteRegister(2,fileDescriptor);
                AdvancePC();
                break;
            }
            case SC_Read:{
                printf("Syscall exception type: SC_Read, CurrentThreadId: %d\n",currentThread->GetSpaceId());
                // 读取寄存器信息
                int addr = machine->ReadRegister(4);
                int size = machine->ReadRegister(5);        // 字节数
                int fileId = machine->ReadRegister(6);      // fd

                // 打开文件读取信息
                char buffer[size+1];
                OpenFile *openfile = new OpenFile(fileId);
                int readnum = openfile->Read(buffer,size);

                for(int i = 0; i < size; i++) {
                    if(!machine->WriteMem(addr, 1, buffer[i]))
                        printf("This is something Wrong.\n");
                }
                buffer[size] = '\0';
                printf("read succeed, the content is \"%s\", the length is %d\n", buffer, size);
                machine->WriteRegister(2,readnum);
                AdvancePC();
                break;
            }
            case SC_Write:{
                printf("Syscall exception type: SC_Write, CurrentThreadId: %d\n",currentThread->GetSpaceId());
                // 读取寄存器信息
                int addr = machine->ReadRegister(4);
                int size = machine->ReadRegister(5);        // 字节数
                int fileId = machine->ReadRegister(6);      // fd
    
                // 打开文件
                OpenFile *openfile = new OpenFile(fileId);
                ASSERT(openfile != NULL);

                // 读取具体数据
                char buffer[128];
                for(int i = 0; i < size; i++){
                    machine->ReadMem(addr+i, 1, (int *)&buffer[i]);
                    if(buffer[i] == '\0') break;
                }
                buffer[size] = '\0';

                // 写入数据
                int writePos;
                if(fileId == 1)
                    writePos = 0;
                else 
                    writePos = openfile->Length();
                // 在 writePos 后面进行数据添加
                int writtenBytes = openfile->WriteAt(buffer, size, writePos);
                if(writtenBytes == 0)
                    printf("write file failed!\n");
                else
                    printf("\"%s\" has wrote in file %d succeed!\n", buffer, fileId);
                AdvancePC();
                break;
            }
            case SC_Close:{
                printf("Syscall exception type: SC_Close, CurrentThreadId: %d\n",currentThread->GetSpaceId());
                int fileId = machine->ReadRegister(4);
                Close(fileId);
                printf("File %d closed succeed!\n", fileId);
                AdvancePC();
                break;
            }
            case SC_Fork:{
                printf("Syscall exception type: SC_Fork, CurrentThreadId: %d\n",currentThread->GetSpaceId());
                int functionPC = machine->ReadRegister(4);

                AddrSpace *space = currentThread->space;
                // space->Print();

                Thread *thread = new Thread("forked thread");
                thread->space = space;
                // thread->space->Print();
                printf("thread->spaceId = %d\n\n", thread->GetSpaceId());

                // printf("functionPC = %d***********************\n", functionPC);
                thread->Fork(StartThread, functionPC);
                // currentThread->Yield();

                AdvancePC();
                break;
            }
            case SC_Yield:{
                printf("Syscall exception type: SC_Yield, CurrentThreadId: %d\n",currentThread->GetSpaceId());
                AdvancePC();
                currentThread->Yield();
                break;
            }
            default: {
                printf("Unexpected syscall %d %d\n", which, type);
                ASSERT(FALSE);
            }
        }
    } else {
	    printf("Unexpected user mode exception %d %d\n", which, type);
        // currentThread->space->Print();
        // printf("current thread->spaceId = %d\n\n", currentThread->GetSpaceId());
	    ASSERT(FALSE);
    }
}

