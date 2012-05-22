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

#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include "iff.h"

int charmatches(char c, const char * filter){
    while(*filter){
        if(c == *filter) return 1;
        filter++;
    }
    return 0;
}

int main(int argc, char *argv[]){
    HANDLE hFile;
    int overwrite = 0;
    char *InFile, *OutDirectory;
    HANDLE ProcessHeap = GetProcessHeap();
    DWORD FileSize;
    DWORD bytestransferred = 0;
    uint8_t * IFFData;
    unsigned chunkcount, chunk;
    IFFFile * IFFFileInfo;
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

    hFile = CreateFile(InFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if(hFile == INVALID_HANDLE_VALUE){
        if(GetLastError() == ERROR_FILE_NOT_FOUND){
            printf("%sThe specified input file does not exist.", "iffexport: error: ");
            return -1;
        }
        printf("%sThe input file could not be opened for reading.", "iffexport: error: ");
        return -1;
    }
    FileSize = GetFileSize(hFile, NULL);
    if(FileSize < 64){
        printf("%sNot a valid IFF file.", "iffexport: error: ");
        return -1;
    }
    IFFData = HeapAlloc(ProcessHeap, HEAP_NO_SERIALIZE, FileSize);
    if(IFFData == NULL){
        printf("%sMemory for this file could not be allocated.", "iffexport: error: ");
        return -1;
    }
    if(!ReadFile(hFile, IFFData, FileSize, &bytestransferred, NULL) || bytestransferred != FileSize){
        printf("%sThe input file could not be read.", "iffexport: error: ");
        return -1;
    }
    CloseHandle(hFile);

    /****
    ** Load header information
    */

    IFFFileInfo = iff_create();
    if(IFFFileInfo == NULL){
        printf("%sMemory for this file could not be allocated.", "iffexport: error: ");
        return -1;
    }
    if(!iff_read_header(IFFFileInfo, IFFData, FileSize)){
        printf("%sNot a valid IFF file.", "iffexport: error: ");
        return -1;
    }

    /****
    ** Load entry information
    */

    if(!iff_enumerate_chunks(IFFFileInfo, IFFData+64, FileSize-64)){
        printf("%sChunk data is corrupt.", "iffexport: error: ");
        return -1;
    }

    chunkcount = IFFFileInfo->ChunkCount;
    printf("This IFF file contains %u chunks.\n\nExporting\n", chunkcount);

    /****
    ** Extract each entry
    */
    for(chunk = 1, ChunkData = IFFFileInfo->Chunks; chunk <= chunkcount; chunk++, ChunkData++){
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

        hFile = CreateFile(destination, GENERIC_WRITE, 0, NULL, CREATE_NEW+overwrite,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if(hFile == INVALID_HANDLE_VALUE){
            printf(" (%u/%u) Skipped (%s): %s\n", chunk, chunkcount,
                (!overwrite && GetLastError() == ERROR_FILE_EXISTS) ? "file exists" : "could not open",
                name);
            continue;
        }

        printf(" (%u/%u) %s (%u bytes)\n", chunk, chunkcount, name, ChunkData->Size-76);

        WriteFile(hFile, ChunkData->Data, ChunkData->Size-76, &bytestransferred, NULL);
        CloseHandle(hFile);
    }

    return 0;
}