/*
    FileHandler - General-purpose file handling library for Niotso
    cst.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
    Author(s): Fatbag <X-Fi6@phppoll.org>

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

#include <stdlib.h>
#include <string.h>
#include "cst.h"

static unsigned cst_count_strings(const char * Buffer, size_t FileSize){
    unsigned count = 0;
    int instring = 0;
    while(FileSize--){
        if(*Buffer == '^' && (instring = !instring) == 0)
            count++;
        Buffer++;
    }
    return count;
}

int cst_read(CSTFile * CSTFileInfo, char * Buffer, size_t FileSize){
    CSTFileInfo->CSTData = Buffer;
    CSTFileInfo->StringCount = cst_count_strings(Buffer, FileSize);
    if(CSTFileInfo->StringCount != 0){
        unsigned i;
        CSTFileInfo->Strings = malloc(CSTFileInfo->StringCount * sizeof(char *));
        if(CSTFileInfo->Strings == NULL)
            return 0;
        for(i=0; i<CSTFileInfo->StringCount; i++){
            CSTFileInfo->Strings[i] = Buffer = strchr(Buffer, '^') + 1;
            *(Buffer = strchr(Buffer, '^')) = 0x00;
            Buffer++;
        }
    }
    return 1;
}