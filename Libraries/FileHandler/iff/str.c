/*
    FileHandler - General-purpose file handling library for Niotso
    str.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include "iffparser.h"

int iff_parse_str(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFString * StringData;
    bytestream b;

    if(ChunkInfo->Size < 2)
        return 0;
    set_bytestream(&b, Buffer, ChunkInfo->Size);
    ChunkInfo->FormattedData = calloc(1, sizeof(IFFString));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    StringData = ChunkInfo->FormattedData;
    StringData->Format = read_uint16(&b);
    if(b.Size < 2) /* TSO allows this; as seen in the animations chunk in personglobals.iff */
        return 1;

    if(StringData->Format < -4 || StringData->Format > 0){
        /* Seen often in The Sims 1's Behavior.iff */
        StringData->Format = 0;
        seekto(&b, 0);
        b.Endian++;
    }

    if(StringData->Format != -3){
        unsigned LanguageSetCount = 1;
        unsigned ls;

        if(StringData->Format == -4){
            LanguageSetCount = read_uint8(&b);
            if(LanguageSetCount > 20) return 0;
        }

        for(ls=0; ls<LanguageSetCount; ls++){
            IFFLanguageSet * LanguageSet = &StringData->LanguageSets[ls];
            unsigned p;

            if(b.Size < 2) return 0;
            LanguageSet->PairCount = read_uint16(&b);
            if(LanguageSet->PairCount == 0)
                continue;

            LanguageSet->Pairs = calloc(LanguageSet->PairCount, sizeof(IFFStringPair));
            if(LanguageSet->Pairs == NULL)
                return 0;

            for(p=0; p<LanguageSet->PairCount; p++){
                IFFStringPair * Pair = &LanguageSet->Pairs[p];
                if(StringData->Format == 0){
                    if(!read_pascal_string(&b, &Pair->Key))
                        return 0;
                }else if(StringData->Format >=-2){
                    if(!read_c_string(&b, &Pair->Key))
                        return 0;
                    if(StringData->Format == -2 && !read_c_string(&b, &Pair->Value))
                        return 0;
                }else{
                    if(!b.Size || read_uint8(&b) != ls)
                        return 0;
                    if(!read_pascal2_string(&b, &Pair->Key) || !read_pascal2_string(&b, &Pair->Value))
                        return 0;
                }
            }
        }
    }else{
        /* FD FF requires a lot of extra work -- and isn't even found in TSO */
        IFFLanguageSet * LanguageSet = StringData->LanguageSets;
        unsigned TotalPairCount;
        unsigned Index[20] = {0};
        unsigned i;

        TotalPairCount = read_uint16(&b);
        if(TotalPairCount == 0)
            return 1;

        /*
        ** Scan through the chunk to count up the number of pairs in each LanguageSet,
        ** and then allocate exactly that much and fill in the data on the second pass
        */

        /* 1st pass */
        for(i=0; i<TotalPairCount; i++){
            unsigned lang, s;
            if(b.Size == 0) return 0;
            lang = read_uint8(&b) - 1;
            if(lang >= 20) return 0;
            LanguageSet[lang].PairCount++;

            for(s=0; s<2; s++){
                unsigned length;
                for(length=0; length != b.Size && b.Buffer[length]; length++);
                if(length == b.Size) return 0;
                skipbytes(&b, length+1);
            }
        }

        for(i=0; i<20; i++){
            LanguageSet[i].Pairs = calloc(LanguageSet[i].PairCount, sizeof(IFFStringPair));
            if(LanguageSet[i].Pairs == NULL) return 0;
        }

        /* 2nd pass */
        set_bytestream(&b, Buffer+4, ChunkInfo->Size-4);
        for(i=0; i<TotalPairCount; i++){
            IFFStringPair * Pair;
            unsigned lang;
            lang = read_uint8(&b) - 1;
            Pair = &LanguageSet[lang].Pairs[Index[lang]++];

            if(!read_c_string(&b, &Pair->Key) || !read_c_string(&b, &Pair->Value))
                return 0;
        }
    }

    return 1;
}

void iff_free_str(void * FormattedData){
    /*
    ** Requirements:
    ** - PairCount and Pairs must be initialized for all 20 LanguageSets
    ** - If the Pairs pointer is nonzero, there must be PairCount initialized pairs in Pairs
    */

    IFFString * StringData = FormattedData;
    unsigned ls;

    for(ls=0; ls<20; ls++){
        IFFLanguageSet * LanguageSet = &StringData->LanguageSets[ls];
        if(LanguageSet->Pairs){
            unsigned p;
            for(p=0; p<LanguageSet->PairCount; p++){
                free(LanguageSet->Pairs[p].Key);
                free(LanguageSet->Pairs[p].Value);
            }
            free(LanguageSet->Pairs);
        }
    }
}