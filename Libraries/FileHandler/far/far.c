/*
    FileHandler - General-purpose file handling library for Niotso
    far.c - Copyright (c) 2011-2012 Niotso Project <http://niotso.org/>
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

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "far.h"

#if defined(FAR_SUPPORT_PERSIST)
 #define FAR_MINSIZE_ANY MINSIZE_PERSIST
#elif defined(FAR_SUPPORT_FAR)
 #define FAR_MINSIZE_ANY MINSIZE_FAR
#else
 #define FAR_MINSIZE_ANY MINSIZE_DBPF
#endif

#ifndef read_int32
 #define read_int32(x) (int)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)) | ((x)[3]<<(8*3)))
 #define read_int24(x) (int)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)))
 #define read_int16(x) (int)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)))
 #define read_int8(x)  (int)(((x)[0]<<(8*0)))
 #define read_uint32(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)) | ((x)[3]<<(8*3)))
 #define read_uint24(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)))
 #define read_uint16(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)))
 #define read_uint8(x)  (unsigned)(((x)[0]<<(8*0)))
#endif

#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

/* These options can be changed during runtime */
static int libfarOptions[] = {
    FAR_DEFAULT_1A,
    FAR_DEFAULT_DBPF_COMPRESSED,
    FAR_DEFAULT_MAX_FILE_NAME_LENGTH,
    FAR_DEFAULT_REFPACK_HNSV
};

void libfar_set_option(int Option, int Value){
    libfarOptions[Option] = Value;
}
int libfar_get_option(int Option){
    return libfarOptions[Option];
}

int far_identify(const uint8_t * Buffer, unsigned FileSize)
{
    if(!FileSize) FileSize = ~0;
    else if(FileSize < FAR_MINSIZE_ANY)
        return FAR_TYPE_INVALID;

    #ifdef FAR_SUPPORT_FAR
    if(FileSize >= MINSIZE_FAR && !memcmp(Buffer, Header_FAR, 8))
        return FAR_TYPE_FAR;
    #endif

    #ifdef FAR_SUPPORT_DBPF
    if(FileSize >= MINSIZE_DBPF && !memcmp(Buffer, Header_DBPF, 4))
        return FAR_TYPE_DBPF;
    #endif

    #ifdef FAR_SUPPORT_PERSIST
    if(FileSize >= MINSIZE_PERSIST && Buffer[0] == 0x01)
        return FAR_TYPE_PERSIST;
    #endif

    return FAR_TYPE_INVALID;
}

FARFile * far_create_archive(int Type)
{
    FARFile *ptr = calloc(1, sizeof(FARFile));
    if(ptr == NULL) return NULL;
    ptr->Type = Type;
    return ptr;
}

PersistFile * far_create_persist()
{
    return calloc(1, sizeof(PersistFile));
}

FAREntryNode * far_add_entry(FARFile * FARFileInfo, int Position)
{
    FAREntryNode *ptr = calloc(1, sizeof(FAREntryNode)), *node;
    if(ptr == NULL) return NULL;
    if(FARFileInfo == NULL) return ptr;

    if(Position >= 0){
        node = FARFileInfo->FirstEntry;

        if(node == NULL){
            FARFileInfo->FirstEntry = ptr;
            FARFileInfo->LastEntry = ptr;
        }else{
            /* Find the node we will take the place of */
            while(Position-- && node->NextEntry != NULL)
                node = node->NextEntry;

            if(node->PrevEntry == NULL)
                FARFileInfo->FirstEntry = ptr;

            /* Shift this node and all nodes after it above us */
            ptr->PrevEntry = node->PrevEntry;
            ptr->NextEntry = node;
            node->PrevEntry = ptr;
        }
    }else{
        node = FARFileInfo->LastEntry;

        if(node == NULL){
            FARFileInfo->FirstEntry = ptr;
            FARFileInfo->LastEntry = ptr;
        }else{
            /* Find the node we will take the place of */
            while(++Position && node->PrevEntry != NULL)
                node = node->PrevEntry;

            if(node->NextEntry == NULL)
                FARFileInfo->LastEntry = ptr;

            /* Shift this node and all nodes before it below us */
            ptr->PrevEntry = node;
            ptr->NextEntry = node->NextEntry;
            node->NextEntry = ptr;
        }
    }

    FARFileInfo->Files++;
    return ptr;
}

