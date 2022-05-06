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

char toupper(char c) {
    if(c>='a' && c<='z') {
        c+='A'-'a';
    }
    return c;
}

//-------------------------------------------------------------------------
// delete all char 'c' in str 
//
//-------------------------------------------------------------------------
char buf[128];
char *EraseStrChar(char *str, char c)
{
    int i = 0;
    //printf("str=%s\n",str);
    while (*str != '\0')
    {
        if (*str != c)
        {
            buf[i] = *str;
            str++;
            i++;
        } //if
        else
            str++;
    } //while
    buf[i] = '\0';
    //printf("buf=%s\n",buf);
    return buf;
}
//---------------------------------------------------------------
//
// Convert integer to str
//
//----------------------------------------------------------------
char istr[10] = "";
char *itostr(int num)
{
    int n;
    char ns[10];
    int k = 0;
    while (true)
    {
        n = num % 10;
        ns[k] = 0x30 + n;
        k++;
        num = num / 10;
        if (num == 0)
            break;
    }
    for (int i = 0; i < k; i++)
        istr[i] = ns[k - i - 1];

    //printf("str=%s\n",str);
    return istr;
}
//---------------------------------------------------------------
//
// set a string to fix length, with blank append at the end of it,
// or truncate it to the fix length
//
//
// parametors:
// str, len
//
//---------------------------------------------------------------
char strSpace[10];
char *setStrLength(char *str, int len)
{
    //printf("str = %s\n",str);
    int strLength = strlen(str);

    if (strLength >= len)
    {
        for (int i = 0; i < len; i++, str++)
            strSpace[i] = *str;
    }
    else
    {
        for (int i = 0; i < strLength; i++, str++)
            strSpace[i] = *str;

        for (int i = strLength; i < len; i++)
            strSpace[i] = ' ';
    }
    strSpace[len] = '\0';
    //printf("strSpace = %s, len=%d\n",strSpace,strlen(strSpace));
    return strSpace;
}

//-----------------------------------------------------------------
//
// divide an integer with ",", such as int (1,234,567) to char * (1,234,567)
//
//-----------------------------------------------------------------
char numStr[10];
char *numFormat(int num)
{
    numStr[0] = '\0';
    char nstr[10] = "";
    int k = 0;
    while (true)
    {
        nstr[k++] = num % 10 + 0x30;
        num = num / 10;
        if (num == 0)
            break;
    }
    nstr[k] = '\0';

    int j = k - 1;
    if (strlen(nstr) >= 4 && strlen(nstr) <= 6)
        j = k;
    else if (strlen(nstr) >= 7 && strlen(nstr) <= 9)
        j = k + 1;
    else if (strlen(nstr) >= 10 && strlen(nstr) <= 12)
        j = k + 2;

    numStr[j + 1] = '\0';
    for (int i = 0; i < k; i++)
    {
        if (i > 0 && i % 3 == 0)
        {
            numStr[j--] = ',';
            numStr[j--] = nstr[i];
        }
        else
            numStr[j--] = nstr[i];
    }
    //printf("numStr= %s\n",numStr);
    return numStr;
}
//-------------------------------------------------------

