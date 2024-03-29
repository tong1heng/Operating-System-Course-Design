// directory.cc 
//	Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//	Fixing this is one of the parts to the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "directory.h"

#define NumDirEntries 		10
#define MaxDirLength        10
#define PathMaxLen          19

//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------

Directory::Directory(int size)
{
    table = new DirectoryEntry[size];
    tableSize = size;
    for (int i = 0; i < tableSize; i++)
	table[i].inUse = FALSE;
}

//----------------------------------------------------------------------
// Directory::~Directory
// 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{ 
    delete [] table;
} 

//----------------------------------------------------------------------
// Directory::FetchFrom
// 	Read the contents of the directory from disk.
//
//	"file" -- file containing the directory contents
//----------------------------------------------------------------------

void
Directory::FetchFrom(OpenFile *file)
{
    (void) file->ReadAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------

void
Directory::WriteBack(OpenFile *file)
{
    (void) file->WriteAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::FindIndex
// 	Look up file name in directory, and return its location in the table of
//	directory entries.  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::FindIndex(char *name)
{
    for (int i = 0; i < tableSize; i++)
        if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen))
	    return i;
    return -1;		// name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::Find(char *name)
{
    int i = FindIndex(name);

    if (i != -1)
	return table[i].sector;
    return -1;
}

//----------------------------------------------------------------------
// Directory::FindDir
// 	Look up the directory according to the file path, and return 
//  the disk sector number where the parent directory file header 
//  is stored. Return -1 if certain directory on the path is 
//  invalid.
//
//	"name" -- the file name to look up(including file path)
//----------------------------------------------------------------------

int
Directory::FindDir(char *name)
{
    DEBUG('-f',"In Directory::FindDir(), name = %s\n", name);
    int sector = 1;
    OpenFile *dir_file = new OpenFile(sector);
    Directory *dir = new Directory(NumDirEntries);
    dir->FetchFrom(dir_file);

    // printf("*******************************before while\n");
    // dir->Print();
    // printf("*******************************\n");
    
    int str_pos = 0;
    int sub_str_pos = 0;
    char sub_str[MaxDirLength];

    while(str_pos < strlen(name)) {     // split path by '/'
        sub_str[sub_str_pos++] = name[str_pos++];
        if(name[str_pos] == '/') {
            sub_str[sub_str_pos] = '\0';

            // printf("-------------------------------\n");
            // dir->Print();
            // printf("-------------------------------\n");

            sector = dir->Find(sub_str);

            if(sector == -1) {
                break;
            }

            DEBUG('-f',"In while loop, sub_str = %s\n", sub_str);
            DEBUG('-f',"In while loop, sector = %d\n", sector);

            dir_file = new OpenFile(sector);
            dir = new Directory(NumDirEntries);
            dir->FetchFrom(dir_file);
            str_pos++;
            sub_str_pos = 0;
        }
    }
    return sector;
}


//----------------------------------------------------------------------
// Directory::Add
// 	Add a file into the directory.  Return TRUE if successful;
//	return FALSE if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::Add(char *name, int newSector)
{ 
    if (FindIndex(name) != -1)
	return FALSE;

    for (int i = 0; i < tableSize; i++)
        if (!table[i].inUse) {
            table[i].inUse = TRUE;
            strncpy(table[i].name, name, FileNameMaxLen); 
            table[i].sector = newSector;
        return TRUE;
	}
    return FALSE;	// no space.  Fix when we have extensible files.
}



bool
Directory::Add(char *name, int newSector, int type)
{ 
    char fileName[FileNameMaxLen + 1];
    int pos = -1;
    for(int i = strlen(name) - 1; i >= 0; i--) {
        if(name[i] == '/') {
            pos = i + 1;
            break;
        }
    }
    if(pos == -1) pos = 0;
    int j = 0;
    for(int i = pos; i < strlen(name); i++)
        fileName[j++] = name[i];
    fileName[j] = '\0';

    if (FindIndex(fileName) != -1)
	    return FALSE;

    for (int i = 0; i < tableSize; i++)
        if (!table[i].inUse) {
            table[i].inUse = TRUE;
            strncpy(table[i].path, name, PathMaxLen);
            strncpy(table[i].name, fileName, FileNameMaxLen); 
            table[i].sector = newSector;
            table[i].type = type;
        return TRUE;
	}
    return FALSE;	// no space.  Fix when we have extensible files.
}

//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return TRUE if successful;
//	return FALSE if the file isn't in the directory. 
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

bool
Directory::Remove(char *name)
{ 
    int i = FindIndex(name);

    if (i == -1)
	return FALSE; 		// name not in directory
    table[i].inUse = FALSE;
    return TRUE;	
}

//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory. 
//----------------------------------------------------------------------

// void
// Directory::List()
// {
//    for (int i = 0; i < tableSize; i++)
// 	if (table[i].inUse)
// 	    printf("%s\n", table[i].name);
// }

void
Directory::List()
{
    printf("@root\n");
    List(1);
}

void
Directory::List(int depth)
{
    for (int i = 0; i < tableSize; i++) {
        if (table[i].inUse) {
            for(int j = 0; j < depth; j++) {
                printf("|    ");
            }
            if (table[i].type == 1) {
                printf("|%s\n", table[i].name);
            }
            else {
                printf("|@%s\n", table[i].name);
                Directory *dir = new Directory(NumDirEntries);
                OpenFile *openFile = new OpenFile(table[i].sector);
                dir->FetchFrom(openFile);
                dir->List(depth + 1);
            }
        }
    }
}

//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void
Directory::Print()
{ 
    printf("Directory contents:\n");
    Print(0);
    printf("\n");
}

void
Directory::Print(int depth)
{ 
    FileHeader *hdr = new FileHeader;

    for (int i = 0; i < tableSize; i++)
	if (table[i].inUse) {
	    printf("Name: %s, Sector: %d, Path: %s, Type: %d\n", table[i].name, table[i].sector, table[i].path, table[i].type);
	    hdr->FetchFrom(table[i].sector);
	    hdr->Print();
        
        if (table[i].type == 0) {
            Directory *dir = new Directory(NumDirEntries);
            OpenFile *openFile = new OpenFile(table[i].sector);
            dir->FetchFrom(openFile);
            dir->Print(depth + 1);
        }
	}
    delete hdr;
}

//----------------------------------------------------------------------
// Directory::GetType
// 	Look up file name in directory, and return the file type. 
//  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::GetType(char *name)
{
    int i = FindIndex(name);

    if (i != -1)
	return table[i].type;
    return -1;
}

//----------------------------------------------------------------------
// Directory::IsEmpty
//  Return true if the directory is empty.
//----------------------------------------------------------------------

bool
Directory::IsEmpty()
{
    for (int i = 0; i < tableSize; i++)
        if (table[i].inUse)
	    return false;
    return true;
}