int far_read_header(FARFile * FARFileInfo, const uint8_t * Buffer, unsigned FileSize)
{
    if(!FileSize) FileSize = ~0;
    else if(FileSize < FAR_MINSIZE_ANY)
        return 0;

    #ifdef FAR_SUPPORT_FAR
    if(FARFileInfo->Type == FAR_TYPE_FAR){
        FARFileInfo->MajorVersion = read_uint32(Buffer+8);
        FARFileInfo->IndexOffset = read_uint32(Buffer+12);

        if(FARFileInfo->MajorVersion != 1 && FARFileInfo->MajorVersion != 3)
            return 0;

        if(FARFileInfo->MajorVersion == 1)
            FARFileInfo->Revision = !libfarOptions[FAR_CONFIG_DEFAULT_TO_1A];

        if(FARFileInfo->IndexOffset > FileSize-4)
            return 0;

        FARFileInfo->Files = read_uint32(Buffer + FARFileInfo->IndexOffset);

        if(FARFileInfo->Files > (FileSize-FARFileInfo->IndexOffset) / (
            (FARFileInfo->MajorVersion == 1) ? (
                (FARFileInfo->Revision == 0) ? MINSIZE_ENTRY_FAR_1A : MINSIZE_ENTRY_FAR_1B
            ) : MINSIZE_ENTRY_FAR_3)
        ) return 0;

        return 1;
    }
    #endif

    #ifdef FAR_SUPPORT_DBPF
    if(FARFileInfo->Type == FAR_TYPE_DBPF){
        int i;

        FARFileInfo->MajorVersion = read_uint32(Buffer+4);
        FARFileInfo->MinorVersion = read_uint32(Buffer+8);
        FARFileInfo->DateCreated = read_uint32(Buffer+24);
        FARFileInfo->DateModified = read_uint32(Buffer+28);
        FARFileInfo->IndexMajorVersion = read_uint32(Buffer+32);
        FARFileInfo->Files = read_uint32(Buffer+36);
        FARFileInfo->IndexOffset = read_uint32(Buffer+40);
        FARFileInfo->IndexSize = read_uint32(Buffer+44);
        FARFileInfo->TrashCount = read_uint32(Buffer+48);
        FARFileInfo->TrashOffset = read_uint32(Buffer+52);
        FARFileInfo->TrashSize = read_uint32(Buffer+56);
        FARFileInfo->IndexMinorVersion = read_uint32(Buffer+60);

        if(FARFileInfo->MajorVersion != 1) return 0;
        if(FARFileInfo->MinorVersion != 0) return 0;
        if(FARFileInfo->IndexMajorVersion != 7) return 0;
        if(FARFileInfo->IndexMinorVersion != 0) return 0;

        for(i=12; i<24; i++)
            if(Buffer[i] != 0x00) return 0;

        if(FARFileInfo->Files){
            if(FARFileInfo->Files > MAX_ENTRIES_DBPF) return 0;
            if(FARFileInfo->IndexSize != FARFileInfo->Files*SIZEOF_ENTRY_DBPF) return 0;
            if(FARFileInfo->IndexOffset > FileSize) return 0;
            if(FARFileInfo->IndexSize > FileSize - FARFileInfo->IndexOffset) return 0;
        }
        if(FARFileInfo->TrashCount){
            if(FARFileInfo->TrashCount > MAX_ENTRIES_DBPF) return 0;
            if(FARFileInfo->TrashSize != FARFileInfo->TrashCount*SIZEOF_ENTRY_DBPF) return 0;
            if(FARFileInfo->TrashOffset > FileSize) return 0;
            if(FARFileInfo->TrashSize > FileSize - FARFileInfo->TrashOffset) return 0;
        }

        return 1;
    }
    #endif

    return 0;
}

