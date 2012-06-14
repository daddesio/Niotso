/*
    FileHandler - General-purpose file handling library for Niotso
    iffparser.h - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

typedef struct {
    const uint8_t * Buffer;
    size_t Size;
    const uint8_t * StartPos;
    size_t TotalSize;
    uint8_t Endian; /* 0 = little, 1 = big */
} bytestream;

static __inline void set_bytestream(bytestream * b, const uint8_t * Buffer, size_t Size){
    b->Buffer    = Buffer;
    b->Size      = Size;
    b->StartPos  = Buffer;
    b->TotalSize = Size;
    b->Endian    = 0;
}

static __inline uint32_t read_uint32(bytestream * b){
    unsigned value = (b->Endian == 0) ? read_uint32le(b->Buffer) : read_uint32be(b->Buffer);
    b->Buffer += 4;
    b->Size -= 4;
    return value;
}

static __inline uint16_t read_uint16(bytestream * b){
    unsigned value = (b->Endian == 0) ? read_uint16le(b->Buffer) : read_uint16be(b->Buffer);
    b->Buffer += 2;
    b->Size -= 2;
    return value;
}

static __inline uint8_t read_uint8(bytestream * b){
    uint8_t value = b->Buffer[0];
    b->Buffer += 1;
    b->Size -= 1;
    return value;
}

static __inline float read_float(bytestream *b){
    union { float f; uint32_t v; } value;
    value.v = read_uint32(b);
    return value.f;
}

static __inline int skipbytes(bytestream * b, size_t bytes){
    if(b->Size < bytes) return 0;
    b->Buffer += bytes; b->Size -= bytes;
    return 1;
}

static __inline int seekto(bytestream * b, size_t Position){
    if(Position > b->TotalSize) return 0;
    b->Buffer = b->StartPos + Position;
    b->Size = b->TotalSize - Position;
    return 1;
}

static __inline size_t read_c_string(bytestream * b, char ** dest){
    size_t length;
    for(length=0; length != b->Size && b->Buffer[length]; length++);
    if(length == b->Size) return 0;

    if(length != 0){
        *dest = malloc(length+1);
        if(*dest == NULL) return 0;
        strcpy(*dest, (char*) b->Buffer);
    }

    b->Buffer += length + 1;
    b->Size   -= length + 1;
    return length + 1;
}

static __inline size_t read_pascal_string(bytestream * b, char ** dest){
    size_t length;
    if(!b->Size) return 0;
    length = b->Buffer[0];
    if(length >= b->Size) return 0;

    if(length > 0){
        *dest = malloc(length+1);
        if(*dest == NULL) return 0;
        memcpy(*dest, b->Buffer+1, length);
        (*dest)[length] = 0x00;
    }

    b->Buffer += 1 + length;
    b->Size   -= 1 + length;
    return 1 + length;
}

static __inline size_t read_pascal2_string(bytestream * b, char ** dest){
    size_t length;
    int countbytes = 1;
    if(!b->Size) return 0;
    length = b->Buffer[0];

    if(length > 127){
        /* 2-byte length */
        if(b->Size == 1) return 0;
        length = (length&127) | (b->Buffer[1]<<7);
        countbytes++;
    }

    if(countbytes+length > b->Size) return 0;

    if(length != 0){
        *dest = malloc(length+1);
        if(*dest == NULL) return 0;
        memcpy(*dest, b->Buffer+countbytes, length);
        (*dest)[length] = 0x00;
    }

    b->Buffer += countbytes + length;
    b->Size   -= countbytes + length;
    return countbytes + length;
}

static __inline size_t skip_padding(bytestream * b){
    size_t padding = 0;
    while(b->Size && b->Buffer[0] == 0xA3){
        padding++;
        b->Buffer++;
        b->Size--;
    }
    return padding;
}