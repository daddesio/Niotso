/*
    FileHandler - General-purpose file handling library for Niotso
    far.h - Copyright (c) 2011 Niotso Project <http://niotso.org/>
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

/****
** Constants
*/

/* libfarOptions array members */
#define FAR_CONFIG_DEFAULT_TO_1A             0
#define FAR_CONFIG_DBPF_COMPRESSED           1
#define FAR_CONFIG_MAX_FILE_NAME_LENGTH      2
#define FAR_CONFIG_REFPACK_HNSV              3

/* Archive types */
#define FAR_TYPE_INVALID    0
#define FAR_TYPE_FAR        1
#define FAR_TYPE_DBPF       2
#define FAR_TYPE_PERSIST    3

/* Numerical constants */
#define FAR_ARCHIVE_MINIMUM_SIZE     14
#define MINSIZE_FAR                  20
#define MINSIZE_DBPF                 64
#define MINSIZE_ENTRY_FAR_1A         16
#define MINSIZE_ENTRY_FAR_1B         14
#define MINSIZE_ENTRY_FAR_3          24
#define SIZEOF_ENTRY_DBPF            20
#define MAX_ENTRIES_FAR_1A           268435455
#define MAX_ENTRIES_FAR_1B           306783377
#define MAX_ENTRIES_FAR_3            178956970
#define MAX_ENTRIES_DBPF             214748364
#define MINSIZE_PERSIST              18
#define MAXSIZE_REFPACK_UNCOMPRESSED 16777215
#define MAXSIZE_REFPACK_COMPRESSED   16777215

/* Header bytes */
static const uint8_t Header_FAR[]  = {'F','A','R','!','b','y','A','Z'};
static const uint8_t Header_DBPF[] = {'D','B','P','F'};

/****
** Data structures
*/

typedef struct PersistFile_s
{
    uint8_t   BodyType;
    uint32_t  DecompressedSize;
    uint32_t  CompressedSize;
    uint32_t  StreamBodySize;
    uint8_t   Compressor;
    uint8_t * Parameters;
    uint8_t * CompressedData;
    uint8_t * DecompressedData;
} PersistFile;

typedef struct RefPackParameters_s
{
    uint8_t hnsv;
    uint32_t DecompressedSize;
} RefPackParameters;

typedef struct FAREntry_s
{
    uint32_t  DecompressedSize;
    uint32_t  CompressedSize;
    uint8_t   DataType;
    uint32_t  DataOffset;
    uint16_t  HasFilename;
    uint32_t  FilenameLength;
    uint32_t  TypeID;
    uint32_t  GroupID;
    uint32_t  FileID;
    char *    Filename;
    PersistFile PersistData;
    uint8_t * CompressedData;
    uint8_t * DecompressedData;
} FAREntry;

typedef struct FAREntryNode_s
{
    FAREntry Entry;
    struct FAREntryNode_s * PrevEntry;
    struct FAREntryNode_s * NextEntry;
} FAREntryNode;

typedef struct FARFile_s
{
    int32_t Type;

    /* Header */
    uint32_t MajorVersion;
    uint32_t MinorVersion;
    uint32_t Revision;
    uint32_t IndexOffset;
    /* DBPF */
    uint32_t DateCreated;
    uint32_t DateModified;
    uint32_t IndexMajorVersion;
    uint32_t IndexSize;
    uint32_t TrashCount;
    uint32_t TrashOffset;
    uint32_t TrashSize;
    uint32_t IndexMinorVersion;

    /* Regular index */
    uint32_t Files;
    FAREntryNode * FirstEntry;
    FAREntryNode * LastEntry;
    struct { /* DIR index for DBPF */
        FAREntryNode * FirstEntry;
        FAREntryNode * LastEntry;
    } Dir;
    struct { /* Trash index for DBPF */
        FAREntryNode * FirstEntry;
        FAREntryNode * LastEntry;
    } Trash;
} FARFile;

/****
** Exported functions
*/

#ifdef __cplusplus
extern "C" {
#endif

void libfar_set_option(int Option, int Value);
int libfar_get_option(int Option);

int far_identify(const uint8_t * Buffer, unsigned FileSize);

FARFile * far_create_archive(int Type);
PersistFile * far_create_persist();

FAREntryNode * far_add_entry(FARFile * FARFileInfo, int Position);

int far_read_header(FARFile * FARFileInfo, const uint8_t * Buffer, unsigned FileSize);
int far_read_entry(const FARFile * FARFileInfo, FAREntry * FAREntryInfo,
    const uint8_t * Buffer, unsigned MaxEntrySize, unsigned ArchiveSize);
int far_read_persist_header(PersistFile * PersistData, const uint8_t * Buffer, unsigned FileSize);
int far_read_entry_data(const FARFile * FARFileInfo, FAREntry * FAREntryInfo, uint8_t * Buffer);
int far_read_persist_data(PersistFile * PersistData, uint8_t * CompressedData);

int far_enumerate_entries(FARFile * FARFileInfo, const uint8_t * Index, unsigned IndexSize, unsigned ArchiveSize);
int far_enumerate_entry_data(const FARFile * FARFileInfo, uint8_t * Buffer);

FAREntryNode * far_search_id();
FAREntryNode * far_search_name();
FAREntryNode * far_search_multi();

void far_delete_entry(FARFile * FARFileInfo, int Position);
void far_delete_archive(FARFile * FARFileInfo);
void far_delete_persist(FARFile * FARFileInfo);
void libfar_free(void * ptr);

int RefPackDecompress(const uint8_t * CompressedData, size_t CompressedSize,
    uint8_t * DecompressedData, size_t DecompressedSize, unsigned HNSV);

#ifdef __cplusplus
}
#endif