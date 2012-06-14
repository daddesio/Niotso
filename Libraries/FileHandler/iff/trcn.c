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

#include "iffparser.h"

int iff_parse_trcn(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFRangeSet *RangeSet;
    bytestream b;
    unsigned i;

    if(ChunkInfo->Size < 16)
        return 0;
    set_bytestream(&b, Buffer, ChunkInfo->Size);
    ChunkInfo->FormattedData = calloc(1, sizeof(IFFRangeSet));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    RangeSet = ChunkInfo->FormattedData;
    RangeSet->Reserved = read_uint32(&b);
    RangeSet->Version = read_uint32(&b);
    memcpy(RangeSet->MagicNumber, b.Buffer, 4);
    skipbytes(&b, 4);
    RangeSet->RangeCount = read_uint32(&b);
    if(RangeSet->Reserved != 0 || RangeSet->Version > 2)
        return 0;
    if(RangeSet->RangeCount == 0)
        return 1;

    RangeSet->Ranges = calloc(RangeSet->RangeCount, sizeof(IFFRangeEntry));
    if(RangeSet->Ranges == NULL)
        return 0;

    for(i=0; i<RangeSet->RangeCount; i++){
        unsigned s;
        IFFRangeEntry * Range = &RangeSet->Ranges[i];
        if(b.Size < 10)
            return 0;

        Range->IsUsed = read_uint32(&b);
        Range->DefaultValue = read_uint32(&b);

        for(s=0; s<2; s++){
            char ** string = (s==0) ? &Range->Name : &Range->Comment;
            if(b.Size == 0) return 0;

            if(RangeSet->Version < 2){
                /* C string, possible padding */
                if(!read_c_string(&b, string))
                    return 0;

                /* Skip past the 0xA3 character;
                ** see global.iff chunk 546 for why you can't do modulo-2 to detect this */
                if(b.Size && *b.Buffer == 0xA3)
                    skipbytes(&b, 1);
            }else{
                /* Extended Pascal string, no padding */
                if(!read_pascal2_string(&b, string))
                    return 0;
            }
        }

        if(RangeSet->Version != 0){
            if(b.Size < 5) return 0;
            Range->Enforced = read_uint8(&b);
            Range->RangeMin = read_uint16(&b);
            Range->RangeMax = read_uint16(&b);
        }
    }

    return 1;
}

void iff_free_trcn(void * FormattedData){
    IFFRangeSet *RangeSet = FormattedData;
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