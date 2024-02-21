#include "PE.h"
#include <Windows.h>

bool ValidatePE(void* pe)
{
	auto dosHeader = PeDosHeader(pe);

	// 'MZ'
	if (!dosHeader || dosHeader->e_magic != 0x5A4D)
		return false;

	// The NT header doesn't differ at the start
	auto ntHeader = PeNtHeader32(pe);

	// 'PE'
	if (!ntHeader || *(DWORD*)ntHeader != 0x4550)
		return false;

	return true;
}

PIMAGE_DOS_HEADER PeDosHeader(void* pe) 
{
	if (!pe)
		return nullptr;

	return (PIMAGE_DOS_HEADER)pe;
}

PIMAGE_NT_HEADERS32 PeNtHeader32(void* pe)
{
	if (!pe)
		return nullptr;

	auto dosHeader = PeDosHeader(pe);

	if (!dosHeader)
		return nullptr;

	auto ntHeader = (PIMAGE_NT_HEADERS32)((size_t)dosHeader + (size_t)dosHeader->e_lfanew);

	return ntHeader;
}

PIMAGE_NT_HEADERS64 PeNtHeader64(void* pe)
{
	return (PIMAGE_NT_HEADERS64)(PeNtHeader32(pe));
}

size_t PeNtHeaderSize(int mode)
{
	if (mode == 32)
		return sizeof(IMAGE_NT_HEADERS32);
	else if (mode == 64)
		return sizeof(IMAGE_NT_HEADERS64);

	return 0;
}

// Returns the correct header type as a void *. 
// user casts it to either a PIMAGE_NT_HEADERS32 / 64 struct;
void* PeNtHeader(void* pe, int mode)
{
	if (mode == 32)
		return (void*)PeNtHeader32(pe);
	else if (mode == 64)
		return (void*)PeNtHeader64(pe);

	return nullptr;
}

PIMAGE_FILE_HEADER PeFileHeader(void* pe, int mode)
{
	void* ntHeader = nullptr;
	PIMAGE_FILE_HEADER fileHeader = nullptr;

	ntHeader = PeNtHeader(pe, mode);

	if (!ntHeader)
		return nullptr;

	if (mode == 32)
		return &((PIMAGE_NT_HEADERS32)ntHeader)->FileHeader;
	else if (mode == 64)
		return &((PIMAGE_NT_HEADERS64)ntHeader)->FileHeader;

	return nullptr;
}

void* PeOptionalHeader(void* pe)
{
	return (void*)PeOptionalHeader32(pe);
}

PIMAGE_OPTIONAL_HEADER32 PeOptionalHeader32(void* pe)
{
	auto ntHeader = PeNtHeader32(pe);

	if (!ntHeader)
		return nullptr;

	return &ntHeader->OptionalHeader;
}

PIMAGE_OPTIONAL_HEADER64 PeOptionalHeader64(void* pe)
{
	return (PIMAGE_OPTIONAL_HEADER64)PeOptionalHeader32(pe);
}

size_t PeOptionalHeaderSize(int mode)
{
	if (mode == 32)
		return sizeof(IMAGE_OPTIONAL_HEADER32);
	else if (mode == 64)
		return sizeof(IMAGE_OPTIONAL_HEADER64);

	return 0;
}

PIMAGE_SECTION_HEADER PeSectionHeaderByIndex(void* pe, size_t index)
{
	auto dosHeader = PeDosHeader(pe);
	auto mode = PeArchitectureMode(pe);
	auto fileHeader = PeFileHeader(pe, mode);

	if (!dosHeader || !fileHeader)
		return nullptr;

	if (index >= fileHeader->NumberOfSections)
		return nullptr;

	// Use the correct header size for the architecture mode of the pe
	auto headersSize = mode == 32 ? sizeof(IMAGE_NT_HEADERS32) : sizeof(IMAGE_NT_HEADERS64);

	size_t rvaSectionHeader = (size_t)((size_t)dosHeader->e_lfanew + headersSize);

	auto sectionHeader = (PIMAGE_SECTION_HEADER)((size_t)dosHeader + (size_t)rvaSectionHeader);

	sectionHeader = &sectionHeader[index];

	return sectionHeader;
}

PIMAGE_SECTION_HEADER PeSectionHeaderByName(void* pe, char* name)
{
	auto dosHeader = PeDosHeader(pe);
	auto mode = PeArchitectureMode(pe);
	auto fileHeader = PeFileHeader(pe, mode);
	bool bFound = false;
	PIMAGE_SECTION_HEADER sectionHeader = nullptr;

	if (!dosHeader || !fileHeader)
		return nullptr;

	// Use the correct header size for the architecture mode of the pe
	auto headersSize = mode == 32 ? sizeof(IMAGE_NT_HEADERS32) : sizeof(IMAGE_NT_HEADERS64);

	size_t rvaSectionHeader = (size_t)((size_t)dosHeader->e_lfanew + headersSize);

	auto first = (PIMAGE_SECTION_HEADER)((size_t)dosHeader + (size_t)rvaSectionHeader);

	for (int i = 0; i < fileHeader->NumberOfSections; i++)
	{
		sectionHeader = &first[i];

		if (!_stricmp(name, (const char*)sectionHeader->Name))
		{
			bFound = true;
			break;
		}		
	}

	if (bFound)
		return sectionHeader;

	return nullptr;
}

PIMAGE_DATA_DIRECTORY PeDataDirectory(void* pe, int entry)
{
	// Last entry in winnt.h
	if (entry > IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR)
		return nullptr;

	auto mode = PeArchitectureMode(pe);
	auto optionalHeader = PeOptionalHeader(pe);

	if (mode == -1)
		return nullptr;

	auto directory = mode == 32 ? &((PIMAGE_OPTIONAL_HEADER32)optionalHeader)->DataDirectory[entry] : &((PIMAGE_OPTIONAL_HEADER64)optionalHeader)->DataDirectory[entry];

	return directory;
}

#define AMD64 0x8664
#define INTEL_386 0x014C

int PeArchitectureMode(void * pe)
{
	// The machine word is at the beginning of the NT header.
	// before sizes differ between architectures.
	auto fileHeader = PeFileHeader(pe, 32);

	if (!fileHeader)
		return -1;

	if (fileHeader->Machine == AMD64)
		return 64;
	
	if (fileHeader->Machine == INTEL_386)
		return 32;

	return -1;
}
