/*
    FileHandler - General-purpose file handling library for Niotso
    trcn.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
    Author(s): Ahmed El-Mahdawy <aa.mahdawy.10@gmail.com>
               Fatbag <X-Fi6@phppoll.org>

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

int iff_parse_trcn(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFRangeSet *RangeSet;
    unsigned Size = ChunkInfo->Size - 76;
    unsigned i;

    if(Size < 16)
        return 0;
    ChunkInfo->FormattedData = malloc(sizeof(IFFRangeSet));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    RangeSet = (IFFRangeSet*) ChunkInfo->FormattedData;
    RangeSet->Ranges = NULL;
    RangeSet->Reserved = read_uint32le(Buffer);
    RangeSet->Version = read_uint32le(Buffer+4);
    memcpy(RangeSet->MagicNumber, Buffer+8, 4);
    RangeSet->MagicNumber[4] = 0x00;
    RangeSet->RangeCount = read_uint32le(Buffer+12);
    if(RangeSet->Version > 2)
        return 0;
    if(RangeSet->RangeCount == 0)
        return 1;

    RangeSet->Ranges = calloc(RangeSet->RangeCount, sizeof(IFFRangeEntry));
    if(RangeSet->Ranges == NULL)
        return 0;

    Buffer += 16; Size -= 16;
    for(i=0; i<RangeSet->RangeCount; i++){
        unsigned s;
        IFFRangeEntry * Range = &RangeSet->Ranges[i];
        if(Size < 10)
            return 0;

        Range->IsUnused = read_uint32le(Buffer);
        Range->DefaultValue = read_uint32le(Buffer+4);
        Buffer += 8; Size -= 8;

        for(s=0; s<2; s++){
            char ** string = (s==0) ? &Range->Name : &Range->Comment;
            unsigned length;
            if(Size == 0) return 0;

            if(RangeSet->Version < 2){
                /* C string */
                for(length=0; length != Size && Buffer[length]; length++);
                if(length == Size) return 0;

                if(length != 0){
                    *string = malloc(length+1);
                    if(*string == NULL) return 0;
                    strcpy(*string, (char*) Buffer);
                }

                Buffer += length+1;
                Size   -= length+1;

                /* Skip past the 0xA3 character;
                ** see global.iff chunk 546 for why you can't do modulo-2 to detect this */
                if(Size && *Buffer == 0xA3){
                    Buffer++; Size--;
                }
            }else{
                /* Pascal string */
                length = read_uint8le(Buffer);
                Buffer++; Size--;

                if(length != 0){
                    *string = malloc(length+1);
                    if(*string == NULL) return 0;
                    memcpy(*string, Buffer, length);
                    (*string)[length] = 0x00;
                }

                Buffer += length;
                Size   -= length;
            }
        }

        if(RangeSet->Version != 0){
            if(Size < 5) return 0;
            Range->Enforced = read_uint8le(Buffer);
            Range->RangeMin = read_uint16le(Buffer+1);
            Range->RangeMax = read_uint16le(Buffer+3);
            Buffer += 5; Size -= 5;
        }
    }

    return 1;
}

void iff_free_trcn(void * FormattedData){
    IFFRangeSet *RangeSet = (IFFRangeSet*) FormattedData;
    if(RangeSet->Ranges){
        unsigned i;
        for(i=0; i<RangeSet->RangeCount; i++){
            IFFRangeEntry *Entry = &RangeSet->Ranges[i];
            free(Entry->Name);
            free(Entry->Comment);
        }
    }
    free(RangeSet->Ranges);
}