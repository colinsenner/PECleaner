#include "StripDebugInformation.h"
#include "Lib/PE.h"
#include "Lib/Hex.h"

#include <Windows.h>
#include <stdio.h>

// Returns the address and size of a module's section
// index is the section you want to get
// e.g. 0 => .text, 1 => .data
bool StripDebugInformation(void* pe)
{
  // Virtual Address shown includes the .rdata section (0x2150)
  auto debugDirectory = PeDataDirectory(pe, IMAGE_DIRECTORY_ENTRY_DEBUG);

  size_t debugEntries = debugDirectory->Size / sizeof(IMAGE_DEBUG_DIRECTORY);

  // We need to get the VA of rdata and subtract them to find the debug directory entry
  auto rdata = PeSectionHeaderByName(pe, (char*)".rdata");

  if (!rdata)
    return false;

  // Take the VA of the debugDirectory (0x2150) and the VA of the .rdata section (0x2000) and subtract them
  // To get the offset into the .rdata section of the DEBUG_DIRECTORY (0x150)
  size_t debugDirectoryOffset = (size_t)debugDirectory->VirtualAddress - (size_t)rdata->VirtualAddress;

  // Add the pe base + rdata's start on disk + the offset of the debug directory.
  auto debugEntry = (PIMAGE_DEBUG_DIRECTORY)((size_t)pe + (size_t)rdata->PointerToRawData + (size_t)debugDirectoryOffset);

  // Strip all the information from them
  auto first = debugEntry;
  for (unsigned int i = 0; i < debugEntries; i++)
  {
    auto entry = &first[i];

    if (entry->PointerToRawData) {
      void* entryData = (void*)((size_t)pe + (size_t)entry->PointerToRawData);

      // Zero out the address that PointerToRawData points to
      ZeroMemory(entryData, entry->SizeOfData);
    }

    // Remove the entry itself from the debug directory
    ZeroMemory(entry, sizeof(IMAGE_DEBUG_DIRECTORY));
  }

  ZeroMemory(debugDirectory, sizeof(IMAGE_DATA_DIRECTORY));

  return true;
}

bool StripRichHeader(void* pe)
{
  auto mode = PeArchitectureMode(pe);
  auto ptrSize = mode == 32 ? 4 : 8;
  auto dosHeader = PeDosHeader(pe);
  auto ntHeader = PeNtHeader(pe, mode);
  bool bFound = false;
  void* pCurHeader = ntHeader;

  while (pCurHeader > dosHeader)
  {
    // 'Rich'
    if (*(DWORD*)pCurHeader == 0x68636952)
    {
      bFound = true;
      break;
    }

    pCurHeader = (void*)((size_t)pCurHeader - ptrSize);
  }

  if (bFound)
  {
    // pCurHeader points to the rich header
    auto richHeaderStart = (void*)((size_t)pe + 0x80);
    auto richHeaderEnd = (void*)((size_t)pCurHeader + 8);

    ZeroMemory(richHeaderStart, (size_t)richHeaderEnd - (size_t)richHeaderStart);
  }

  return bFound;
}
