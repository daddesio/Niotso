/*
    str.c - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>

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

int iff_parse_str(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFF_STR * StringData;
    unsigned Size = ChunkInfo->Size - 76;

    if(Size < 2)
        return 0;
    ChunkInfo->FormattedData = calloc(1, sizeof(IFF_STR));
    if(ChunkInfo->FormattedData == NULL)
        return 0;
    
    StringData = (IFF_STR*) ChunkInfo->FormattedData;
    StringData->Format = read_int16le(Buffer);
    if((Size-=2) < 2) /* TSO allows this; as seen in the animations chunk in personglobals.iff */
        return 1;
    Buffer += 2;
    
    switch(StringData->Format){
    
    case 0: { /* 00 00 */
        unsigned i;
        IFFLanguageSet * LanguageSet = &StringData->LanguageSets[0];

        LanguageSet->PairCount = read_uint16le(Buffer);
        Buffer += 2; Size -= 2;
        if(LanguageSet->PairCount == 0)
            return 1;

        LanguageSet->Pairs = calloc(LanguageSet->PairCount, sizeof(IFFStringPair));
        if(LanguageSet->Pairs == NULL)
            return 0;

        for(i=0; i<LanguageSet->PairCount; i++){
            unsigned length;

            if(Size == 0) return 0;
            length = read_uint8le(Buffer);
            Buffer++; Size--;
            if(length != 0){
                if(length > Size) return 0;
                LanguageSet->Pairs[i].Key = malloc(length+1);
                if(LanguageSet->Pairs[i].Key == NULL) return 0;
                memcpy(LanguageSet->Pairs[i].Key, Buffer, length);
                LanguageSet->Pairs[i].Key[length] = 0x00;

                Buffer += length;
                Size   -= length;
            }
        }
    } return 1;

    case -1: { /* FF FF */
        unsigned i;
        IFFLanguageSet * LanguageSet = &StringData->LanguageSets[0];
        
        LanguageSet->PairCount = read_uint16le(Buffer);
        Buffer += 2; Size -= 2;
        if(LanguageSet->PairCount == 0)
            return 1;
        
        LanguageSet->Pairs = calloc(LanguageSet->PairCount, sizeof(IFFStringPair));
        if(LanguageSet->Pairs == NULL)
            return 0;
        
        for(i=0; i<LanguageSet->PairCount; i++){
            unsigned length;

            if(Size == 0) return 0;
            for(length=0; Size-length && Buffer[length]; length++);
            if(Buffer[length] != 0x00) return 0;
            
            if(length != 0){
                LanguageSet->Pairs[i].Key = malloc(length+1);
                if(LanguageSet->Pairs[i].Key == NULL) return 0;
                strcpy(LanguageSet->Pairs[i].Key, (char*) Buffer);

                Buffer += length;
                Size   -= length;
            }
            Buffer++; Size--;
        }
    } return 1;

    case -2: { /* FE FF */
        unsigned i;
        IFFLanguageSet * LanguageSet = &StringData->LanguageSets[0];
        
        LanguageSet->PairCount = read_uint16le(Buffer);
        Buffer += 2; Size -= 2;
        if(LanguageSet->PairCount == 0)
            return 1;
        
        LanguageSet->Pairs = calloc(LanguageSet->PairCount, sizeof(IFFStringPair));
        if(LanguageSet->Pairs == NULL)
            return 0;
        
        for(i=0; i<LanguageSet->PairCount; i++){
            int s;

            for(s=0; s<2; s++){
                unsigned length;
                if(Size == 0) return 0;
                for(length=0; Size-length && Buffer[length]; length++);
                if(Buffer[length] != 0x00) return 0;
                
                if(length != 0){
                    char ** string = (s==0) ? &LanguageSet->Pairs[i].Key : &LanguageSet->Pairs[i].Value;
                    *string = malloc(length+1);
                    if(*string == NULL) return 0;
                    strcpy(*string, (char*) Buffer);

                    Buffer += length;
                    Size   -= length;
                }
                Buffer++; Size--;
            }
        }
    } return 1;

    case -3: { /* FD FF */
        IFFLanguageSet * LanguageSet = StringData->LanguageSets;
        unsigned i;
        unsigned TotalPairCount = read_uint16le(Buffer);
        unsigned Index[20] = {0};
        const uint8_t * Start = (Buffer += 2);
        Size -= 2;

        if(TotalPairCount == 0)
            return 1;
        
        /*
        ** Scan through the chunk to count up the number of strings in each LanguageSet,
        ** and then allocate exactly that much and fill in the data on the second pass
        */

        /* 1st pass */
        for(i=0; i<TotalPairCount; i++){
            unsigned lang, s;
            if(Size == 0) return 0;
            lang = read_uint8le(Buffer) - 1;
            if(lang >= 20) return 0;
            LanguageSet[lang].PairCount++;
            Buffer++; Size--;
            
            for(s=0; s<2; s++){
                /* Includes the string length check too */
                unsigned length;
                if(Size == 0) return 0;
                for(length=0; Size-length && Buffer[length]; length++);
                if(Buffer[length] != 0x00) return 0;
                Buffer += length+1;
                Size   -= length+1;
            }
        }
        
        for(i=0; i<20; i++){
            LanguageSet[i].Pairs = calloc(LanguageSet[i].PairCount, sizeof(IFFStringPair));
            if(LanguageSet[i].Pairs == NULL) return 0;
        }
        
        /* 2nd pass */
        Buffer = Start;
        for(i=0; i<TotalPairCount; i++){
            unsigned lang = read_uint8le(Buffer) - 1, s;
            IFFStringPair * Pair = &LanguageSet[lang].Pairs[Index[lang]++];
            Buffer++;
            
            for(s=0; s<2; s++){
                unsigned length = strlen((char*) Buffer);
                if(length != 0){
                    char ** string = (s==0) ? &Pair->Key : &Pair->Value;
                    *string = malloc(length+1);
                    if(*string == NULL) return 0;
                    strcpy(*string, (char*) Buffer);

                    Buffer += length;
                }
                Buffer++;
            }
        }
    } return 1;

    case -4: { /* FC FF */
        unsigned lang;
        unsigned LanguageSetCount = read_uint8le(Buffer);
        Buffer++; Size--;
        if(LanguageSetCount > 20) return 0;
        
        for(lang=0; lang<LanguageSetCount; lang++){
            unsigned i;
            IFFLanguageSet * LanguageSet = &StringData->LanguageSets[lang];
            
            if(Size < 2) return 0;
            LanguageSet->PairCount = read_uint16le(Buffer);
            Buffer += 2; Size -= 2;
            if(LanguageSet->PairCount == 0)
                continue;

            LanguageSet->Pairs = calloc(LanguageSet->PairCount, sizeof(IFFStringPair));
            if(LanguageSet->Pairs == NULL)
                return 0;
            
            for(i=0; i<LanguageSet->PairCount; i++){
                unsigned s;
                if(Size == 0) return 0;
                Buffer++; Size--; /* Skip over the "Language set index" */
                
                for(s=0; s<2; s++){
                    unsigned length;
                    if(Size == 0) return 0;
                    length = read_uint8le(Buffer);
                    Buffer++; Size--;
                    if(length > 127){
                        if(Size == 0) return 0;
                        length = (length&127) | (read_uint8le(Buffer)<<7);
                        Buffer++; Size--;
                    }
                    if(length != 0){
                        IFFStringPair * Pair = &LanguageSet->Pairs[i];
                        char ** string = (s==0) ? &Pair->Key : &Pair->Value;
                        if(length > Size) return 0;
                        *string = malloc(length+1);
                        if(*string == NULL) return 0;
                        memcpy(*string, Buffer, length);
                        (*string)[length] = 0x00;

                        Buffer += length;
                        Size   -= length;
                    }
                }
            }
        }
    } return 1;
    
    }

    return 0;
}

void iff_free_str(void * FormattedData){
    /*
    ** Requirements:
    ** - PairCount and Pairs must be initialized for all 20 LanguageSets
    ** - If the Pairs pointer is nonzero, there must be PairCount initialized pairs in Pairs
    */

    IFF_STR * StringData = (IFF_STR*) FormattedData;
    unsigned ls;
    
    for(ls=0; ls<20; ls++){
        IFFLanguageSet * LanguageSet = &StringData->LanguageSets[ls];
        unsigned p;
        if(LanguageSet->Pairs){
            for(p=0; p<LanguageSet->PairCount; p++){
                free(LanguageSet->Pairs[p].Key);
                free(LanguageSet->Pairs[p].Value);
            }
            free(LanguageSet->Pairs);
        }
    }
}