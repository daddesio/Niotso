/*
    FileHandler - General-purpose file handling library for Niotso
    iff.h - Copyright (c) 2012 Niotso Project <http://niotso.org/>
    Author(s): Fatbag <X-Fi6@phppoll.org>
               Ahmed El-Mahdawy <aa.mahdawy.10@gmail.com>

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

#ifndef read_uint32be
 #define read_int32be(x)  (signed)(((x)[0]<<(8*3)) | ((x)[1]<<(8*2)) | ((x)[2]<<(8*1)) | ((x)[3]<<(8*0)))
 #define read_int24be(x)  (signed)(((x)[0]<<(8*2)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*0)))
 #define read_int16be(x)  (signed)(((x)[0]<<(8*1)) | ((x)[1]<<(8*0)))
 #define read_int8be(x)   (signed)(((x)[0]<<(8*0)))
 #define read_int32le(x)  (signed)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)) | ((x)[3]<<(8*3)))
 #define read_int24le(x)  (signed)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)))
 #define read_int16le(x)  (signed)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)))
 #define read_int8le(x)   (signed)(((x)[0]<<(8*0)))
 #define read_uint32be(x) (unsigned)(((x)[0]<<(8*3)) | ((x)[1]<<(8*2)) | ((x)[2]<<(8*1)) | ((x)[3]<<(8*0)))
 #define read_uint24be(x) (unsigned)(((x)[0]<<(8*2)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*0)))
 #define read_uint16be(x) (unsigned)(((x)[0]<<(8*1)) | ((x)[1]<<(8*0)))
 #define read_uint8be(x)  (unsigned)(((x)[0]<<(8*0)))
 #define read_uint32le(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)) | ((x)[3]<<(8*3)))
 #define read_uint24le(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)))
 #define read_uint16le(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)))
 #define read_uint8le(x)  (unsigned)(((x)[0]<<(8*0)))
#endif

/*
** IFF file headers
*/

typedef struct IFFChunk_s
{
    char      Type[5];
    uint32_t  Size;
    uint16_t  ChunkID;
    uint16_t  Flags;
    char      Label[65];
    uint8_t * Data;
    void *    FormattedData;
} IFFChunk;

typedef struct IFFFile_s
{
    uint8_t Header[64];

    uint32_t ChunkCount;
    size_t SizeAllocated;
    IFFChunk * Chunks;
    IFFChunk * ResourceMap;
} IFFFile;

static const uint8_t Header_IFF[] = "IFF FILE 2.5:TYPE FOLLOWED BY SIZE\0 JAMIE DOORNBOS & MAXIS 1";

/*
** IFF chunk structs
*/

/* BCON chunk */

typedef struct IFF_BCON_s
{
    uint8_t ConstantCount;
    uint8_t Flags;
    uint16_t * Constants;
} IFF_BCON;

/* STR# chunk */

enum IFFLanguage {
    IFFLANG_DEFAULT             = 0,
    IFFLANG_EN_US               = 1,
    IFFLANG_EN_INTERNATIONAL    = 2,
    IFFLANG_FRENCH              = 3,
    IFFLANG_GERMAN              = 4,
    IFFLANG_ITALIAN             = 5,
    IFFLANG_SPANISH             = 6,
    IFFLANG_DUTCH               = 7,
    IFFLANG_DANISH              = 8,
    IFFLANG_SWEDISH             = 9,
    IFFLANG_NORWEGIAN           = 10,
    IFFLANG_FINNISH             = 11,
    IFFLANG_HEBREW              = 12,
    IFFLANG_RUSSIAN             = 13,
    IFFLANG_PORTUGUESE          = 14,
    IFFLANG_JAPANESE            = 15,
    IFFLANG_POLISH              = 16,
    IFFLANG_CHINESE_SIMPLIFIED  = 17,
    IFFLANG_CHINESE_TRADITIONAL = 18,
    IFFLANG_THAI                = 19,
    IFFLANG_KOREAN              = 20
};

typedef struct IFFStringPair_s
{
    uint8_t LanguageSet;
    char * Key;
    char * Value;
} IFFStringPair;

typedef struct IFFLanguageSet_s
{
    uint16_t PairCount;
    IFFStringPair * Pairs;
} IFFLanguageSet;

typedef struct IFF_STR_s
{
    int16_t Format;
    IFFLanguageSet LanguageSets[20];
} IFFString;

/* TRCN chunk */

typedef struct IFFRangeEntry_s
{
    uint32_t IsUnused;
    uint32_t DefaultValue;
    char * Name;
    char * Comment;
    uint8_t Enforced;
    uint16_t RangeMin;
    uint16_t RangeMax;
} IFFRangeEntry;

typedef struct IFFRangeSet_s
{
    uint32_t Reserved;
    uint32_t Version;
    char MagicNumber[5];
    uint32_t RangeCount;
    IFFRangeEntry * Ranges;
} IFFRangeSet;

#ifdef __cplusplus
extern "C" {
#endif

/*
** IFF file functions
*/

IFFFile * iff_create();
int iff_read_header(IFFFile * IFFFileInfo, const uint8_t * Buffer, unsigned FileSize);

IFFChunk * iff_add_chunk(IFFFile * IFFFileInfo);
int iff_read_chunk(IFFChunk * ChunkInfo, const uint8_t * Buffer, unsigned MaxChunkSize);
int iff_parse_chunk(IFFChunk * ChunkInfo, const uint8_t * Buffer);
int iff_enumerate_chunks(IFFFile * IFFFileInfo, const uint8_t * Buffer, unsigned BufferSize);

void iff_free_chunk(IFFChunk * ChunkInfo);
void iff_delete_chunk(IFFFile * IFFFileInfo, int Position);
void iff_delete(IFFFile * IFFFileInfo);

#ifdef __cplusplus
}
#endif