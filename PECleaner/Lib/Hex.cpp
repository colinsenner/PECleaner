#include "Hex.h"
#include <stdio.h>

#define COLOR_BLUE(string) "\x1b[36m" string "\x1b[0m"

bool isPrintableAscii(char c) 
{
	return c >= 33 && c <= 126;
}

void PrintSpaces(size_t num) {
	for (size_t i = 0; i < num; i++)
	{
		printf(" ");
	}
}

void Hex(void* bytes, size_t len, size_t columns, size_t separator, bool ascii)
{
	size_t offset = 0;
	size_t offset_ascii = 0;
	size_t address = (size_t)bytes;
	size_t lineLength = 0;

	size_t rows = (len / columns) + 1;
	size_t numSeparators = (columns / separator) - 1;
	size_t columnLength = 6 + (columns * 3) + (numSeparators * 2);

	printf("          ");

	// Print columns
	for (unsigned int col = 0; col < columns; col++)
	{
		if (col != 0 && col % separator == 0)
			printf("| ");

		printf(COLOR_BLUE("%02X "), col);
	}

	printf("\n");

	for (unsigned int row = 0; row < rows; row++)
	{
		if (offset >= len)
			break;

		offset_ascii = offset;

		printf(COLOR_BLUE("%08X  "), (unsigned int)address + (unsigned int)offset);
		lineLength = 6;

		for (unsigned int col = 0; col < columns; col++)
		{
			if (offset >= len)
				break;

			if (col != 0 && col % separator == 0)
			{
				printf("| ");
				lineLength += 2;
			}

			printf("%02X ", ((unsigned char*)bytes)[offset++]);
			lineLength += 3;
		}		

		// Print ASCII characters
		if (ascii)
		{
			// Print spaces out to the correct alignment
			size_t spacesToAdd = columnLength - lineLength + 4;
			PrintSpaces(spacesToAdd);

			for (unsigned int col = 0; col < columns; col++)
			{
				if (offset_ascii >= len)
					break;

				char c = ((unsigned char*)bytes)[offset_ascii++];

				c = isPrintableAscii(c) ? c : '.';
				printf("%c", c);
			}
		}

		printf("\n");
	}
}