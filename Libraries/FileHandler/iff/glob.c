/*
    FileHandler - General-purpose file handling library for Niotso
    glob.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include "iff.h"

int iff_parse_glob(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    char ** string = (char**) &ChunkInfo->FormattedData;
    unsigned Size = ChunkInfo->Size - 76;
    unsigned length;

    if(Size == 0) return 0;

    /* Try reading as a C string */
    for(length=0; length != Size && Buffer[length] && Buffer[length] != 0xA3; length++);

    if(length != Size){
        if(length > 0){
            *string = malloc(length+1);
            if(*string == NULL) return 0;
            strcpy(*string, (char*) Buffer);
        }
        return 1;
    }

    /* Try again as a Pascal string */
    length = Buffer[0];
    if(length >= Size) return 0;

    if(length > 0){
        *string = malloc(length+1);
        if(*string == NULL) return 0;
        memcpy(*string, Buffer+1, length);
        (*string)[length] = 0x00;
    }
    return 1;
}