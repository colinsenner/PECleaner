#include <Windows.h>
#include <stdio.h>
#include "Lib/PE.h"
#include "StripDebugInformation.h"
#include "Lib/Hex.h"

unsigned int FileLength(FILE* fs)
{
	fseek(fs, 0, SEEK_END);
	long fsize = ftell(fs);
	fseek(fs, 0, SEEK_SET);

	return fsize;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Usage:  PeCleaner.exe <exefile>\n");
		printf("\nPeCleaner strips out Rich header and debug information from PE files overwriting file.\n");

		return 0;
	}

	char* inFile = argv[1];
	char* outFile = inFile;

	size_t bytesRead = 0;
	DWORD bytesWritten = 0;
	DWORD fileLength = 0;
	unsigned char* buffer = nullptr;
	int mode = 0;

	HANDLE hFile = nullptr;
	FILE* fs;

	if (fopen_s(&fs, inFile, "rb"))
	{
		printf("Couldn't open file '%s'.\n", inFile);
		return 1;
	}

	fileLength = FileLength(fs);

	// Make a buffer the size of the file we're reading
	buffer = (unsigned char*)malloc(fileLength + 1);
	ZeroMemory(buffer, fileLength + 1);

	if (!buffer) 
	{
		printf("Couldn't allocate %d bytes for a buffer.\n", fileLength);
		goto cleanup;
	}

	// Read the file into a buffer
	bytesRead = fread_s(buffer, fileLength, 1, fileLength, fs);

	if (bytesRead != fileLength)
	{
		printf("Couldn't read entire file into buffer.  Bytes Read: %zd.\n", bytesRead);
		goto cleanup;
	}

	if (fclose(fs))
		printf("Problem closing the file %s\n", inFile);

	if (!ValidatePE(buffer))
	{
		printf("Invalid PE File format.\n");
		goto cleanup;
	}

	mode = PeArchitectureMode(buffer);

	if (mode == -1)
	{
		printf("Couldn't detect the architecture mode of the PE file.\n");
		goto cleanup;
	}

	printf("Architecture is: %d-bit\n", mode);

	// Strip the DebugDirectory Information
	if (!StripDebugInformation(buffer))
	{
		printf("Couldn't strip debug information.\n");
		goto cleanup;
	}

	printf("Debug Information Stripped...\n");

	// Strip the RICH header
	StripRichHeader(buffer);

	printf("Rich header stripped...\n");

	// Create the out file
	hFile = CreateFileA(outFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (!hFile) 
	{
		printf("Couldn't create the file '%s' for writing.\n", outFile);
		goto cleanup;
	}

	if (!WriteFile(hFile, buffer, fileLength, &bytesWritten, NULL))
	{
		printf("Couldn't write file.  Error: %08X.\n", GetLastError());
		goto cleanup;
	}


	// Close handles, cleanup and exit
cleanup:
	fclose(fs);

	if (buffer)
		free(buffer);

	if (hFile)
		CloseHandle(hFile);

    return 0;
}