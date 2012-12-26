/*
    hitutils - The Sims HIT (dis)assembler and linker
    hitdump.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef min
 #define min(x,y) ((x) < (y) ? (x) : (y))
#endif
#ifndef max
 #define max(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef read_int32
 #define read_uint32(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)) | ((x)[3]<<(8*3)))
 #define read_uint16(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)))
#endif

static void Shutdown_M(const char * Message, ...);

enum {
    hsm, hot, evt, hit, out, filecount
};

enum {
    VERSION_TS1 = 1, VERSION_TSO
};

#define OPERAND_BYTES(x) (x)
#define OPERAND(x, y) ((y)<<((x)*4+4))
#define UNIMPLEMENTED ((uint32_t)~0)

enum operand_t {
    o_byte = 1,
    o_dword,
    o_address,
    o_variable,
    o_jump
};

typedef struct {
    const char * Name;
    uint32_t Operands;
} instruction_t;

typedef struct {
    const char * Name;
    uint32_t Value;
} global_t;

static const uint8_t HITHeader[] = {'H','I','T','!',0x01,0x00,0x00,0x00,0x08,0x00,0x00,0x00,'T','R','A','X'};

static const instruction_t Instructions[] = {
    {"note",                UNIMPLEMENTED},
    {"note_on",             OPERAND_BYTES(1) | OPERAND(0, o_variable)},
    {"note_off",            OPERAND_BYTES(1) | OPERAND(0, o_variable)},
    {"loadb",               OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_byte)},
    {"loadl",               OPERAND_BYTES(5) | OPERAND(0, o_variable) | OPERAND(1, o_dword)},
    {"set",                 OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"call",                OPERAND_BYTES(4) | OPERAND(0, o_address)},
    {"return",              OPERAND_BYTES(0)},
    {"wait",                OPERAND_BYTES(1) | OPERAND(0, o_variable)},
    {"callentrypoint",      UNIMPLEMENTED},
    {"wait_samp",           OPERAND_BYTES(0)},
    {"end",                 OPERAND_BYTES(0)},
    {"jump",                OPERAND_BYTES(1) | OPERAND(0, o_jump)},
    {"test",                OPERAND_BYTES(1) | OPERAND(0, o_variable)},
    {"nop",                 OPERAND_BYTES(0)},
    {"add",                 OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"sub",                 OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"div",                 OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"mul",                 OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"cmp",                 OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"less",                UNIMPLEMENTED},
    {"greater",             UNIMPLEMENTED},
    {"not",                 UNIMPLEMENTED},
    {"rand",                OPERAND_BYTES(3) | OPERAND(0, o_variable) | OPERAND(1, o_variable) | OPERAND(2, o_variable)},
    {"abs",                 UNIMPLEMENTED},
    {"limit",               UNIMPLEMENTED},
    {"error",               UNIMPLEMENTED},
    {"assert",              UNIMPLEMENTED},
    {"add_to_group",        UNIMPLEMENTED},
    {"remove_from_group",   UNIMPLEMENTED},
    {"get_var",             UNIMPLEMENTED},
    {"loop",                OPERAND_BYTES(0)},
    {"set_loop",            OPERAND_BYTES(0)},
    {"callback",            UNIMPLEMENTED},
    {"smart_add",           UNIMPLEMENTED},
    {"smart_remove",        UNIMPLEMENTED},
    {"smart_removeall",     UNIMPLEMENTED},
    {"smart_setcrit",       UNIMPLEMENTED},
    {"smart_choose",        OPERAND_BYTES(1) | OPERAND(0, o_variable)},
    {"and",                 UNIMPLEMENTED},
    {"nand",                UNIMPLEMENTED},
    {"or",                  UNIMPLEMENTED},
    {"nor",                 UNIMPLEMENTED},
    {"xor",                 UNIMPLEMENTED},
    {"max",                 OPERAND_BYTES(5) | OPERAND(0, o_variable) | OPERAND(1, o_dword)},
    {"min",                 OPERAND_BYTES(5) | OPERAND(0, o_variable) | OPERAND(1, o_dword)},
    {"inc",                 OPERAND_BYTES(1) | OPERAND(0, o_variable)},
    {"dec",                 OPERAND_BYTES(1) | OPERAND(0, o_variable)},
    {"printreg",            UNIMPLEMENTED},
    {"play_trk",            OPERAND_BYTES(1) | OPERAND(0, o_variable)},
    {"kill_trk",            OPERAND_BYTES(1) | OPERAND(0, o_variable)},
    {"push",                UNIMPLEMENTED},
    {"push_mask",           UNIMPLEMENTED},
    {"push_vars",           UNIMPLEMENTED},
    {"call_mask",           UNIMPLEMENTED},
    {"call_push",           UNIMPLEMENTED},
    {"pop",                 UNIMPLEMENTED},
    {"test1",               OPERAND_BYTES(0)},
    {"test2",               OPERAND_BYTES(0)},
    {"test3",               OPERAND_BYTES(0)},
    {"test4",               OPERAND_BYTES(0)},
    {"ifeq",                OPERAND_BYTES(4) | OPERAND(0, o_address)},
    {"ifne",                OPERAND_BYTES(4) | OPERAND(0, o_address)},
    {"ifgt",                OPERAND_BYTES(4) | OPERAND(0, o_address)},
    {"iflt",                OPERAND_BYTES(4) | OPERAND(0, o_address)},
    {"ifge",                OPERAND_BYTES(4) | OPERAND(0, o_address)},
    {"ifle",                OPERAND_BYTES(4) | OPERAND(0, o_address)},
    {"smart_setlist",       OPERAND_BYTES(1) | OPERAND(0, o_variable)},
    {"seqgroup_kill",       OPERAND_BYTES(1) | OPERAND(0, o_byte)},
    {"seqgroup_wait",       UNIMPLEMENTED},
    {"seqgroup_return",     OPERAND_BYTES(1) | OPERAND(0, o_byte)},
    {"getsrcdatafield",     OPERAND_BYTES(3) | OPERAND(0, o_variable) | OPERAND(1, o_variable) | OPERAND(2, o_variable)},
    {"seqgroup_trkid",      OPERAND_BYTES(2) | OPERAND(0, o_byte)     | OPERAND(1, o_byte)},
    {"setll",               OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"setlt",               OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"settl",               OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"waiteq",              OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"waitne",              OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"waitgt",              OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"waitlt",              OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"waitge",              OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"waitle",              OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"duck",                OPERAND_BYTES(0)},
    {"unduck",              OPERAND_BYTES(0)},
    {"testx",               UNIMPLEMENTED},
    {"setlg",               OPERAND_BYTES(5) | OPERAND(0, o_variable) | OPERAND(1, o_dword)},
    {"setgl",               OPERAND_BYTES(5) | OPERAND(0, o_variable) | OPERAND(1, o_dword)},
    {"throw",               UNIMPLEMENTED},
    {"setsrcdatafield",     OPERAND_BYTES(3) | OPERAND(0, o_variable) | OPERAND(1, o_variable) | OPERAND(2, o_variable)},
    {"stop_trk",            OPERAND_BYTES(1) | OPERAND(0, o_variable)},
    {"setchanreg",          UNIMPLEMENTED},
    {"play_note",           UNIMPLEMENTED},
    {"stop_note",           UNIMPLEMENTED},
    {"kill_note",           UNIMPLEMENTED},
    {"smart_index",         OPERAND_BYTES(2) | OPERAND(0, o_variable) | OPERAND(1, o_variable)},
    {"note_on_loop",        OPERAND_BYTES(1) | OPERAND(0, o_variable)}
};

static const char *Registers[] = {
    "arg0",
    "arg1",
    "arg2",
    "arg3",
    "arg4",
    "v1",
    "v2",
    "v3",
    "v4",
    "v5",
    "v6",
    "v7",
    "v8",
    "h1",
    "h2",
    "h3",
    "h4"
};

#define TSOGlobalCount 66
static const global_t TSOGlobals[] = {
    {"argstype", 0x10},
    {"trackdatasource", 0x11},
    {"patch", 0x12},
    {"priority", 0x13},
    {"vol", 0x14},
    {"extvol", 0x15},
    {"pan", 0x16},
    {"pitch", 0x17},
    {"paused", 0x18},
    {"fxtype", 0x19},
    {"fxlevel", 0x1a},
    {"duckpri", 0x1b},
    {"Is3d", 0x1c},
    {"IsHeadRelative", 0x1d},
    {"MinDistance", 0x1e},
    {"MaxDistance", 0x1f},
    {"X", 0x20},
    {"Y", 0x21},
    {"Z", 0x22},
    {"attack", 0x23},
    {"decay", 0x24},
    {"IsStreamed", 0x25},
    {"bufsizemult", 0x26},
    {"fade_dest", 0x27},
    {"fade_var", 0x28},
    {"fade_speed", 0x29},
    {"fade_on", 0x2a},
    {"Preload", 0x2b},
    {"isplaying", 0x2c},
    {"whattodowithupdate", 0x2d},
    {"tempo", 0x2e},
    {"target", 0x2f},
    {"ctrlgroup", 0x30},
    {"interrupt", 0x31},
    {"ispositioned", 0x32},
    {"AppObjectId", 0x34},
    {"callbackarg", 0x35},
    {"pitchrandmin", 0x36},
    {"pitchrandmax", 0x37},
    {"spl", 0x38},
    {"sem", 0x39},
    {"starttrackid", 0x3a},
    {"endtrackid", 0x3b},
    {"startdelay", 0x3c},
    {"fadeinspeed", 0x3d},
    {"fadeoutspeed", 0x3e},
    {"hitlist", 0x3f},
    {"SimSpeed", 0x64},
    {"test_g1", 0x65},
    {"test_g2", 0x66},
    {"test_g3", 0x67},
    {"test_g4", 0x68},
    {"test_g5", 0x69},
    {"test_g6", 0x6a},
    {"test_g7", 0x6b},
    {"test_g8", 0x6c},
    {"test_g9", 0x6d},
    {"main_songnum", 0x6e},
    {"main_musichitlistid", 0x6f},
    {"campfire_nexttrack", 0x70},
    {"campfire_busy", 0x71},
    {"main_duckpri", 0x7b},
    {"main_vol", 0x7c},
    {"main_fxtype", 0x7d},
    {"main_fxlevel", 0x7e},
    {"main_pause", 0x7f}
};

#define TS1GlobalCount 71
static const global_t TS1Globals[] = {
    {"priority", 0x11},
    {"vol", 0x12},
    {"extvol", 0x13},
    {"pan", 0x14},
    {"pitch", 0x15},
    {"paused", 0x16},
    {"fxtype", 0x17},
    {"fxlevel", 0x18},
    {"duckpri", 0x19},
    {"Is3d", 0x1a},
    {"IsHeadRelative", 0x1b},
    {"MinDistance", 0x1c},
    {"MaxDistance", 0x1d},
    {"X", 0x1e},
    {"Y", 0x1f},
    {"Z", 0x20},
    {"filter_type", 0x21},
    {"filter_cutoff", 0x22},
    {"filter_level", 0x23},
    {"attack", 0x24},
    {"decay", 0x25},
    {"IsStreamed", 0x26},
    {"BufSizeMult", 0x27},
    {"fade_dest", 0x28},
    {"fade_var", 0x29},
    {"fade_speed", 0x2a},
    {"Preload", 0x2b},
    {"IsLooped", 0x2c},
    {"fade_on", 0x2d},
    {"isplaying", 0x2e},
    {"source", 0x2f},
    {"patch", 0x32},
    {"WhatToDoWithUpdate", 0x33},
    {"tempo", 0x34},
    {"target", 0x35},
    {"mutegroup", 0x36},
    {"interrupt", 0x37},
    {"IsPositioned", 0x38},
    {"Spl", 0x39},
    {"MultipleInstances", 0x3a},
    {"AssociatedTrack1", 0x3b},
    {"AssociatedTrack2", 0x3c},
    {"AssociatedTrack3", 0x3d},
    {"AssociatedTrack4", 0x3e},
    {"AssociatedTrack5", 0x3f},
    {"AssociatedTrack6", 0x40},
    {"AssociatedTrack7", 0x41},
    {"AssociatedTrack8", 0x42},
    {"SimSpeed", 0x64},
    {"test_g1", 0x65},
    {"test_g2", 0x66},
    {"test_g3", 0x67},
    {"test_g4", 0x68},
    {"test_g5", 0x69},
    {"test_g6", 0x6a},
    {"test_g7", 0x6b},
    {"test_g8", 0x6c},
    {"test_g9", 0x6d},
    {"main_songnum", 0x6e},
    {"main_musichitlistid", 0x6f},
    {"campfire_nexttrack", 0x70},
    {"campfire_busy", 0x71},
    {"main_duckpri", 0x7b},
    {"main_vol", 0x7c},
    {"main_fxtype", 0x7d},
    {"main_fxlevel", 0x7e},
    {"main_pause", 0x7f},
};

static __inline const char * find_global(uint32_t x, const global_t * Globals, size_t GlobalCount){
    size_t i;
    for(i=0; i<GlobalCount; i++)
        if(Globals[i].Value == x)
            return Globals[i].Name;
    return NULL;
}

typedef struct {
    uint8_t * Data;
    size_t Size;
} ByteReaderContext;

enum TokenizeType {
    TK_STRING,
    TK_ID
};

static int parser_find(ByteReaderContext *brc, ...){
    va_list args;
    va_start(args, brc);

    while(1){
        uint8_t * Start = brc->Data;
        const char * Pattern;
        size_t Length;
        void * Destination;
        enum TokenizeType Type;

        Pattern = va_arg(args, const char *);
        if(Pattern == NULL){
            va_end(args);
            return 1;
        }

        for(Length = strlen(Pattern); ; brc->Data++, brc->Size--){
            if(brc->Size < Length){
                va_end(args);
                return 0;
            }
            if(!memcmp(brc->Data, Pattern, Length)) break;
        }
        *brc->Data = '\0';
        brc->Data += Length; brc->Size -= Length;

        Destination = va_arg(args, void *);
        if(Destination == NULL)
            continue;

        Type = va_arg(args, enum TokenizeType);
        if(Type == TK_STRING)
            *((char**)Destination) = (char*)Start;
        else
            *((uint32_t*)Destination) = strtoul((char*)Start, NULL, 0);
    }
}

typedef struct {
    uint32_t LogicalAddress;
    uint32_t TrackID;
    uint32_t SoundID;
    char * Name;
    uint32_t Exported;
} address_t;

typedef struct {
    size_t Size;
    size_t Count;
    address_t * Entries;
} addresslist_t;

static address_t * add_address(addresslist_t * List){
    if(List->Count == List->Size){
        List->Entries = realloc(List->Entries, (List->Size <<= 1) * sizeof(address_t));
        if(!List->Entries)
            Shutdown_M("%sCould not allocate memory for address list.\n", "hitdump: Error: ");
    }
    return memset(List->Entries + List->Count++, 0, sizeof(address_t));
}

static __inline address_t * find_address_by_track_id(addresslist_t * List, uint32_t TrackID){
    unsigned i;
    for(i=0; i<List->Count; i++){
        if(List->Entries[i].TrackID == TrackID)
            return List->Entries + i;
    }
    return NULL;
}

static __inline address_t * find_address_by_sound_id(addresslist_t * List, uint32_t SoundID){
    unsigned i;
    for(i=0; i<List->Count; i++){
        if(List->Entries[i].SoundID == SoundID)
            return List->Entries + i;
    }
    return NULL;
}

static __inline address_t * find_address_by_logical_address(addresslist_t * List, uint32_t LogicalAddress){
    unsigned i;
    for(i=0; i<List->Count; i++){
        if(List->Entries[i].LogicalAddress == LogicalAddress)
            return List->Entries + i;
    }
    return NULL;
}

static __inline address_t * find_address_by_name(addresslist_t * List, const char * Name){
    unsigned i;
    for(i=0; i<List->Count; i++){
        if(List->Entries[i].Name && !strcmp(List->Entries[i].Name, Name))
            return List->Entries + i;
    }
    return NULL;
}

static __inline void read_hit_addresses(uint8_t * Data, size_t Size, addresslist_t * AddressList, uint32_t * SymbolTable){
    uint8_t * TableData;
    unsigned i, count = 0;
    ByteReaderContext brc;
    brc.Data = Data; brc.Size = Size;

    if(!parser_find(&brc, "ENTP", NULL, NULL) || brc.Size < 4) return;
    TableData = brc.Data;
    *SymbolTable = TableData - 4 - Data;

    while(memcmp(Data, "EENT", 4)){
        if(Size < 12) return;
        Data+=8; Size-=8;
        count++;
    }

    for(i=0; i<count; i++){
        address_t * Address = add_address(AddressList);
        Address->Exported       = 1;
        Address->TrackID        = read_uint32(TableData); TableData+=4;
        Address->LogicalAddress = read_uint32(TableData); TableData+=4;
    }
}

static __inline void read_evt_addresses(uint8_t * Data, size_t Size, addresslist_t * AddressList){
    ByteReaderContext brc;
    brc.Data = Data; brc.Size = Size;

    while(1){
        address_t * Address;
        char *Name;
        uint32_t TrackID;
        if(!parser_find(&brc,
            ",", &Name, TK_STRING,
            ",", NULL,
            ",", &TrackID, TK_ID,
        NULL)) return;

        Address = find_address_by_track_id(AddressList, TrackID);
        if(!Address){
            Address = add_address(AddressList);
            Address->Exported = 1;
            Address->TrackID = TrackID;
        }
        Address->Name = Name;
        if(!parser_find(&brc, "\n", NULL, NULL)) return;
    }
}

static __inline void read_hsm_addresses(uint8_t * Data, size_t Size, addresslist_t * AddressList){
    ByteReaderContext brc;
    brc.Data = Data; brc.Size = Size;

    while(1){
        address_t * Address;
        char * Name;
        uint32_t SoundID, LogicalAddress;
        if(!parser_find(&brc,
            "\ntkd_", NULL,
            " ", &Name, TK_STRING,
            " ", &SoundID, TK_ID,
            " ", NULL,
            " ", &LogicalAddress, TK_ID,
        NULL)) return;

        Address = find_address_by_logical_address(AddressList, LogicalAddress);
        if(!Address){
            Address = find_address_by_name(AddressList, (char*)Name);
            if(!Address){
                Address = add_address(AddressList);
                Address->Name = (char*)Name;
            }
            Address->LogicalAddress = LogicalAddress;
        } else Address->Name = Name;
        Address->SoundID = SoundID;
    }
}

static __inline void read_hot_trackdata(uint8_t * Data, size_t Size, addresslist_t * AddressList){
    ByteReaderContext brc;
    brc.Data = Data; brc.Size = Size;

    if(!parser_find(&brc, "[TrackData]", NULL, NULL)) return;

    while(1){
        address_t * Address;
        uint32_t SoundID, LogicalAddress;
        if(!brc.Size || *brc.Data == '\n' || *brc.Data == '[') return;
        if(!parser_find(&brc,
            "=", &SoundID, TK_ID,
            "\n", &LogicalAddress, TK_ID,
        NULL)) return;

        Address = find_address_by_logical_address(AddressList, LogicalAddress);
        if(!Address){
            Address = find_address_by_sound_id(AddressList, SoundID);
            if(!Address){
                Address = add_address(AddressList);
                Address->SoundID = SoundID;
            }
            Address->LogicalAddress = LogicalAddress;
        } else Address->SoundID = SoundID;
    }
}

static __inline void read_hot_track(uint8_t * Data, size_t Size, addresslist_t * AddressList){
    ByteReaderContext brc;
    brc.Data = Data; brc.Size = Size;

    if(!parser_find(&brc, "[Track]", NULL, NULL)) return;

    while(1){
        address_t * Address;
        char * Name;
        uint32_t TrackID;
        if(!brc.Size || *brc.Data == '\n' || *brc.Data == '[') return;
        if(!parser_find(&brc,
            "=", &TrackID, TK_ID,
            ",", NULL,
            ",", &Name, TK_STRING,
        NULL)) return;

        Address = find_address_by_name(AddressList, (char*)Name);
        if(!Address){
            Address = find_address_by_track_id(AddressList, TrackID);
            if(!Address){
                Address = add_address(AddressList);
                Address->TrackID = TrackID;
            }
            Address->Name = Name;
        } else Address->TrackID = TrackID;
        Address->Exported = 1;

        if(!parser_find(&brc, "\n", NULL, NULL)) return;
    }
}

static __inline void read_hot_addresses(uint8_t * Data, size_t Size, addresslist_t * AddressList){
    read_hot_trackdata(Data, Size, AddressList);
    read_hot_track(Data, Size, AddressList);
}

static FILE *hFile = NULL;
static char *path[filecount] = {NULL};
static uint8_t *data[filecount] = {NULL};
static char *basename = NULL;
static addresslist_t AddressList = {0};

static void Shutdown(){
    unsigned i;
    for(i=0; i<filecount; i++){
        free(path[i]);
        free(data[i]);
    }
    free(basename);
    free(AddressList.Entries);
    if(hFile)
        fclose(hFile);
}

static void Shutdown_M(const char * Message, ...){
    va_list args;
    va_start(args, Message);
    vfprintf(stderr, Message, args);
    va_end(args);

    Shutdown();
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){
    unsigned i, j, addr;
    int SimsVersion = 0;
    int overwrite = 0;
    int ShowAddresses = 0;
    int length;
    size_t filesize[filecount-1];
    const global_t * Globals;
    size_t GlobalCount;
    uint32_t SymbolTable = 0;
    uint32_t BaseSoundID = 0, BaseSoundIDSet = 0;

    /****
    ** Parse the command-line arguments
    */

    if(argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        printf("Usage: hitdump [-ts1|-tso] [-f] [-a] [-o outfile.txt] [-hsm infile.hsm]\n"
            "       [-hot infile.hot] [-evt infile.evt] infile.hit\n"
            "Disassemble a HIT binary.\n"
            "\n"
            "The HSM, HOT, and EVT files are not strictly necessary but\n"
            "each help in their own way to provide labels for addresses.\n"
            "Use -f to force overwriting without confirmation.\n"
            "Use -a to show addresses (verbose).\n"
            "\n"
            "Report bugs to <X-Fi6@phppoll.org>.\n"
            "hitutils is maintained by the Niotso project.\n"
            "Home page: <http://www.niotso.org/>\n");
        return 0;
    }

    for(i=1; i<(unsigned)argc-1; i++){
        if(!strcmp(argv[i], "-ts1"))      SimsVersion = VERSION_TS1;
        else if(!strcmp(argv[i], "-tso")) SimsVersion = VERSION_TSO;
        else if(!strcmp(argv[i], "-f"))   overwrite = 1;
        else if(!strcmp(argv[i], "-a"))   ShowAddresses = 1;
        else if(i != (unsigned)argc-2){
            if(!strcmp(argv[i], "-o"))        path[out] = argv[++i];
            else if(!strcmp(argv[i], "-hsm")) path[hsm] = argv[++i];
            else if(!strcmp(argv[i], "-hot")) path[hot] = argv[++i];
            else if(!strcmp(argv[i], "-evt")) path[evt] = argv[++i];
            else break;
        }
        else break;
    }
    path[hit] = argv[i];
    for(j=0; j<filecount; j++)
        if(path[j])
            path[j] = strdup(path[j]); /* necessary for free(path[i]) in Shutdown_M */

    if(!SimsVersion)
        Shutdown_M("%sSims version not specified. (Use -ts1 or -tso.)\n", "hitdump: Error: ");

    if(SimsVersion == VERSION_TS1){
        Globals = TS1Globals;
        GlobalCount = TS1GlobalCount;
    }else{
        Globals = TSOGlobals;
        GlobalCount = TSOGlobalCount;
    }

    length = strlen(path[hit]);
    if(path[out] == NULL){
        path[out] = malloc(max(length+1, 5));
        strcpy(path[out], path[hit]);
        strcpy(path[out] + max(length-4, 0), ".txt");
    }
    length = max(length+1-4, 1);
    basename = malloc(length);
    memcpy(basename, path[hit], length-1);
    basename[length-1] = '\0';

    /****
    ** Read all of the requested files
    */

    for(i=0; i<filecount-1; i++){
        size_t bytestransferred;
        if(!path[i]) continue;

        hFile = fopen(path[i], "rb");
        if(hFile == NULL){
            if(i != hit){
                fprintf(stderr, "%sCould not open file: %s.\n", "hitdump: Warning: ", path[i]);
                continue;
            }else
                Shutdown_M("%sCould not open file: %s.\n", "hitdump: Error: ", path[i]);
        }

        fseek(hFile, 0, SEEK_END);
        filesize[i] = ftell(hFile);
        if(filesize[i] == 0){
            fclose(hFile); hFile = NULL;
            if(i != hit){
                fprintf(stderr, "%sFile is invalid: %s.\n", "hitdump: Warning: ", path[i]);
                continue;
            }else
                Shutdown_M("%sFile is invalid: %s.\n", "hitdump: Error: ", path[i]);
        }

        data[i] = malloc(filesize[i]);
        if(data[i] == NULL){
            fclose(hFile); hFile = NULL;
            if(i != hit){
                fprintf(stderr, "%sCould not allocate memory for file: %s.\n", "hitdump: Warning: ", path[i]);
                continue;
            }else
                Shutdown_M("%sCould not allocate memory for file: %s.\n", "hitdump: Error: ", path[i]);
        }

        fseek(hFile, 0, SEEK_SET);
        bytestransferred = fread(data[i], 1, filesize[i], hFile);
        fclose(hFile); hFile = NULL;
        if(bytestransferred != filesize[i]){
            free(data[i]); data[i] = NULL;
            if(i != hit){
                fprintf(stderr, "%sCould not read file: %s.\n", "hitdump: Warning: ", path[i]);
                continue;
            }else
                Shutdown_M("%sCould not read file: %s.\n", "hitdump: Error: ", path[i]);
        }
    }

    /****
    ** Open the output file for writing
    */

    if(!overwrite){
        hFile = fopen(path[out], "rb");
        if(hFile != NULL){
            /* File exists */
            char c;
            fclose(hFile); hFile = NULL;
            fprintf(stderr, "%sFile \"%s\" exists.\nContinue anyway? (y/n) ", "hitdump: ", path[out]);
            c = getchar();
            if(c != 'y' && c != 'Y')
                Shutdown_M("\nAborted.\n");
        }
    }
    hFile = fopen(path[out], "wb");
    if(hFile == NULL)
        Shutdown_M("%sCould not open file: %s.\n", "hitdump: Error: ", path[out]);

    /****
    ** Verify the header of the HIT file
    */

    if(filesize[hit] < 16 || memcmp(data[hit], HITHeader, 16))
        Shutdown_M("%sFile is invalid: %s.\n", "hitdump: Error: ", path[hit]);

    /****
    ** Build up the address list
    */

    AddressList.Size = 32;
    AddressList.Count = 0;
    AddressList.Entries = malloc(32 * sizeof(address_t));

    read_hit_addresses(data[hit], filesize[hit], &AddressList, &SymbolTable);
    if(data[evt]) read_evt_addresses(data[evt], filesize[evt], &AddressList);
    if(data[hsm]) read_hsm_addresses(data[hsm], filesize[hsm], &AddressList);
    if(data[hot]) read_hot_addresses(data[hot], filesize[hot], &AddressList);
    /* scan_branch_destinations(data[hit], filesize[hit], &AddressList); */

    for(i=0; i<AddressList.Count; i++){
        if(AddressList.Entries[i].SoundID != 0 && (!BaseSoundIDSet || AddressList.Entries[i].SoundID < BaseSoundID)){
            BaseSoundID = AddressList.Entries[i].SoundID;
            BaseSoundIDSet = 1;
        }

        if(ShowAddresses){
            printf("Address %u:\n  Exported: %u\n  TrackID: %u\n  SoundID: %u\n  Name: %s\n  LogicalAddress: %u\n", i,
                AddressList.Entries[i].Exported,
                AddressList.Entries[i].TrackID,
                AddressList.Entries[i].SoundID,
                AddressList.Entries[i].Name ? AddressList.Entries[i].Name : "",
                AddressList.Entries[i].LogicalAddress
            );
        }
    }

    /****
    ** Perform the disassembly
    */

    fprintf(hFile, "BASEID_TRACKDATA %u\r\n"
        "\r\n"
        ";\r\n"
        "; generated by hitdump.\r\n"
        ";\r\n"
        "\r\n"
        "; useful symbols:\r\n"
        "; kSndobPlay = 1\r\n"
        "; tkd_Generic 1\r\n"
        "; tkd_GenericLooped 2\r\n"
        "; tkd_GenericHitList 3\r\n"
        "\r\n"
        "INCLUDE defaultsyms.txt\r\n"
        "INCLUDE SimsGlobals.txt\r\n"
        "\r\n"
        "LIST [Options] Version=1\r\n"
        "LIST [Options] LoadPriority=2\r\n"
        "\r\n"
        ";LIST [EventMappingEquate] kSndobPlay=1\r\n"
        "; --- end of standard intro text ---\r\n"
        "\r\n"
        "SYMBOLFILE %s%s\r\n"
        "INIFILE %s.ini", BaseSoundID, path[hsm] ? path[hsm] : basename, path[hsm] ? "" : ".hsm", basename);

    fprintf(hFile, "\r\n\r\nBINARY\r\n[");

    for(addr=16; addr<filesize[hit];){
        unsigned i;
        uint8_t opcode;
        const instruction_t * instruction;
        uint32_t operands;
        const address_t * Address;
        int HadSymbolTable = 0;

        if(SymbolTable && addr == SymbolTable){
            if(addr != 16)
                fprintf(hFile, "\r\n]\r\n\r\nBINARY\r\n[");
            fprintf(hFile, "\r\n"
                "\t69\r\n"
                "\t78\r\n"
                "\t84\r\n"
                "\t80\r\n");

            for(addr+=4; memcmp(data[hit]+addr, "EENT", 4); addr+=8){
                uint32_t TrackID = read_uint32(data[hit]+addr), LogicalAddress = read_uint32(data[hit]+addr+4);

                Address = find_address_by_logical_address(&AddressList, LogicalAddress);
                fprintf(hFile, "\r\n\t#%u\t\t#", TrackID);
                if(Address && Address->Name) fprintf(hFile, "%s", Address->Name);
                else                         fprintf(hFile, "%u", LogicalAddress);
            }

            if(addr-4 != SymbolTable)
                fprintf(hFile, "\r\n");
            fprintf(hFile, "\r\n"
                "\t69\r\n"
                "\t69\r\n"
                "\t78\r\n"
                "\t84");

            if(addr+4 == filesize[hit])
                break;
            fprintf(hFile, "\r\n]\r\n\r\nBINARY\r\n[");

            addr += 4;
            SymbolTable = 0;
            HadSymbolTable++;
        }

        Address = find_address_by_logical_address(&AddressList, addr);
        if(Address){
            if(!HadSymbolTable && addr != 16 && Address->Exported)
                fprintf(hFile, "\r\n]\r\n\r\nBINARY\r\n[");
            if(Address->Name)
                fprintf(hFile, "\r\n%s", Address->Name);
        }

        opcode = data[hit][addr];
        if(opcode == 0 || opcode > 96)
            Shutdown_M("%sIllegal opcode 0x%02X at address 0x%08X.\n", "hitdump: Error: ", opcode, addr);

        instruction = Instructions + opcode - 1;
        operands = instruction->Operands;
        if(operands == UNIMPLEMENTED)
            Shutdown_M("%sUnimplemented instruction '%s' at address 0x%08X.\n", "hitdump: Error: ", instruction->Name, addr);

        addr++;

        if(filesize[hit] - addr < (operands & 15))
            Shutdown_M("%sInsufficient operand bytes for '%s' instruction at address 0x%08X (%u of %u supplied).\n",
                "hitdump: Error: ", instruction->Name, addr, filesize[hit] - addr, instruction->Operands);

        fprintf(hFile, "\r\n\t\t%s", instruction->Name);
        for(i=0; (operands >>= 4) != 0; i++){
            int type = operands & 15;
            const char *position[] = {"first","second","third","fourth"};
            if(type == o_byte){
                fprintf(hFile, " #%u", data[hit][addr]);
                addr += 1;
            }else if(type == o_dword){
                fprintf(hFile, " #%u", read_uint32(data[hit]+addr));
                addr += 4;
            }else if(type == o_address){
                int LogicalAddress = read_uint32(data[hit]+addr);

                Address = find_address_by_logical_address(&AddressList, LogicalAddress);
                if(Address && Address->Name) fprintf(hFile, " #%s", Address->Name);
                else                         fprintf(hFile, " #%u", LogicalAddress);
                addr += 4;
            }else if(type == o_variable){
                int x = data[hit][addr];
                if(x > 16){
                    const char * Global = find_global(x, Globals, GlobalCount);
                    if(Global == NULL)
                        Shutdown_M("%sInvalid %s operand 0x%02X for '%s' instruction at address 0x%08X (expected %s).\n",
                            "hitdump: Error: ", position[i], x, instruction->Name, addr, "argument, register, or global");
                    fprintf(hFile, " %s", Global);
                } else fprintf(hFile, " %s", Registers[x]);
                addr += 1;
            }else if(type == o_jump){
                unsigned x = 0;

                if(filesize[hit]-addr >= 4)
                    x = read_uint32(data[hit]+addr);
                else if(data[hit][addr] != 0x05 && data[hit][addr] != 0x06)
                    Shutdown_M("%sInsufficient operand bytes for '%s' instruction at address 0x%08X (%u of %u supplied).\n",
                        "hitdump: Error: ", instruction->Name, addr, filesize[hit] - addr, 4);

                if(x >= 16 && x < filesize[hit]){
                    Address = find_address_by_logical_address(&AddressList, x);
                    if(Address && Address->Name) fprintf(hFile, " #%s", Address->Name);
                    else                         fprintf(hFile, " #%u", x);
                    addr += 4;
                }else{
                    x = data[hit][addr];
                    if(x > 16){
                        const char * Global = find_global(x, Globals, GlobalCount);
                        if(Global == NULL)
                            Shutdown_M("%sInvalid %s operand 0x%02X for '%s' instruction at address 0x%08X (expected %s).\n",
                                "hitdump: Error: ", position[i], x, instruction->Name, addr, "argument, register, or global");
                        fprintf(hFile, " %s", Global);
                    } else fprintf(hFile, " %s", Registers[x]);
                    addr += (data[hit][addr] != 0x05 && data[hit][addr] != 0x06) ? 4 : 1;
                }
            }
        }
    }

    fprintf(hFile, "\r\n]\r\n\r\n");

    Shutdown();

    return 0;
}