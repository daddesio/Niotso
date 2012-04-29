/*
    trcn.c - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>

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
    return 0; /*
    IFF_TRCN * TRCNData;
    unsigned Size = ChunkInfo->Size - 76;
    unsigned i;
    
    if(Size < 16)
        return 0;
    ChunkInfo->FormattedData = malloc(sizeof(IFF_TRCN));
    if(ChunkInfo->FormattedData == NULL)
        return 0;
    
    TRCNData = (IFF_TRCN*) ChunkInfo->FormattedData;
    TRCNData->Reserved = read_uint32le(Buffer+0);
    TRCNData->Version = read_uint32le(Buffer+4);
    memcpy(TRCNData->MagicNumber, Buffer+8, 4);
    TRCNData->MagicNumber[4] = 0x00;
    TRCNData->EntryCount = read_uint32le(Buffer+12);
    
    if(TRCNData->Reserved != 0 || TRCNData->Version > 2 || strcmp(TRCNData->MagicNumber, "NCRT")){
        free(TRCNData);
        return 0;
    }
    ChunkInfo->FormattedData = malloc(TRCNData->EntryCount * sizeof(IFFRangePair));
    if(ChunkInfo->FormattedData == NULL){
        free(TRCNData);
        return 0;
    }
    
    Buffer += 16;
    for(i=0; i<TRCNData->EntryCount; i++){
    } */
}

void iff_free_trcn(void * FormattedData){
    /*IFF_TRCN *TRCNData = (IFF_TRCN*) FormattedData;*/
}