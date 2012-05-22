/*
    FileHandler - General-purpose file handling library for Niotso
    cats.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

int iff_parse_cats(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFStringPair * StringData;
    unsigned Size = ChunkInfo->Size - 76;
    int s;

    ChunkInfo->FormattedData = calloc(1, sizeof(IFFStringPair));
    if(ChunkInfo->FormattedData == NULL)
        return 0;
    StringData = (IFFStringPair*) ChunkInfo->FormattedData;

    for(s=0; s<2; s++){
        unsigned length;
        for(length=0; length != Size && Buffer[length]; length++);
        if(length == Size) return 0;

        if(length != 0){
            char ** string = (s==0) ? &StringData->Key : &StringData->Value;
            *string = malloc(length+1);
            if(*string == NULL) return 0;
            strcpy(*string, (char*) Buffer);

            Buffer += length;
            Size   -= length;
        }
        Buffer++; Size--;
    }

    return 1;
}

void iff_free_cats(void * FormattedData){
    IFFStringPair * StringData = (IFFStringPair*) FormattedData;
    free(StringData->Key);
    free(StringData->Value);
}