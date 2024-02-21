#pragma once

bool StripDebugInformation(void* pe);

// Scans from the beginning of the NT header back to the dosHeader looking for 'Rich'
// Once it finds rich, it knows where the end of the rich header is.
// Zeros out PE + 0x80 - end of rich header.
bool StripRichHeader(void* pe);