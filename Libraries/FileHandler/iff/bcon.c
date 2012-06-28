/*
    FileHandler - General-purpose file handling library for Niotso
    bcon.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

int iff_parse_bcon(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFF_BCON *BCONData;
    unsigned i;

    if(ChunkInfo->Size < 2)
        return 0;
    ChunkInfo->FormattedData = calloc(1, sizeof(IFF_BCON));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    BCONData = ChunkInfo->FormattedData;
    BCONData->ConstantCount = read_uint8le(Buffer);
    BCONData->Flags = read_uint8le(Buffer + 1);
    if(BCONData->ConstantCount == 0)
        return 1;
    if(BCONData->ConstantCount * 2 /* bytes */ > ChunkInfo->Size - 2)
        return 0;

    BCONData->Constants = malloc(BCONData->ConstantCount * sizeof(uint16_t));
    if(BCONData->Constants == NULL)
        return 0;

    Buffer += 2;
    for(i=0; i<BCONData->ConstantCount; i++, Buffer += 2)
        BCONData->Constants[i] = read_uint16le(Buffer);
 	return 1;
}

void iff_free_bcon(void * FormattedData){
    IFF_BCON *BCONData = FormattedData;
    free(BCONData->Constants);
}