int far_read_entry(const FARFile * FARFileInfo, FAREntry * FAREntryInfo,
    const uint8_t * Buffer, unsigned MaxEntrySize, unsigned ArchiveSize)
{
    int MajorVersion = FARFileInfo->MajorVersion, Revision = FARFileInfo->Revision;
    if(MaxEntrySize == 0) MaxEntrySize = ~0;
    if(ArchiveSize == 0) ArchiveSize = ~0;

    #ifdef FAR_SUPPORT_FAR
    if(FARFileInfo->Type == FAR_TYPE_FAR){
        unsigned MinEntrySize =
            (MajorVersion == 1) ? (
                (Revision == 0) ? MINSIZE_ENTRY_FAR_1A : MINSIZE_ENTRY_FAR_1B
            ) : MINSIZE_ENTRY_FAR_3;

        if(MaxEntrySize < MinEntrySize)
            return 0;

        if(MajorVersion == 1){
            FAREntryInfo->DecompressedSize = read_uint32(Buffer+0);
            FAREntryInfo->CompressedSize = read_uint32(Buffer+4);
            FAREntryInfo->DataOffset = read_uint32(Buffer+8);
            if(Revision == 0)
                FAREntryInfo->FilenameLength = read_uint32(Buffer+12);
            else
                FAREntryInfo->FilenameLength = read_uint16(Buffer+12);
        }else if(MajorVersion == 3){
            FAREntryInfo->DecompressedSize = read_uint32(Buffer+0);
            FAREntryInfo->CompressedSize = read_uint24(Buffer+4);
            FAREntryInfo->DataType = read_uint8(Buffer+7);
            FAREntryInfo->DataOffset = read_uint32(Buffer+8);
            FAREntryInfo->HasFilename = read_uint16(Buffer+12);
            FAREntryInfo->FilenameLength = read_uint16(Buffer+14);
            FAREntryInfo->TypeID = read_uint32(Buffer+16);
            FAREntryInfo->FileID = read_uint32(Buffer+20);
        }

        if(FAREntryInfo->FilenameLength > MaxEntrySize - MinEntrySize) return 0;
        if(FAREntryInfo->FilenameLength > (unsigned)libfarOptions[FAR_CONFIG_MAX_FILE_NAME_LENGTH]) return 0;

        if(FAREntryInfo->CompressedSize > FAREntryInfo->DecompressedSize) return 0;
        if(FAREntryInfo->DecompressedSize != 0){
            if(FAREntryInfo->DataOffset > ArchiveSize) return 0;
            if(FAREntryInfo->CompressedSize > ArchiveSize - FAREntryInfo->DataOffset) return 0;
        }else if(MajorVersion == 3 && FAREntryInfo->DataType != 0) return 0;

        FAREntryInfo->Filename = malloc(FAREntryInfo->FilenameLength + 1);
        if(FAREntryInfo->Filename == NULL) return 0;
        if(FAREntryInfo->FilenameLength != 0)
            memcpy(FAREntryInfo->Filename, Buffer+MinEntrySize, FAREntryInfo->FilenameLength);
        FAREntryInfo->Filename[FAREntryInfo->FilenameLength] = '\0';
        return 1;
    }
    #endif

    #ifdef FAR_SUPPORT_DBPF
    if(FARFileInfo->Type == FAR_TYPE_DBPF){
        if(MaxEntrySize < SIZEOF_ENTRY_DBPF) return 0;

        FAREntryInfo->TypeID = read_uint32(Buffer+0);
        FAREntryInfo->GroupID = read_uint32(Buffer+4);
        FAREntryInfo->FileID = read_uint32(Buffer+8);
        FAREntryInfo->DataOffset = read_uint32(Buffer+12);
        FAREntryInfo->CompressedSize = read_uint32(Buffer+16);
        FAREntryInfo->DecompressedSize = FAREntryInfo->CompressedSize;

        if(FAREntryInfo->CompressedSize != 0){
            if(FAREntryInfo->CompressedSize > ArchiveSize - FAREntryInfo->DataOffset) return 0;
            if(FAREntryInfo->DataOffset > ArchiveSize) return 0;
            if(FAREntryInfo->CompressedSize > ArchiveSize - FAREntryInfo->DataOffset) return 0;
        }

        return 1;
    }
    #endif

    return 0;
}

int far_read_persist_header(PersistFile * PersistData, const uint8_t * Buffer, unsigned FileSize)
{
    RefPackParameters * Parameters;
    PersistData->BodyType = read_uint8(Buffer);
    PersistData->DecompressedSize = read_uint32(Buffer+1);
    PersistData->CompressedSize = read_uint32(Buffer+5);
    PersistData->StreamBodySize = read_uint32(Buffer+9);
    PersistData->Compressor = read_uint8(Buffer+13);

    if(PersistData->BodyType != 1)
        return 0;
    if(PersistData->CompressedSize < FileSize-9)
        return 0;

    if(PersistData->CompressedSize != PersistData->StreamBodySize)
        return 0;

    PersistData->Parameters = malloc(sizeof(RefPackParameters));
    if(PersistData->Parameters == NULL)
        return 0;

    Parameters = (RefPackParameters *) PersistData->Parameters;
    Parameters->hnsv = read_uint8(Buffer+14);
    Parameters->DecompressedSize = (Buffer[15]<<16) | (Buffer[16]<<8) | Buffer[17]; /* Big endian */

    if((PersistData->DecompressedSize&0x00FFFFFF) != Parameters->DecompressedSize){
        free(PersistData->Parameters); return 0;
    }
    return 1;
}

