/*
    chunks.c - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>

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
#include <stdint.h>
#include "iff.h"

int iff_parse_chunk(IFFChunk * ChunkInfo, const uint8_t * Buffer, IFFFile *SourceFile){
    if( !strcmp(ChunkInfo->Type, "STR#")  ||
        !strcmp(ChunkInfo->Type, "CTSS") ||
        !strcmp(ChunkInfo->Type, "FAMs") ||
        !strcmp(ChunkInfo->Type, "TTAs") ) 
        return iff_parse_str(ChunkInfo, Buffer);
    else if (!strcmp(ChunkInfo->Type, "BHAV"))
        return iff_parse_bhav(ChunkInfo, Buffer);
    else if (!strcmp(ChunkInfo->Type, "SPR#") || !strcmp(ChunkInfo->Type, "SPR2"))
        return iff_parse_sprite(ChunkInfo, Buffer, SourceFile);
    else if (!strcmp(ChunkInfo->Type, "PALT"))
        return iff_parse_pmap(ChunkInfo, Buffer);
        return 0;
}

int iff_parse_rsmp(IFFChunk * ChunkInfo, const uint8_t * Buffer, unsigned IFFSize){
    return 1;
}