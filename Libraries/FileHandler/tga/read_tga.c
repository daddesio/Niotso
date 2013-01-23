/*
    FileHandler - General-purpose file handling library for Niotso
    read_tga.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "read_tga.h"

#ifndef read_uint32
 #define read_uint32(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)) | ((x)[3]<<(8*3)))
 #define read_uint16(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)))
#endif

int tga_read_header(tgaheader_t * TGAHeader, const uint8_t * Buffer, size_t FileSize){
    unsigned padding;
    if(FileSize < ?) return 0;
    TGAHeader->IDLength = read_uint8(Buffer);
    TGAHeader->ColorMapType = read_uint8(Buffer+1);
    TGAHeader->ImageType = read_uint8(Buffer+2);
    
    /* Color map */
    TGAHeader->FirstEntry = read_uint16(Buffer+3);
    TGAHeader->ColorMapLength = read_uint16(Buffer+5);
    TGAHeader->EntrySize = read_uint8(Buffer+7);
    
    /* Image */
    TGAHeader->XOrigin = read_uint16(Buffer+8);
    TGAHeader->YOrigin = read_uint16(Buffer+10);
    TGAHeader->Width = read_uint16(Buffer+12);
    TGAHeader->Height = read_uint16(Buffer+14);
    TGAHeader->ImageDepth = read_uint8(Buffer+16);
    TGAHeader->Descriptor = read_uint8(Buffer+17);

    return 1;
}
