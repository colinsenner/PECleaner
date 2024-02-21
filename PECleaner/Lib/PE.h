#pragma once
#include <Windows.h>

PIMAGE_DOS_HEADER PeDosHeader(void* pe);

// Returns the correct header type as a void *. 
// user casts it to either a PIMAGE_NT_HEADERS32/64 struct;
void* PeNtHeader(void* pe, int mode);

PIMAGE_NT_HEADERS32 PeNtHeader32(void* pe);

PIMAGE_NT_HEADERS64 PeNtHeader64(void* pe);

// Returns the correct sizeof(NT_HEADER) given the mode
size_t PeNtHeaderSize(int mode);

PIMAGE_FILE_HEADER PeFileHeader(void* pe, int mode);

// Returns the correct header type as a void *. 
// user casts it to either a PIMAGE_OPTIONAL_HEADER32/64;
void* PeOptionalHeader(void* pe);

PIMAGE_OPTIONAL_HEADER32 PeOptionalHeader32(void* pe);

PIMAGE_OPTIONAL_HEADER64 PeOptionalHeader64(void* pe);

size_t PeOptionalHeaderSize(int mode);

// Returns information about the section.
// index 0 => section header for '.text' section.
PIMAGE_SECTION_HEADER PeSectionHeaderByIndex(void* pe, size_t index = 0);

// Returns information about the section.
// name - e.g. ".rdata", ".text"
PIMAGE_SECTION_HEADER PeSectionHeaderByName(void* pe, char* name);

// Returns a struct with a VirtualAddress and Size.
// arg: 
//		entry : corresponds with windows defines in winnt.h
// e.g.
// IMAGE_DIRECTORY_ENTRY_EXPORT          0   // Export Directory
// IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
// IMAGE_DIRECTORY_ENTRY_RESOURCE        2   // Resource Directory
// IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   // Exception Directory
// IMAGE_DIRECTORY_ENTRY_SECURITY        4   // Security Directory
// IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
// IMAGE_DIRECTORY_ENTRY_DEBUG           6   // Debug Directory
PIMAGE_DATA_DIRECTORY PeDataDirectory(void* pe, int entry);

// Returns true if the ptr appears to point to a valid pe file
bool ValidatePE(void* pe);

// Returns 32 or 64 for the PE architecture, -1 on error.
int PeArchitectureMode(void * pe);