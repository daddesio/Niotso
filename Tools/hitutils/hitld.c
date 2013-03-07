/*
    hitutils - The Sims HIT (dis)assembler and linker
    hitld.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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
#include "hitutils.h"

static const uint8_t ObjectHeader[] = {0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01};
static const uint8_t ArchiveHeader[] = {'!', '<', 'a', 'r', 'c', 'h', '>', '\n'};

enum {
    hsm, hot, out, filecount
};

typedef struct {
    char *path;
    uint8_t *data;
} object_t;

typedef struct {
    size_t SizeAllocated;
    size_t Count;
    object_t * Entries;
} objectlist_t;

static object_t * add_object(objectlist_t * List){
    if(List->Count*sizeof(object_t) == List->SizeAllocated){
        void * ptr;
        if(List->SizeAllocated > SIZE_MAX/2 || !(ptr = realloc(List->Entries, List->SizeAllocated<<=1)))
            Shutdown_M("%sCould not allocate memory for object list.\n", "hitld: Error: ");
        List->Entries = ptr;
    }
    return memset(List->Entries + List->Count++, 0, sizeof(object_t));
}

static FILE *hFile = NULL;
static char *path[filecount] = {NULL};
static uint8_t *data[filecount] = {NULL};
static objectlist_t ObjectList = {0};

static void Shutdown(){
    unsigned i;
    for(i=0; i<filecount; i++){
        free(path[i]);
        free(data[i]);
    }
    for(i=0; i<ObjectList.Count; i++){
        free(ObjectList.Entries[i].path);
        free(ObjectList.Entries[i].data);
    }
    free(ObjectList.Entries);
    if(hFile)
        fclose(hFile);
}

int main(int argc, char *argv[]){
    unsigned i;
    unsigned ObjectArg;
    int SimsVersion = 0;
    int overwrite = 0;
    size_t filesize[filecount-1];

    if(argc < 3){
        printf("Usage: hitld [-ts1|-tso] [-f] [-o outfile.hit] [-hsm infile.hsm]\n"
            "       [-hot infile.hot] INFILES\n"
            "Link object files produced by hitasm into a HIT binary, and\n"
            "relink the game's HSM and HOT files.\n"
            "Use -f to force overwriting without confirmation.\n"
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
        else if(i != (unsigned)argc-2){
            if(!strcmp(argv[i], "-o"))        path[out] = argv[++i];
            else if(!strcmp(argv[i], "-hsm")) path[hsm] = argv[++i];
            else if(!strcmp(argv[i], "-hot")) path[hot] = argv[++i];
            else break;
        }
        else break;
    }
    ObjectArg = i;
    for(i=0; i<filecount; i++)
        if(path[i])
            path[i] = strdup(path[i]); /* necessary for free(path[i]) in Shutdown_M */

    if(!SimsVersion)
        Shutdown_M("%sSims version not specified. (Use -ts1 or -tso.)\n", "hitasm: Error: ");

    if(path[out] == NULL){
        int length = strlen(argv[ObjectArg]);
        path[out] = malloc(max(length+1+2, 5));
        strcpy(path[out], argv[ObjectArg]);
        strcpy(path[out] + max(length-2, 0), ".hit");
    }

    /****
    ** Read all of the requested files
    */

    for(i=0; i<filecount-1; i++){
        size_t bytestransferred;
        if(!path[i]) continue;

        hFile = fopen(path[i], "rb");
        if(hFile == NULL)
            Shutdown_M("%sCould not open file: %s.\n", "hitasm: Error: ", path[i]);

        fseek(hFile, 0, SEEK_END);
        filesize[i] = ftell(hFile);
        if(filesize[i] == 0)
            Shutdown_M("%sFile is invalid: %s.\n", "hitasm: Error: ", path[i]);

        data[i] = malloc(filesize[i]);
        if(data[i] == NULL)
            Shutdown_M("%sCould not allocate memory for file: %s.\n", "hitasm: Error: ", path[i]);

        fseek(hFile, 0, SEEK_SET);
        bytestransferred = fread(data[i], 1, filesize[i], hFile);
        fclose(hFile); hFile = NULL;
        if(bytestransferred != filesize[i])
            Shutdown_M("%sCould not read file: %s.\n", "hitasm: Error: ", path[i]);
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

    fwrite(HITHeader, 1, sizeof(HITHeader), hFile);

    Shutdown();

    return 0;
}