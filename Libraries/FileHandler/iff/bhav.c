/*
    FileHandler - General-purpose file handling library for Niotso
    bhav.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#define HEADER_SIZE (12 + (Behavior->Version == 0x8003))

int iff_parse_bhav(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFBehavior *Behavior;
    unsigned i;

    if(ChunkInfo->Size < 12)
        return 0;
    ChunkInfo->FormattedData = calloc(1, sizeof(IFFBehavior));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    Behavior = ChunkInfo->FormattedData;
    Behavior->Version = read_uint16le(Buffer);
    if(Behavior->Version < 0x8000 || Behavior->Version > 0x8003 || (Behavior->Version == 0x8003 && ChunkInfo->Size < 13))
        return 0;

    switch(Behavior->Version){
    case 0x8000: {
        Behavior->InstructionCount = read_uint16le(Buffer+2);
    } break;

    case 0x8001: {
        Behavior->InstructionCount = read_uint16le(Buffer+2);
    } break;

    case 0x8002: {
        Behavior->InstructionCount = read_uint16le(Buffer+2);
        Behavior->Type = read_uint8le(Buffer+4);
        Behavior->ArgumentCount = read_uint8le(Buffer+5);
        Behavior->LocalCount = read_uint16le(Buffer+6);
        Behavior->Flags = read_uint16le(Buffer+8);
    } break;

    default: {
        Behavior->Type = read_uint8le(Buffer+2);
        Behavior->ArgumentCount = read_uint8le(Buffer+3);
        Behavior->LocalCount = read_uint8le(Buffer+4);
        Behavior->Flags = read_uint16le(Buffer+7);
        Behavior->InstructionCount = read_uint32le(Buffer+9);
    } break;
    }

    if(Behavior->InstructionCount == 0)
        return 1;
    if(Behavior->InstructionCount > 255 || Behavior->InstructionCount*12 > ChunkInfo->Size - HEADER_SIZE)
        return 0;

    Behavior->Instructions = calloc(Behavior->InstructionCount, sizeof(IFFInstruction));
    if(Behavior->Instructions == NULL)
        return 0;

    Buffer += HEADER_SIZE;

    for(i=0; i<Behavior->InstructionCount; i++){
        Behavior->Instructions[i].Opcode = read_uint16le(Buffer);
        Behavior->Instructions[i].TDest = read_uint8le(Buffer+2);
        Behavior->Instructions[i].FDest = read_uint8le(Buffer+3);
        memcpy(Behavior->Instructions[i].Operands, Buffer+4, 8);

        Buffer += 12;
    }

    return 1;
}

void iff_free_bhav(void * FormattedData){
    IFFBehavior *Behavior = FormattedData;
    free(Behavior->Instructions);
}