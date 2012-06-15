/*
    FileHandler - General-purpose file handling library for Niotso
    objf.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

int iff_parse_objf(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFFunctionTable *Table;
    unsigned i;

    if(ChunkInfo->Size < 16)
        return 0;
    ChunkInfo->FormattedData = calloc(1, sizeof(IFFFunctionTable));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    Table = ChunkInfo->FormattedData;
    Table->Reserved = read_uint32le(Buffer);
    Table->Version = read_uint32le(Buffer+4);
    memcpy(Table->MagicNumber, Buffer+8, 4);
    Table->MagicNumber[4] = 0x00;
    Table->FunctionCount = read_uint32le(Buffer+12);
    if(Table->Reserved != 0 || Table->Version != 0)
        return 0;
    if(Table->FunctionCount == 0)
        return 1;
    if(Table->FunctionCount > (ChunkInfo->Size - 16)/4)
        return 0;

    Table->Functions = malloc(Table->FunctionCount * sizeof(IFFFunction));
    if(Table->Functions == NULL)
        return 0;

    Buffer += 16;
    for(i=0; i<Table->FunctionCount; i++, Buffer += 4){
        Table->Functions[i].ConditionID = read_uint16le(Buffer);
        Table->Functions[i].ActionID = read_uint16le(Buffer+2);
    }
 	return 1;
}

void iff_free_objf(void * FormattedData){
    IFFFunctionTable *Table = FormattedData;
    free(Table->Functions);
}