int far_read_entry_data(const FARFile * FARFileInfo, FAREntry * FAREntryInfo, uint8_t * Buffer)
{
    int Compressed = (FARFileInfo->Type == FAR_TYPE_FAR) ? (
        (FARFileInfo->MajorVersion == 1) ? (
        FAREntryInfo->DecompressedSize != FAREntryInfo->CompressedSize
    ) : FAREntryInfo->DataType == 0x80) : libfarOptions[FAR_CONFIG_DBPF_COMPRESSED];

    FAREntryInfo->CompressedData = Buffer+FAREntryInfo->DataOffset;

    if(!Compressed)
        FAREntryInfo->DecompressedData = FAREntryInfo->CompressedData;
    else{
        PersistFile * PersistInfo = &(FAREntryInfo->PersistData);
        if(far_identify(FAREntryInfo->CompressedData, FAREntryInfo->CompressedSize) != FAR_TYPE_PERSIST)
            return 0;

        if(!far_read_persist_header(PersistInfo, FAREntryInfo->CompressedData, FAREntryInfo->CompressedSize))
            return 0;

        /* Verify that the Persist data agrees with the entry data */
        if(PersistInfo->DecompressedSize != FAREntryInfo->DecompressedSize){
            free(PersistInfo->Parameters); return 0;
        }if(PersistInfo->CompressedSize != FAREntryInfo->CompressedSize - 9){
            free(PersistInfo->Parameters); return 0;
        }if(PersistInfo->Compressor != 0x10){
            free(PersistInfo->Parameters); return 0;
        }

        PersistInfo->DecompressedData = FAREntryInfo->DecompressedData;

        if(!far_read_persist_data(PersistInfo, FAREntryInfo->CompressedData+18)){
            free(PersistInfo->Parameters); return 0;
        }
        FAREntryInfo->DecompressedData = PersistInfo->DecompressedData;
    }

    return 1;
}

int far_read_persist_data(PersistFile * PersistData, uint8_t * CompressedData)
{
    if(CompressedData != NULL) PersistData->CompressedData = CompressedData;

    PersistData->DecompressedData = malloc(PersistData->DecompressedSize);
    if(PersistData->DecompressedData == NULL)
        return 0;

    if(!RefPackDecompress(PersistData->CompressedData, PersistData->CompressedSize-9,
        PersistData->DecompressedData, PersistData->DecompressedSize,
        libfarOptions[FAR_CONFIG_REFPACK_HNSV])){
        free(PersistData->DecompressedData);
        return 0;
    }
    return 1;
}

int far_enumerate_entries(FARFile * FARFileInfo, const uint8_t * Index, unsigned IndexSize, unsigned ArchiveSize)
{
    unsigned Files = FARFileInfo->Files;
    int ArchiveType = FARFileInfo->Type;
    unsigned MinEntrySize = (ArchiveType == FAR_TYPE_FAR) ? (
        (FARFileInfo->MajorVersion == 1) ? (
        (FARFileInfo->Revision == 0) ? MINSIZE_ENTRY_FAR_1A : MINSIZE_ENTRY_FAR_1B
    ) : MINSIZE_ENTRY_FAR_3) : SIZEOF_ENTRY_DBPF;
    unsigned EntriesAdded = 0;
    FARFileInfo->Files = 0;

    if(FARFileInfo->Type == FAR_TYPE_FAR)
        Index += 4;

    while(EntriesAdded < Files){
        FAREntryNode * node = far_add_entry(FARFileInfo, -1);
        if(node == NULL)
            return 0;
        if(!far_read_entry(FARFileInfo, &(node->Entry), Index, IndexSize, ArchiveSize)){
            free(node);
            return 0;
        }

        EntriesAdded++;
        Index += MinEntrySize;
        IndexSize -= MinEntrySize;
        if(ArchiveType != FAR_TYPE_DBPF){
            Index += node->Entry.FilenameLength;
            IndexSize -= node->Entry.FilenameLength;
        }
    }

    return (EntriesAdded == FARFileInfo->Files);
}

int far_enumerate_entry_data(const FARFile * FARFileInfo, uint8_t * Buffer)
{
    FAREntryNode * node = FARFileInfo->FirstEntry;
    while(node){
        if(!far_read_entry_data(FARFileInfo, &(node->Entry), Buffer))
            return 0;
        node = node->NextEntry;
    }
    return 1;
}

void libfar_free(void * ptr)
{
    free(ptr);
}