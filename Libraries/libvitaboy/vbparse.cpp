/*
    libvitaboy - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>

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

#include <windows.h>
#include "libvitaboy.hpp"

enum VBFileType {
    VBFILE_ANIM,
    VBFILE_APR,
    VBFILE_BND,
    VBFILE_COL,
    VBFILE_HAG,
    VBFILE_MESH,
    VBFILE_OFT,
    VBFILE_PO,
    VBFILE_SKEL
};

int main(int argc, char *argv[]){
    int type;
    char * InFile;
    
    if(argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        printf("Usage: vbparse [-t type] infile\n"
        "Parse a TSO VitaBoy file.\n"
        "\n"
        "Supported types:\n"
        " (*) ANIM - Animation\n"
        " (*) APR  - Appearance\n"
        " (*) BND  - Binding\n"
        " (*) COL  - Collection\n"
        " (*) HAG  - Hand group\n"
        " (*) MESH - Mesh\n"
        " (*) OFT  - Outfit\n"
        " (*) PO   - Purchasable object\n"
        " (*) SKEL - Skeleton\n"
        "\n"
        "Report bugs to <X-Fi6@phppoll.org>.\n"
        "vbparse is maintained by the Niotso project.\n"
        "Home page: <http://www.niotso.org/>");
        return 0;
    }
    
    if(argc >= 4 && !strcmp(argv[1], "-t")){
        if(!stricmp(argv[2], "anim"))      type = 0;
        else if(!stricmp(argv[2], "apr"))  type = 1;
        else if(!stricmp(argv[2], "bnd"))  type = 2;
        else if(!stricmp(argv[2], "col"))  type = 3;
        else if(!stricmp(argv[2], "hag"))  type = 4;
        else if(!stricmp(argv[2], "mesh")) type = 5;
        else if(!stricmp(argv[2], "oft"))  type = 6;
        else if(!stricmp(argv[2], "po"))   type = 7;
        else if(!stricmp(argv[2], "skel")) type = 8;
        else{
            printf("%sUnrecognized type '%s'", "vbparse: Error: ", argv[2]);
            return -1;
        }
        InFile = argv[3];
    }else{
        char * pos = strrchr(argv[1], '.') + 1;
        if(!stricmp(pos, "anim"))      type = 0;
        else if(!stricmp(pos, "apr"))  type = 1;
        else if(!stricmp(pos, "bnd"))  type = 2;
        else if(!stricmp(pos, "col"))  type = 3;
        else if(!stricmp(pos, "hag"))  type = 4;
        else if(!stricmp(pos, "mesh")) type = 5;
        else if(!stricmp(pos, "oft"))  type = 6;
        else if(!stricmp(pos, "po"))   type = 7;
        else if(!stricmp(pos, "skel")) type = 8;
        else{
            printf("%sUnrecognized type", "vbparse: Error: ");
            return -1;
        }
        InFile = argv[1];
    }
    
    HANDLE ProcessHeap = GetProcessHeap();
    HANDLE hFile;
    unsigned FileSize;
    uint8_t *InData;
    DWORD bytestransferred;
    
    hFile = CreateFile(InFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if(hFile == INVALID_HANDLE_VALUE){
        if(GetLastError() == ERROR_FILE_NOT_FOUND){
            printf("%sThe specified input file does not exist.", "vbparse: Error: ");
            return -1;
        }
        printf("%sThe input file could not be opened for reading.", "vbparse: Error: ");
        return -1;
    }
    FileSize = GetFileSize(hFile, NULL);
    InData = (uint8_t*) HeapAlloc(ProcessHeap, HEAP_NO_SERIALIZE, FileSize);
    if(InData == NULL){
        printf("%sMemory for this file could not be allocated.", "vbparse: Error: ");
        return -1;
    }
    if(!ReadFile(hFile, InData, FileSize, &bytestransferred, NULL) || bytestransferred != FileSize){
        printf("%sThe input file could not be read.", "vbparse: Error: ");
        return -1;
    }
    CloseHandle(hFile);
    
    VBFile.set(InData, FileSize);
    
    if(type == VBFILE_ANIM){
        Animation_t Animation;
        ReadAnimation(Animation);
    }
    
    return 0;
}