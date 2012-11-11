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

int charmatches(char c, const char * filter){
    while(*filter){
        if(c == *filter) return 1;
        filter++;
    }
    return 0;
}

int main(int argc, char *argv[]){
    FILE * hFile;
    int overwrite = 0;
    char *InFile, *OutDirectory;
    size_t FileSize;
    uint8_t * IFFData;
    clock_t BeginningTime;
    unsigned chunkcount, chunk;
    unsigned exported = 0;
    IFFFile IFFFileInfo;
    IFFChunk * ChunkData;

    if(argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
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
    if(hFile == NULL){
        printf("%sThe specified input file does not exist or could not be opened for reading.", "iffexport: error: ");
        return -1;
    }
    fseek(hFile, 0, SEEK_END);
    FileSize = ftell(hFile);
    if(FileSize < 24){
        printf("%sNot a valid IFF file.", "iffexport: error: ");
        return -1;
    }
    fseek(hFile, 0, SEEK_SET);

    IFFData = malloc(FileSize);
    if(IFFData == NULL){
        printf("%sMemory for this file could not be allocated.", "iffexport: error: ");
        return -1;
    }
    if(!fread(IFFData, FileSize, 1, hFile)){
        printf("%sThe input file could not be read.", "iffexport: error: ");
        return -1;
    }
    fclose(hFile);

    /****
    ** Load header information
    */

    if(!iff_create(&IFFFileInfo)){
        printf("%sMemory for this file could not be allocated.", "iffexport: error: ");
        return -1;
    }
    if(!iff_read_header(&IFFFileInfo, IFFData, FileSize)){
        printf("%sNot a valid IFF file.", "iffexport: error: ");
        return -1;
    }

    /****
    ** Load entry information
    */

    if(!iff_enumerate_chunks(&IFFFileInfo, IFFData+64, FileSize-64)){
        printf("%sChunk data is corrupt.", "iffexport: error: ");
        return -1;
    }

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
                fclose(hFile);
                printf("File \"%s\" exists.\nContinue anyway? (y/n) ", destination);
                c = getchar();
                if(c != 'y' && c != 'Y'){
                    printf("\nAborted.");
                    return -1;
                }
            }
            overwrite++;
        }
        hFile = fopen(destination, "wb");
        if(hFile == NULL){
            printf("%sThe output file could not be opened for writing.", "iffexport: error: ");
            return -1;
        }
        fwrite(ChunkData->Data, 1, ChunkData->Size, hFile);
        fclose(hFile);

        exported++;
    }

    printf("\nExported %u of %u chunks in %.2f seconds.", exported, chunkcount,
        ((float) (clock() - BeginningTime))/CLOCKS_PER_SEC);

    return 0;
}