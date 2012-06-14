/*
    FileHandler - General-purpose file handling library for Niotso
    palt.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include "iffparser.h"

int iff_parse_palt(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFPalette *Palette;

    if(ChunkInfo->Size < 16)
        return 0;
    ChunkInfo->FormattedData = malloc(sizeof(IFFPalette));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    Palette = ChunkInfo->FormattedData;
    Palette->Version = read_uint32le(Buffer);
    Palette->ColorCount = read_uint32le(Buffer+4);
    Palette->Reserved1 = read_uint32le(Buffer+8);
    Palette->Reserved2 = read_uint32le(Buffer+12);
    if(Palette->Version != 1 || Palette->ColorCount == 0 || Palette->ColorCount > 256 ||
        Palette->Reserved1 != 0 || Palette->Reserved2 != 0 || Palette->ColorCount*3 > ChunkInfo->Size-16)
        return 0;

    memcpy(Palette->Data, Buffer+16, Palette->ColorCount*3);
    return 1;
}