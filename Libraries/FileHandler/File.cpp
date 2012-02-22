/*
    libvitaboy - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdint.h>
#include <windows.h>
#include "FileHandler.hpp"

namespace File {

int Error = 0;
unsigned FileSize = 0;

uint8_t * ReadFile(const char * Filename){
    HANDLE hFile = CreateFile(Filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if(hFile == INVALID_HANDLE_VALUE){
        File::Error = (GetLastError() == ERROR_FILE_NOT_FOUND) ? FERR_NOT_FOUND : FERR_OPEN;
        return NULL;
    }
    
    FileSize = GetFileSize(hFile, NULL);
    if(FileSize == 0){
        CloseHandle(hFile);
        File::Error = FERR_BLANK;
        return NULL;
    }
    
    uint8_t * InData = (uint8_t*) malloc(FileSize);
    if(InData == NULL){
        CloseHandle(hFile);
        File::Error = FERR_MEMORY;
        return NULL;
    }

    DWORD bytestransferred;
    BOOL result = ::ReadFile(hFile, InData, FileSize, &bytestransferred, NULL);
    CloseHandle(hFile);
    
    if(!result || bytestransferred != FileSize){
        free(InData);
        File::Error = FERR_READ;
        return NULL;
    }
    return InData;
}

}