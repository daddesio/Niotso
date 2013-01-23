/*
    hitutils - The Sims HIT (dis)assembler and linker
    hitutils.h - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#ifndef write_int32
 #define write_uint32(dest, src) do {\
    (dest)[0] = ((src)&0x000000FF)>>(8*0); \
    (dest)[1] = ((src)&0x0000FF00)>>(8*1); \
    (dest)[2] = ((src)&0x00FF0000)>>(8*2); \
    (dest)[3] = ((src)&0xFF000000)>>(8*3); \
    } while(0)
 #define write_uint16(dest, src) do {\
    (dest)[0] = ((src)&0x00FF)>>(8*0); \
    (dest)[1] = ((src)&0xFF00)>>(8*1); \
    } while(0)
#endif

extern char *strdup (const char *__s);

static void Shutdown();

static void Shutdown_M(const char * Message, ...){
    va_list args;
    va_start(args, Message);
    vfprintf(stderr, Message, args);
    va_end(args);

    Shutdown();
    exit(EXIT_FAILURE);
}

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
} variable_t;

typedef struct {
    uint8_t * Data;
    size_t Size;
} ByteReaderContext;

static const uint8_t HITHeader[] = {'H','I','T','!',0x01,0x00,0x00,0x00,0x08,0x00,0x00,0x00,'T','R','A','X'};

#define InstructionCount 96
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

#define TSOVariableCount 82
static const variable_t TSOVariables[] = {
    {"arg0", 0x00},
    {"arg1", 0x01},
    {"arg2", 0x02},
    {"arg3", 0x03},
    {"arg4", 0x04},
    {"v1", 0x05},
    {"v2", 0x06},
    {"v3", 0x07},
    {"v4", 0x08},
    {"v5", 0x09},
    {"v6", 0x0A},
    {"v7", 0x0B},
    {"v8", 0x0C},
    {"h1", 0x0D},
    {"h2", 0x0E},
    {"h3", 0x0F},
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
    {"main_pause", 0x7f},
};

#define TS1VariableCount 87
static const variable_t TS1Variables[] = {
    {"arg0", 0x00},
    {"arg1", 0x01},
    {"arg2", 0x02},
    {"arg3", 0x03},
    {"arg4", 0x04},
    {"v1", 0x05},
    {"v2", 0x06},
    {"v3", 0x07},
    {"v4", 0x08},
    {"v5", 0x09},
    {"v6", 0x0a},
    {"v7", 0x0b},
    {"v8", 0x0c},
    {"h1", 0x0d},
    {"h2", 0x0e},
    {"h3", 0x0f},
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

#define ConstantCount 72
static const variable_t Constants[] = {
    {"duckpri_always", 0x0},
    {"duckpri_low", 0xa},
    {"duckpri_normal", 0x14},
    {"duckpri_high", 0x1e},
    {"duckpri_higher", 0x28},
    {"duckpri_evenhigher", 0x32},
    {"duckpri_never", 0x64},
    {"spl_infinite", 0x0},
    {"spl_loud", 0xa},
    {"spl_normal", 0x14},
    {"spl_quiet", 0x64},
    {"Instance", 0x0},
    {"Gender", 0x1},
    {"GroupMusic", 0x1},
    {"GroupDialogMain", 0x2},
    {"GroupDialogOverlay", 0x3},
    {"PchDishwasherClose", 0x32},
    {"PchDishwasherLoad", 0x33},
    {"PchDishwasherLoad2", 0x34},
    {"PchDishwasherOpen", 0x35},
    {"PchDishwasherTurnDial", 0x39},
    {"PchDishwasherTurnDial2", 0x3a},
    {"TrkRadioStationCountry", 0x104},
    {"TrkRadioStationBossaNova", 0x10e},
    {"TrkRadioStationClassical", 0x10d},
    {"TrkRadioStationRock", 0x118},
    {"kSndobPlay", 1},
    {"kSndobStop", 2},
    {"kSndobKill", 3},
    {"kSndobUpdate", 4},
    {"kSndobSetVolume", 5},
    {"kSndobSetPitch", 6},
    {"kSndobSetPan", 7},
    {"kSndobSetPosition", 8},
    {"kSndobSetFxType", 9},
    {"kSndobSetFxLevel", 10},
    {"kSndobPause", 11},
    {"kSndobUnpause", 12},
    {"kSndobLoad", 13},
    {"kSndobUnload", 14},
    {"kSndobCache", 15},
    {"kSndobUncache", 16},
    {"kSndobCancelNote", 19},
    {"kPlayPiano", 43},
    {"kSetMusicMode", 36},
    {"kLive", 0},
    {"kBuy", 1},
    {"kBuild", 2},
    {"kHood", 3},
    {"kFrontEnd", 4},
    {"kGroupSfx", 1},
    {"kGroupMusic", 2},
    {"kGroupVox", 3},
    {"kAction", 1000},
    {"kComedy", 1001},
    {"kRomance", 1002},
    {"kNews", 1003},
    {"kCountry", 1004},
    {"kRock", 1005},
    {"kJazz", 1006},
    {"kClassical", 1007},
    {"kArgsNormal", 0},
    {"kArgsVolPan", 1},
    {"kArgsIdVolPan", 2},
    {"kArgsXYZ", 3},
    {"kKillAll", 20},
    {"kPause", 21},
    {"kUnpause", 22},
    {"kKillInstance", 23},
    {"kTurnOnTv", 30},
    {"kTurnOffTv", 31},
    {"kUpdateSourceVolPan", 32},
};

static const char SHStringTable[] =
    "\0.text"
    "\0.shstrtab"
    "\0.symtab"
    "\0.strtab"
    "\0.rel.text"
;

static const uint8_t SymbolTableHeader[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0xF1, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x00
};