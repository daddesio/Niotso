/*
    FileHandler - General-purpose file handling library for Niotso
    farextract.c - Copyright (c) 2011 Niotso Project <http://niotso.org/>
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
#include "far.h"

enum {
    profile_ts1 = 1,
    profile_tso,
    profile_sc4,
    profile_ts2,
    profile_spore,
    profile_ts3
};

int main(int argc, char *argv[]){
    int profile = 0, overwrite = 0;
    char infile[256] = "", outdirectory[256] = "";
    FILE * hFile;
    size_t ArchiveSize;
    uint8_t * ArchiveData;
    int ArchiveType;
    clock_t BeginningTime, EndingTime;
    unsigned extracted = 0;
    int i;

    /****
    ** Check the arguments
    */

    if(argc == 1 || (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))){
        printf("Usage: farextract [OPTIONS] [FILE] [OUTDIRECTORY]\n"
            "Extract the contents of a FAR, DBPF, or Persist file.\n"
            "With no FILE, or when FILE is -, read from standard input.\n"
            "\n"
            "Profile options:\n"
            "  -ts1, -tso, -sc4,       Select presets suitable for The Sims 1,\n"
            "    -ts2, -spore, -ts3    The Sims Online, SimCity 4, Spore, or The Sims 3\n"
            "\n");
        printf("Miscellaneous options:\n"
            "  -f, --force             Force overwriting of files without confirmation\n"
            "  -h, --help              Show this help and exit\n"
            "\n"
            "Report bugs to <X-Fi6@phppoll.org>.\n"
            "farextract and libfar are maintained by the Niotso project.\n"
            "Home page: <http://www.niotso.org/>");
        return 0;
    }

    for(i=1; !infile[0] && i != argc-1; i++){
        /* Match for options */
        if(!profile){
            if(!strcmp(argv[i], "-ts1")){ profile = profile_ts1; continue; }
            if(!strcmp(argv[i], "-tso")){ profile = profile_tso; continue; }
            if(!strcmp(argv[i], "-sc4")){ profile = profile_sc4; continue; }
            if(!strcmp(argv[i], "-ts2")){ profile = profile_ts2; continue; }
            if(!strcmp(argv[i], "-spore")){ profile = profile_spore; continue; }
            if(!strcmp(argv[i], "-ts3")){ profile = profile_ts3; continue; }
        }
        if(!overwrite && (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--force"))){
            overwrite = 1;
            continue;
        }
        /* Not an option */
        if(!strcmp(argv[i], "-")){
            printf("%sReading from standard input is not yet implemented.", "farextract: error: ");
            return -1;
        }
        strcpy(infile, argv[i]);
        continue;
    }
    /* We're left with the out directory */
    if(!infile[0]){
        printf("%sReading from standard input is not yet implemented.", "farextract: error: ");
        return -1;
    }
    strcpy(outdirectory, argv[i]);

    /****
    ** Handle profile settings
    */
    if(!profile) profile = profile_tso;
    libfar_set_option(FAR_CONFIG_DEFAULT_TO_1A, (profile == profile_ts1));
    libfar_set_option(FAR_CONFIG_DBPF_COMPRESSED, (profile >= profile_sc4));
    libfar_set_option(FAR_CONFIG_REFPACK_HNSV, 0xFB);

    /****
    ** Open the file and read in the entire contents to memory
    */

    hFile = fopen(infile, "rb");
    if(hFile == NULL){
        printf("%sThe specified input file does not exist or could not be opened for reading.", "farextract: error: ");
        return -1;
    }
    fseek(hFile, 0, SEEK_END);
    ArchiveSize = ftell(hFile);
    if(ArchiveSize < 24){
        printf("%sNot a valid archive.", "farextract: error: ");
        return -1;
    }
    fseek(hFile, 0, SEEK_SET);

    ArchiveData = malloc(ArchiveSize);
    if(ArchiveData == NULL){
        printf("%sMemory for this archive could not be allocated.", "farextract: error: ");
        return -1;
    }
    if(!fread(ArchiveData, ArchiveSize, 1, hFile)){
        printf("%sThe input file could not be read.", "farextract: error: ");
        return -1;
    }
    fclose(hFile);

    /****
    ** Identify the type of archive
    */

    ArchiveType = far_identify(ArchiveData, ArchiveSize);
    if(ArchiveType == FAR_TYPE_INVALID){
        printf("%sNot a valid archive.", "farextract: error: ");
        return -1;
    }

    if(ArchiveType != FAR_TYPE_PERSIST){
        FAREntryNode * EntryNode;
        unsigned file = 0, filescount;

        /****
        ** Load header information
        */

        FARFile * FARFileInfo = far_create_archive(ArchiveType);
        if(FARFileInfo == NULL){
            printf("%sMemory for this archive could not be allocated.", "farextract: error: ");
            return -1;
        }
        if(!far_read_header(FARFileInfo, ArchiveData, ArchiveSize)){
            printf("%sNot a valid archive.", "farextract: error: ");
            return -1;
        }

        filescount = FARFileInfo->Files;
        printf("This archive contains %u files.\n\nExtracting\n", filescount);
        BeginningTime = clock();

        /****
        ** Load entry information
        */

        if(!far_enumerate_entries(FARFileInfo, ArchiveData+FARFileInfo->IndexOffset,
            ArchiveSize-FARFileInfo->IndexOffset, ArchiveSize)){
            printf("%sEntry data is corrupt.", "farextract: error: ");
            return -1;
        }

        /****
        ** Extract each entry
        */
        for(EntryNode = FARFileInfo->FirstEntry; EntryNode; EntryNode = EntryNode->NextEntry){
            char destination[256];

            file++;
            if(EntryNode->Entry.Filename)
                sprintf(destination, "%s/%s", outdirectory, EntryNode->Entry.Filename);
            else
                sprintf(destination, "%s/%08x-%08x-%08x.dat", outdirectory,
                    EntryNode->Entry.TypeID, EntryNode->Entry.GroupID, EntryNode->Entry.FileID);

            if(!far_read_entry_data(FARFileInfo, &(EntryNode->Entry), ArchiveData)){
                printf(" (%u/%u) Skipped (%s): %s\n", file, filescount,
                    "entry data is corrupt", EntryNode->Entry.Filename);
                continue;
            }
            /* Decompression, if any, was successful */

            if(!overwrite){
                hFile = fopen(destination, "rb");
                if(hFile != NULL){
                    /* File exists */
                    fclose(hFile);
                    printf(" (%u/%u) Skipped (%s): %s\n", file, filescount, "could not open", EntryNode->Entry.Filename);
                    if(EntryNode->Entry.DecompressedData != EntryNode->Entry.CompressedData)
                        libfar_free(EntryNode->Entry.DecompressedData);
                    continue;
                }
            }
            hFile = fopen(destination, "wb");
            if(hFile == NULL){
                printf(" (%u/%u) Skipped (%s): %s\n", file, filescount, "could not open", EntryNode->Entry.Filename);
                if(EntryNode->Entry.DecompressedData != EntryNode->Entry.CompressedData)
                    libfar_free(EntryNode->Entry.DecompressedData);
                continue;
            }

            if(EntryNode->Entry.Filename)
                printf(" (%u/%u) %s (%u bytes)\n", file, filescount,
                    EntryNode->Entry.Filename, EntryNode->Entry.DecompressedSize);
            else
                printf(" (%u/%u) %08x-%08x-%08x (%u bytes)\n", file, filescount,
                    EntryNode->Entry.TypeID, EntryNode->Entry.GroupID, EntryNode->Entry.FileID,
                    EntryNode->Entry.DecompressedSize);

            fwrite(EntryNode->Entry.DecompressedData, 1, EntryNode->Entry.DecompressedSize, hFile);
            fclose(hFile);

            if(EntryNode->Entry.DecompressedData != EntryNode->Entry.CompressedData)
                libfar_free(EntryNode->Entry.DecompressedData);
            extracted++;
        }
        printf("\nFinished extracting %u of %u files in %.2f seconds.", extracted, filescount,
            ((float) (clock() - BeginningTime))/CLOCKS_PER_SEC);
    }else{
        /* Persist file */
        PersistFile * PersistInfo;
        char destination[256];
        sprintf(destination, "%s/%s.out", outdirectory, infile);

        /****
        ** Load header information
        */

        PersistInfo = far_create_persist();
        if(PersistInfo == NULL){
            printf("%sMemory for this archive could not be allocated.", "farextract: error: ");
            return -1;
        }
        if(!far_read_persist_header(PersistInfo, ArchiveData, ArchiveSize)){
            printf("%sNot a valid archive.", "farextract: error: ");
            return -1;
        }

        /****
        ** Extract the data
        */
        printf("Extracting\n");
        BeginningTime = clock();
        if(!far_read_persist_data(PersistInfo, ArchiveData+18)){
            printf("%sNot a valid archive.", "farextract: error: ");
            return -1;
        }
        EndingTime = clock();

        if(!overwrite){
            hFile = fopen(destination, "rb");
            if(hFile != NULL){
                /* File exists */
                fclose(hFile);
                printf("%sFile exists.", "farextract: error: ");
                libfar_free(PersistInfo->DecompressedData);
                return -1;
            }
        }
        hFile = fopen(destination, "wb");
        if(hFile == NULL){
            printf("%sCould not open.", "farextract: error: ");
            libfar_free(PersistInfo->DecompressedData);
            return -1;
        }

        fwrite(PersistInfo->DecompressedData, 1, PersistInfo->DecompressedSize, hFile);
        fclose(hFile);
        printf("Extracted %u bytes in %.2f seconds.\n", PersistInfo->DecompressedSize,
            ((float) (EndingTime - BeginningTime))/CLOCKS_PER_SEC);
    }
    return 0;
}