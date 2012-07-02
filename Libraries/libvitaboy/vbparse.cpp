/*
    libvitaboy - Open source OpenGL TSO character animation library
    vbparse.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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
#include <strings.h>
#include <FileHandler.hpp>
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

    uint8_t *InData = File::ReadFile(InFile);
    if(InData == NULL){
        const char * Message;
        switch(File::Error){
        case FERR_NOT_FOUND:
            Message = "%s does not exist.";
            break;
        case FERR_OPEN:
            Message = "%s could not be opened for reading.";
            break;
        case FERR_BLANK:
            Message = "%s is corrupt or invalid.";
            break;
        case FERR_MEMORY:
            Message = "Memory for %s could not be allocated.";
            break;
        default:
            Message = "%s could not be read.";
            break;
        }
        printf(Message, InFile);
        return -1;
    }

    VBFile.set(InData, File::FileSize);

    switch(type){
    case VBFILE_ANIM:
        Animation_t Animation;
        ReadAnimation(Animation);
        break;
    case VBFILE_APR:
        Appearance_t Appearance;
        ReadAppearance(Appearance);
        break;
    case VBFILE_BND:
        Binding_t Binding;
        ReadBinding(Binding);
        break;
    case VBFILE_COL:
        Collection_t Collection;
        ReadCollection(Collection);
        break;
    case VBFILE_HAG:
        HandGroup_t HandGroup;
        ReadHandGroup(HandGroup);
        break;
    case VBFILE_MESH:
        Mesh_t Mesh;
        ReadMesh(Mesh);
        break;
    case VBFILE_OFT:
        Outfit_t Outfit;
        ReadOutfit(Outfit);
        break;
    case VBFILE_PO:
        PurchasableOutfit_t PurchasableOutfit;
        ReadPurchasableOutfit(PurchasableOutfit);
        break;
    case VBFILE_SKEL:
        Skeleton_t Skeleton;
        ReadSkeleton(Skeleton);
    }

    return 0;
}
