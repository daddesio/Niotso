/*
    tsoscan - IFF statistical webpage generator
    tsoscan.h - Copyright (c) 2012 Niotso Project <http://niotso.org/>
    Author(s): Ahmed El-Mahdawy <aa.mahdawy.10@gmail.com>

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

#include <stdint.h>

#define TSOSCAN_WARNING "tsoscan: warning: "
#define TSOSCAN_ERROR "tsoscan: error: "
#ifdef _WIN32
 #define PATH_SEP '\\'
#else
 #define PATH_SEP '/'
#endif

typedef struct CommandLineArgs_s
{
    int ForceWrite;
    char * OutFile;
    unsigned InDirCount;
    char ** InDirs;
} CommandLineArgs;

typedef struct VersionInfo_s
{
    unsigned Version;
    unsigned Count;
} VersionInfo;

typedef struct ChunkStats_s
{
    char Type[5];
    unsigned ChunkCount;
    unsigned VersionCount;
    VersionInfo * Versions;
} ChunkStats;

typedef struct IFFStats_s
{
    unsigned FileCount;
    float AverageChunkCount;
    unsigned ChunkTypeCount;
    ChunkStats * ChunkTypes;
} IFFStats;

CommandLineArgs* cmd_parse_args(int argc, char *argv[]);
void cmd_delete(CommandLineArgs *args);

int stats_create(IFFStats *stats);
int stats_version_increment(IFFStats *stats, char *type, unsigned version);
unsigned stats_get_version(char *type, uint8_t *data);
void stats_delete(IFFStats *stats);