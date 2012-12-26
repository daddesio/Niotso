/*
    FileHandler - General-purpose file handling library for Niotso
    iffexport.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "iff.h"

static FILE * hFile = NULL;
static uint8_t * IFFData = NULL;
static IFFFile IFFFileInfo;
static int iffcreated = 0;

static void Shutdown_M(const char * Message){
    fprintf(stderr, "iffexport: error: %s.\n", Message);
    if(iffcreated)
        iff_delete(&IFFFileInfo);
    free(IFFData);
    if(hFile)
        fclose(hFile);
    exit(EXIT_FAILURE);
}

static int charmatches(char c, const char * filter){
    while(*filter){
        if(c == *filter) return 1;
        filter++;
    }
    return 0;
}

int main(int argc, char *argv[]){
    int overwrite = 0;
    const char *InFile, *OutDirectory;
    size_t FileSize;
    clock_t BeginningTime;
    unsigned chunkcount, chunk;
    unsigned exported = 0;
    IFFChunk * ChunkData;

    if(argc < 3 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        printf("Usage: iffexport [-f] infile outdirectory\n"
        "Export the resources of an EA IFF file.\n"
        "Use -f to force overwriting without confirmation.\n"
        "\n"
        "Report bugs to <X-Fi6@phppoll.org>.\n"
        "iffexport is maintained by the Niotso project.\n"
        "Home page: <http://www.niotso.org/>\n");
        return 0;
    }

    if(argc >= 4 && !strcmp(argv[1], "-f")){
        overwrite++;
        InFile = argv[2];
        OutDirectory = argv[3];
    }else{
        InFile = argv[1];
        OutDirectory = argv[2];
    }

    /****
    ** Open the file and read in entire contents to memory
    */

    hFile = fopen(InFile, "rb");
    if(hFile == NULL)
        Shutdown_M("The specified input file does not exist or could not be opened for reading");
    fseek(hFile, 0, SEEK_END);
    FileSize = ftell(hFile);
    if(FileSize < 24)
        Shutdown_M("Not a valid IFF file");
    fseek(hFile, 0, SEEK_SET);

    IFFData = malloc(FileSize);
    if(IFFData == NULL)
        Shutdown_M("Memory for this file could not be allocated");
    if(fread(IFFData, 1, FileSize, hFile) != FileSize)
        Shutdown_M("The input file could not be read");
    fclose(hFile); hFile = NULL;

    /****
    ** Load header information
    */

    if(!iff_create(&IFFFileInfo))
        Shutdown_M("Memory for this file could not be allocated");
    iffcreated++;
    if(!iff_read_header(&IFFFileInfo, IFFData, FileSize))
        Shutdown_M("Not a valid IFF file");

    /****
    ** Load entry information
    */

    if(!iff_enumerate_chunks(&IFFFileInfo, IFFData+64, FileSize-64))
        Shutdown_M("Chunk data is corrupt");

    free(IFFData); IFFData = NULL;

    chunkcount = IFFFileInfo.ChunkCount;
    printf("This IFF file contains %u chunks.\n\nExporting\n", chunkcount);
    BeginningTime = clock();

    /****
    ** Extract each entry
    */
    for(chunk = 1, ChunkData = IFFFileInfo.Chunks; chunk <= chunkcount; chunk++, ChunkData++){
        char name[256], destination[256];
        char filter[] = "\\/:*?\"<>|";
        int i;

        sprintf(name, "%03u-%s-%04X-%s", chunk, ChunkData->Type, ChunkData->ChunkID, ChunkData->Label);
        for(i=0; name[i] != 0x00; i++){
            if(name[i] == '\t') name[i] = ' ';
            else if(name[i] < ' ' || name[i] > '~') name[i] = '.';
            else if(charmatches(name[i], "\\/:*?\"<>|")) name[i] = '.';
        }
        for(i=0; i<9; i++){
            char * c = name;
            while((c = strchr(c, filter[i])) != NULL)
                *c = '.';
        }
        sprintf(destination, "%s/%s.%s", OutDirectory, name,
            (!memcmp(ChunkData->Type, "BMP_", 4) || !memcmp(ChunkData->Type, "FBMP", 4)) ? "bmp" : "dat");

        if(!overwrite){
            hFile = fopen(destination, "rb");
            if(hFile != NULL){
                /* File exists */
                char c;
                fclose(hFile); hFile = NULL;
                printf("File \"%s\" exists.\nContinue anyway? (y/n) ", destination);
                c = getchar();
                if(c != 'y' && c != 'Y'){
                    printf("\n");
                    Shutdown_M("Aborted");
                }
            }
            overwrite++;
        }
        hFile = fopen(destination, "wb");
        if(hFile == NULL)
            Shutdown_M("The output file could not be opened for writing");
        fwrite(ChunkData->Data, 1, ChunkData->Size, hFile);
        fclose(hFile);

        exported++;
    }

    printf("\nExported %u of %u chunks in %.2f seconds.", exported, chunkcount,
        ((float) (clock() - BeginningTime))/CLOCKS_PER_SEC);

    iff_delete(&IFFFileInfo);

    return 0;
}