/*
    FileHandler - General-purpose file handling library for Niotso
    glob.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

int iff_parse_glob(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    bytestream b;
    set_bytestream(&b, Buffer, ChunkInfo->Size);

    /* Try reading as a C string, and if that fails, again as a Pascal string */
    return (read_c_string(&b, (char**) &ChunkInfo->FormattedData) ||
        read_pascal_string(&b, (char**) &ChunkInfo->FormattedData));
}