extern void Print(char *name);
extern void NAppend(char *from, char *to);
extern void Append(char *from, char *to, int half);
extern void Copy(char *from, char *to);
extern void PerformanceTest();

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
	            machine->WriteRegister(2, exitCode);
	            currentThread->SetExitCode(exitCode);
	            // 父进程的退出码特殊标记，由 Join 的实现方式决定
	            // if(exitCode == 99)
		        //     scheduler->EmptyList(scheduler->GetTerminatedList());

	            // delete currentThread->space;
	            currentThread->Finish();
                printf("**************************\n");
                AdvancePC();
	            break;
            }
            case SC_Exec: {
                printf("Syscall exception type: SC_Exec, CurrentThreadId: %d\n", currentThread->GetSpaceId());
                //DEBUG('f',"Execute system call Exec()\n");
                //read argument (i.e. filename) of Exec(filename)
                char filename[128]; 
                int addr=machine->ReadRegister(4); 
                int ii=0;
                //read filename from mainMemory
                do{
                    machine->ReadMem(addr+ii,1,(int *)&filename[ii]);
                } while (filename[ii++]!='\0');
                
                //---------------------------------------------------------
                //
                // inner commands --begin
                //
                //---------------------------------------------------------
                //printf("Call Exec(%s)\n",filename);
                if (filename[0] == 'l' && filename[1] == 's') //ls
                { 
                    printf("File(s) on Nachos DISK:\n");
                    fileSystem->List();
                    machine->WriteRegister(2,127); //
                    AdvancePC(); 
                    break; 
                } 
                else if (filename[0] == 'r' && filename[1] == 'm') //rm file
                { 
                    char fn[128];
                    strncpy(fn,filename,strlen(filename) - 5);
                    fn[strlen(filename) - 5] = '\0';
                    fn[0] = ' ';
                    fn[1] = ' ';
                    char *file = EraseStrChar(fn,' ');
                    if (file != NULL && strlen(file) >0)
                    {
                        fileSystem->Remove(file);
                        machine->WriteRegister(2,127);
                    }
                    else
                    {
                        printf("Remove: invalid file name.\n");
                        machine->WriteRegister(2,-1);
                    }
                    AdvancePC(); 
                    break; 
                } //rm
                else if (filename[0] == 'c' && filename[1] == 'a' && filename[2] == 't') //cat file
                {
                    char fn[128];
                    strncpy(fn,filename,strlen(filename) - 5);
                    fn[strlen(filename) - 5] = '\0';
                    fn[0] = ' ';
                    fn[1] = ' ';
                    fn[2] = ' ';
                    char *file = EraseStrChar(fn,' ');
                    //printf("filename=%s, fn=%s, file=%s\n",filename,fn, file);
                    if (file != NULL && strlen(file) >0)
                    {
                        Print(file);
                        machine->WriteRegister(2,127);
                    }
                    else
                    {
                        printf("Cat: file not exists.\n");
                        machine->WriteRegister(2,-1);
                    }
                    AdvancePC(); 
                    break; 
                } 
                else if (filename[0] == 'c' && filename[1] == 'f') //create a nachos file
                { //create file
                    char fn[128];
                    strncpy(fn,filename,strlen(filename) - 5);
                    fn[strlen(filename) - 5] = '\0';
                    //printf("filename=%s, fn=%s\n",filename,fn);
                    fn[0] = ' ';
                    fn[1] = ' ';
                    
                    char *file = EraseStrChar(fn,' ');
                    if (file != NULL && strlen(file) >0)
                    {
                        fileSystem->Create(file,0);
                        machine->WriteRegister(2,127);
                    }
                    else
                    {
                        printf("Create: file already exists.\n");
                        machine->WriteRegister(2,-1);
                    }
                    AdvancePC(); 
                    break;
                }   
                else if (filename[0] == 'c' && filename[1] == 'p') //cp source dest
                {
                    char fn[128];
                    strncpy(fn,filename,strlen(filename) - 5);
                    fn[strlen(filename) - 5] = '\0';
                    fn[0] = '#';
                    fn[1] = '#';
                    fn[2] = '#';
                    char source[128];
                    char dest[128];
                    int startPos = 3;
                    int j = 0;
                    int k = 0;
                    for (int i = startPos; i < 128 /*, fn[i] != '\0'*/;i++)
                    if (fn[i] != ' ')
                        source[j++] = fn[i];
                    else 
                        break;
                    source[j] = '\0';
                    j++; 
                    //printf("j = %d\n",j);
                    
                    for (int i = startPos + j; i < 128 /*,fn[i] != '\0'*/;i++)
                    if (fn[i] != ' ')
                        dest[k++] = fn[i];
                    else 
                        break; 
                    dest[k] = '\0';
                    
                    if (source == NULL || strlen(source) <= 0)
                    {
                        printf("cp: Source file not exists.\n");
                        machine->WriteRegister(2,-1);
                        AdvancePC();
                        break; 
                    }
                    if (dest != NULL && strlen(dest) > 0)
                    {
                        NAppend(source, dest);
                        machine->WriteRegister(2,127);
                    }
                    else
                    {
                        printf("cp: Missing dest file.\n");
                        machine->WriteRegister(2,-1);
                    }
                    AdvancePC(); 
                    break; 
                }
                //uap source(Unix) dest(nachos)
                else if ((filename[0] == 'u' && filename[1] == 'a' && filename[2] == 'p') 
                    || (filename[0] == 'u' && filename[1] == 'c' && filename[2] == 'p')) 
                {
                    char fn[128];
                    strncpy(fn,filename,strlen(filename) - 5);
                    fn[strlen(filename) - 5] = '\0';
                    fn[0] = '#';
                    fn[1] = '#';
                    fn[2] = '#';
                    fn[3] = '#';
                    char source[128];
                    char dest[128];
                    int startPos = 4;
                    int j = 0;
                    int k = 0;
                    for (int i = startPos; i < 128/*, fn[i] != '\0'*/;i++)
                    if (fn[i] != ' ')
                    source[j++] = fn[i];
                    else 
                    break;
                    source[j] = '\0';
                    j++; 
                    for (int i = startPos + j; i < 128/*,fn[i] != '\0'*/;i++)
                        if (fn[i] != ' ')
                            dest[k++] = fn[i];
                        else 
                            break; 
                    dest[k] = '\0';
                    
                    if (source == NULL || strlen(source) <= 0)
                    {
                        printf("uap or ucp: Source file not exists.\n");
                        machine->WriteRegister(2,-1);
                        AdvancePC(); 
                        break; 
                    }
                    if (dest != NULL && strlen(dest) > 0)
                    {
                        if (filename[0] == 'u' && filename[1] == 'c' && filename[2] == 'p')
                            Append(source, dest,0); //append dest file at the end of source file
                        else
                            Copy(source, dest); //ucp
                        machine->WriteRegister(2,127);
                    }
                    else
                    {
                        printf("uap or ucp: Missing dest file.\n");
                        machine->WriteRegister(2,-1);
                    }
                    AdvancePC(); 
                    break; 
                }
                //nap source dest
                else if (filename[0] == 'n' && filename[1] == 'a' && filename[2] == 'p')
                {
                    char fn[128];
                    strncpy(fn,filename,strlen(filename) - 5);
                    fn[strlen(filename) - 5] = '\0';
                    fn[0] = '#';
                    fn[1] = '#';
                    fn[2] = '#';
                    fn[3] = '#';
                    //char *file = EraseStrSpace(fn, ' ');
                    char source[128];
                    char dest[128];
                    int j = 0;
                    int k = 0;
                    int startPos = 4;
                    for (int i = startPos; i < 128/*, fn[i] != '\0'*/;i++)
                        if (fn[i] != ' ')
                            source[j++] = fn[i];
                        else 
                            break;
                    source[j] = '\0';
                    j++; 
                    //printf("j = %d\n",j);
                    
                    for (int i = startPos + j; i < 128/*,fn[i] != '\0'*/;i++)
                        if (fn[i] != ' ')
                            dest[k++] = fn[i];
                        else 
                            break; 
                    dest[k] = '\0';
                        
                    if (source == NULL || strlen(source) <= 0)
                    {
                        printf("nap: Source file not exists.\n");
                        machine->WriteRegister(2,-1);
                        AdvancePC(); 
                        break; 
                    }
                    if (dest != NULL && strlen(dest) > 0)
                    {
                        NAppend(source, dest);
                        machine->WriteRegister(2,127);
                    }
                    else
                    {
                        printf("ap: Missing dest file.\n");
                        machine->WriteRegister(2,-1);
                    }
                    AdvancePC(); 
                    break; 
                }
                //rename source dest
                else if (filename[0] == 'r' && filename[1] == 'e' && filename[2] == 'n')
                {
                    char fn[128];
                    strncpy(fn,filename,strlen(filename) - 5);
                    fn[strlen(filename) - 5] = '\0';
                    fn[0] = '#';
                    fn[1] = '#';
                    fn[2] = '#';
                    fn[3] = '#';
                    char source[128];
                    char dest[128];
                    int j = 0;
                    int k = 0;
                    int startPos = 4;
                    for (int i = startPos; i < 128/*, fn[i] != '\0'*/;i++)
                        if (fn[i] != ' ')
                            source[j++] = fn[i];
                        else 
                            break;
                    source[j] = '\0';
                    j++; 
                    //printf("j = %d\n",j);
                    
                    for (int i = startPos + j; i < 128/*,fn[i] != '\0'*/;i++)
                        if (fn[i] != ' ')
                            dest[k++] = fn[i];
                        else 
                            break; 
                    dest[k] = '\0';
                    
                    if (source == NULL || strlen(source) <= 0)
                    {
                        printf("rename: Source file not exists.\n");
                        machine->WriteRegister(2,-1);
                        AdvancePC(); 
                        break; 
                    }
                    if (dest != NULL && strlen(dest) > 0)
                    {
                        fileSystem->Rename(source, dest);
                        machine->WriteRegister(2,127);
                    }
                    else
                    {
                        printf("rename: Missing dest file.\n");
                        machine->WriteRegister(2,-1);
                    }
                    AdvancePC(); 
                    break; 
                }
                else if (strstr(filename,"format") != NULL) //format
                { 
                    printf("strstr(filename,\"format\"=%s \n",strstr(filename,"format"));
                    printf("WARNING: Format Nachos DISK will erase all the data on it.\n");
                    printf("Do you want to continue (y/n)? ");
                    char ch;
                    while (true)
                    {
                        ch = getchar();
                        if (toupper(ch) == 'Y' || toupper(ch) == 'N')
                        break;
                    } //while
                    if (toupper(ch) == 'N')
                    {
                        machine->WriteRegister(2,127); //
                        AdvancePC(); 
                        break; 
                    }
                    
                    printf("Format the DISK and create a file system on it.\n");
                    fileSystem->FormatDisk(true);
                    machine->WriteRegister(2,127); //
                    AdvancePC(); 
                    break; 
                } 
                else if (strstr(filename,"fdisk") != NULL) //fdisk
                { 
                    fileSystem->Print();
                    machine->WriteRegister(2,127); //
                    AdvancePC(); 
                    break; 
                } 
                else if (strstr(filename,"perf") != NULL) //Performance
                { 
                    PerformanceTest();
                    machine->WriteRegister(2,127); //
                    AdvancePC(); 
                    break; 
                } 
                else if (filename[0] == 'p' && filename[1] == 's') //ps
                {
                    scheduler->PrintThreads();
                    machine->WriteRegister(2,127); //
                    AdvancePC(); 
                    break; 
                }
                else if (strstr(filename,"help") != NULL) //fdisk
                {
                    printf("Commands and Usage:\n");
                    printf("ls : list files on DISK.\n");
                    printf("fdisk : display DISK information.\n");
                    printf("format : format DISK with creating a file system on it.\n");
                    printf("performence : test DISK performence.\n");
                    printf("cf name : create a file \"name\" on DISK.\n");
                    printf("cp source dest : copy Nachos file \"source\" to \"dest\".\n");
                    printf("nap source dest : Append Nachos file \"source\" to \"dest\"\n");
                    printf("ucp source dest : Copy Unix file \"source\" to Nachos file \"dest\"\n");
                    printf("uap source dest : Append Unix file \"source\" to Nachos file \"dest\"\n");
                    printf("cat name : print content of file \"name\".\n");
                    printf("rm name : remove file \"name\".\n"); 
                    printf("rename source dest: Rename Nachos file \"source\" to \"dest\".\n");
                    printf("ps : display the system threads.\n"); 
                    //-----------------------------------------------------------
                    
                    machine->WriteRegister(2,127); //
                    AdvancePC(); 
                    break; 
                } 
                else //check if the parameter of exec(file), i.e file, is valid
                {
                    if (strchr(filename,' ') != NULL || strstr(filename,".noff") == NULL) 
                    //not an inner command, and not a valid Nachos executable, then return 
                    {
                        machine->WriteRegister(2,-1); 
                        AdvancePC(); 
                        break;
                    }
                }
                // inner commands --end
                //
                //---------------------------------------------------------
                //---------------------------------------------------------
                //
                // loading an executable and execute it
                //
                //---------------------------------------------------------
                //call open() in FILESYS, see filesys.h
                OpenFile *executable = fileSystem->Open(filename); 
                if (executable == NULL) {
                    //printf("\nUnable to open file %s\n", filename);
                    DEBUG('f',"\nUnable to open file %s\n", filename);
                    machine->WriteRegister(2,-1); 
                    AdvancePC(); 
                    break;
                    //return;
                }
                
                //new address space
                AddrSpace *space = new AddrSpace(executable); 
                delete executable; // close file
                
                DEBUG('H',"Execute system call Exec(\"%s\"), it's SpaceId(pid) = %d \n",filename,space->GetSpaceId());
                //new and fork thread
                char *forkedThreadName = filename;
                
                //------------------------------------------------
                char *fname=strrchr(filename,'/');
                if (fname != NULL) // there exists "/" in filename 
                    forkedThreadName=strtok(fname,"/"); 
                //----------------------------------------------- 
                Thread *thread = new Thread(forkedThreadName);
                //printf("exec -- new thread pid =%d\n",space->getSpaceID());
                thread->Fork(StartProcess, space->GetSpaceId());
                thread->space = space;
                printf("user process \"%s(%d)\" map to kernel thread \" %s \"\n",filename,space->GetSpaceId(),forkedThreadName);
                
                //return spaceID
                machine->WriteRegister(2,space->GetSpaceId());
                //printf("Exec()--space->getSpaceID()=%d\n",space->getSpaceID());
 
                //=========================================================
                //run the new thread,
                //otherwise, this process will not execute in order to release its memory, 
                //thread "main" may continue to create new processes,
                //and will not have enough memory for new processes 
                
                currentThread->Yield();
                
                //but introduce another problem:
                // when Joiner waits for a Joinee, the joinee maybe finish before Joiner call Join(),
                // but Joinee go to sleep after calling Finish()
                //
                //============================================================
                //advance PC
                AdvancePC(); 
                break;
            }
            case SC_Join: {
                printf("Syscall exception type: SC_Join, CurrentThreadId: %d\n", currentThread->GetSpaceId());
                int spaceId = machine->ReadRegister(4);     // i.e. ThreadId or SpaceId
                // printf("before join******************************\n");
                currentThread->Join(spaceId);
                // printf("after join*******************************\n");
                //返回 Joinee 的退出码 waitProcessExitCode
                machine->WriteRegister(2, currentThread->GetWaitProcessExitCode());
                AdvancePC();
                break;
            }
            case SC_Create:{
                int addr = machine->ReadRegister(4);
                char filename[128];
                for(int i = 0; i < 128; i++){
                    machine->ReadMem(addr+i,1,(int *)&filename[i]);
                    if(filename[i] == '\0') break;
                }
                if(!fileSystem->Create(filename,0)) printf("create file %s failed!\n",filename);
                else printf("create file %s succeed!\n",filename);
                AdvancePC();
                break;
            }

            case SC_Open:{
                int addr = machine->ReadRegister(4), fileId;
                char filename[128];
                for(int i = 0; i < 128; i++){
                    machine->ReadMem(addr+i,1,(int *)&filename[i]);
                    if(filename[i] == '\0') break;
                }
                OpenFile *openfile = fileSystem->Open(filename);
                if(openfile == NULL) {
                    printf("File \"%s\" not Exists, could not open it.\n",filename);
                    fileId = -1;
                }
                else{
                    fileId = currentThread->space->getFileDescriptor(openfile);
                    if(fileId < 0) printf("Too many files opened!\n");
                    else printf("file:\"%s\" open succeed, the file id is %d\n",filename,fileId);
                }
                machine->WriteRegister(2,fileId);
                AdvancePC();
                break;
            }

            case SC_Read:{
                // 读取寄存器信息
                int addr = machine->ReadRegister(4);
                int size = machine->ReadRegister(5);       // 字节数
                int fileId = machine->ReadRegister(6);      // fd
        
                // 打开文件
                OpenFile *openfile = currentThread->space->getFileId(fileId);

                // 打开文件读取信息
                char buffer[size+1];
                int readnum = 0;
                if(fileId == 0) readnum = openfile->ReadStdin(buffer,size);
                else readnum = openfile->Read(buffer,size);

                // printf("readnum:%d,fileId:%d,size:%d\n",readnum,fileId,size);
                // printf("buffer = %s\n", buffer);
                for(int i = 0; i < readnum; i++)
                    machine->WriteMem(addr,1,buffer[i]);
                buffer[readnum] = '\0';

                for(int i = 0; i < readnum; i++)
                    if(buffer[i] >= 0 && buffer[i] <= 9) buffer[i] = buffer[i]+0x30;
                char *buf = buffer;
                if(readnum > 0){
                    if(fileId != 0)
                        printf("Read file (%d) succeed! the content is \"%s\", the length is %d\n",fileId,buf,readnum);
                }
                else printf("\nRead file failed!\n");
                machine->WriteRegister(2,readnum);
                AdvancePC();
                break;
            }

            case SC_Write:{
                // 读取寄存器信息
                int addr = machine->ReadRegister(4);       // 写入数据
                int size = machine->ReadRegister(5);       // 字节数
                int fileId = machine->ReadRegister(6);      // fd
                
                // 创建文件
                OpenFile *openfile = new OpenFile(fileId);
                ASSERT(openfile != NULL);

                // 读取具体写入的数据
                char buffer[128];
                for(int i = 0; i < size; i++){
                    machine->ReadMem(addr+i,1,(int *)&buffer[i]);
                    if(buffer[i] == '\0') break;
                }
                buffer[size] = '\0';


                // 打开文件
                openfile = currentThread->space->getFileId(fileId);
                if(openfile == NULL) {
                    printf("Failed to Open file \"%d\".\n",fileId);
                    AdvancePC();
                    break;
                }
                if(fileId == 1 || fileId == 2){
                    openfile->WriteStdout(buffer,size);
                    // delete []buffer;
                    AdvancePC();
                    break;
                }

                // 写入数据
                int writePos = openfile->Length();
                openfile->Seek(writePos);

                // 在 writePos 后面进行数据添加
                int writtenBytes = openfile->Write(buffer,size);
                if(writtenBytes == 0) printf("Write file failed!\n");
                else if(fileId != 1 & fileId != 2)
                    printf("\"%s\" has wrote in file %d succeed!\n",buffer,fileId);
                AdvancePC();
                break;
            }

            case SC_Close:{
                int fileId = machine->ReadRegister(4);
                OpenFile *openfile = currentThread->space->getFileId(fileId);
                if(openfile != NULL) {
                    openfile->WriteBack(); // 将文件写入DISK
                    delete openfile;
                    currentThread->space->releaseFileDescriptor(fileId);
                    printf("File %d closed succeed!\n",fileId);
                }
                else printf("Failed to Close File %d.\n",fileId);
                AdvancePC();
                break;
            }

            case SC_Fork:{
                printf("Syscall exception type: SC_Fork, CurrentThreadId: %d\n",currentThread->GetSpaceId());
                
                int functionPC = machine->ReadRegister(4);

                // OpenFile *executable = fileSystem->Open("../test/fork.noff");
                // if(executable == NULL) {
                //     printf("Unable to open file\n");
                //     return ;
                // }
                // // new address space
                // AddrSpace *space = new AddrSpace(executable);
                // // space->Print();
                // delete executable;

                AddrSpace *space = currentThread->space;
                // space->pageMap->Print();
                space->Print();

                Thread *thread = new Thread("forked thread");
                thread->space = space;
                thread->space->Print();
                // thread->space->pageMap->Print();
                printf("thread->spaceId = %d\n\n", thread->GetSpaceId());

                printf("functionPC = %d***********************\n", functionPC);
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
        currentThread->space->Print();
        printf("current thread->spaceId = %d\n\n", currentThread->GetSpaceId());
	    ASSERT(FALSE);
    }
}

