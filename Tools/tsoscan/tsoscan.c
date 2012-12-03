/*
    tsoscan - IFF statistical webpage generator
    tsoscan.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
    Author(s): Ahmed El-Mahdawy <aa.mahdawy.10@gmail.com>
               Fatbag <X-Fi6@phppoll.org>

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
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <iff/iff.h>
#include "tsoscan.h"

static void print_usage(){
    printf("Usage: tsoscan [-f] [-o outfile] indir1 [indir2 [...]]\n"
    "Generate a statistical HTML page based on a number of IFF files.\n"
    "Use -f to force overwriting without confirmation.\n"
    "If outfile is unspecified, the output HTML page will be written to stats.html.\n"
    "\n"
    "Report bugs to <X-Fi6@phppoll.org>.\n"
    "tsoscan is maintained by the Niotso project.\n"
    "Home page: <http://www.niotso.org/>\n");
}

int main(int argc, char *argv[]){
    CommandLineArgs *CmdArgs;
    unsigned i, version, FileCount = 0;
    char **Files = NULL;
    IFFStats Stats;
    FILE *OutFile;

    if(!stats_create(&Stats)){
        fprintf(stderr, "%sUnable to allocate enough memory.\n", TSOSCAN_ERROR);
        return -1;
    }

    if(argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        print_usage();
        return 0;
    }

    CmdArgs = cmd_parse_args(argc, argv);
    if(CmdArgs == NULL || CmdArgs->InDirCount == 0){
        print_usage();
        return -1;
    }
    if(CmdArgs->OutFile == NULL){
        CmdArgs->OutFile = "stats.html";
    }

    /****
    ** List selected input directories
    */

    for(i=0; i<CmdArgs->InDirCount; i++){
        DIR *dir = opendir(CmdArgs->InDirs[i]);
        struct dirent *entry;
        unsigned DirStartIndex;

        if(dir == NULL){
            fprintf(stderr, "%sUnable to open the specified directory '%s'. Skipping.\n", TSOSCAN_WARNING, CmdArgs->InDirs[i]);
            continue;
        }

        DirStartIndex = FileCount;
        while((entry = readdir(dir)) != NULL){
            if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
                FileCount++;
        }
        rewinddir(dir);
        Files = realloc(Files, FileCount*sizeof(char**));
        if(Files == NULL){
            fprintf(stderr, "%sUnable to allocate enough memory.\n", TSOSCAN_ERROR);
            return -1;
        }

        for(; DirStartIndex<FileCount;){
            entry = readdir(dir);
            if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")){
                int dirlen = strlen(CmdArgs->InDirs[i]);
                int pathlen = strlen(entry->d_name);
                Files[DirStartIndex] = malloc(dirlen+pathlen+2);
                if(Files[DirStartIndex] == NULL){
                    fprintf(stderr, "%sUnable to allocate enough memory.\n", TSOSCAN_ERROR);
                    return -1;
                }

                memcpy(Files[DirStartIndex], CmdArgs->InDirs[i], dirlen);
                Files[DirStartIndex][dirlen] = PATH_SEP;
                memcpy(Files[DirStartIndex]+dirlen+1, entry->d_name, pathlen);
                Files[DirStartIndex][dirlen+pathlen+1] = '\0';

                DirStartIndex++;
            }
        }

        closedir(dir);
    }

    /****
    ** Load and parse IFF files
    */

    for(i=0; i<FileCount; i++){
        FILE *file;
        size_t FileSize;
        uint8_t *data;
        IFFFile iff;
        IFFChunk *ChunkData;
        unsigned ChunkIndex;

        printf("(%d/%d)\r", i+1, FileCount);

        file = fopen(Files[i], "rb");
        if(file == NULL){
            fprintf(stderr, "%sUnable to open the specified file '%s'. Skipping.\n", TSOSCAN_WARNING, Files[i]);
            continue;
        }
        fseek(file, 0, SEEK_END);
        FileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        data = malloc(FileSize);
        if(data == NULL){
            fprintf(stderr, "%sUnable to allocate memory for the specified files.\n", TSOSCAN_ERROR);
            return -1;
        }
        if(fread(data, 1, FileSize, file) != FileSize){
            fprintf(stderr, "%sUnable to read the specified file '%s'. Skipping.\n", TSOSCAN_WARNING, Files[i]);
            free(data);
            fclose(file);
            continue;
        }
        fclose(file);

        if(!iff_create(&iff)){
            fprintf(stderr, "%sUnable to allocate memory for the specified files.\n", TSOSCAN_ERROR);
            return -1;
        }
        if(!iff_read_header(&iff, data, FileSize) || !iff_enumerate_chunks(&iff, data+64, FileSize-64)){
            /* Skip non-IFF files silently */
            free(data);
            continue;
        }
        free(data);

        Stats.FileCount++;
        if(Stats.AverageChunkCount == -1){
            Stats.AverageChunkCount = iff.ChunkCount;
        }else{
            Stats.AverageChunkCount += iff.ChunkCount;
            Stats.AverageChunkCount /= 2;
        }

        for(ChunkIndex = 0, ChunkData = iff.Chunks; ChunkIndex < iff.ChunkCount; ChunkIndex++, ChunkData++){
            unsigned version = stats_get_version(ChunkData->Type, ChunkData->Data);
            if(!stats_version_increment(&Stats, ChunkData->Type, version)){
                fprintf(stderr, "%sUnable to allocate enough memory.\n", TSOSCAN_ERROR);
                return -1;
            }
        }
        iff_delete(&iff);
    }

    /****
    ** Write output file
    */

    if(!CmdArgs->ForceWrite){
        OutFile = fopen(CmdArgs->OutFile, "rb");
        if(OutFile != NULL){
            char c;
            fclose(OutFile);
            printf("File \"%s\" exists. Continue anyway? (y/n) ", CmdArgs->OutFile);
            c = getchar();
            if(c != 'y' && c != 'Y')
                return -1;
        }
    }
    OutFile = fopen(CmdArgs->OutFile, "wb");
    if(OutFile == NULL){
        fprintf(stderr, "%sThe output file '%s' could not be opened for writing.", TSOSCAN_ERROR, CmdArgs->OutFile);
        return -1;
    }

    fprintf(OutFile,
        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
    fprintf(OutFile, "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\" dir=\"ltr\">\n");
    fprintf(OutFile, "<head>\n");
    fprintf(OutFile, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n");
    fprintf(OutFile, "<meta http-equiv=\"Content-Style-Type\" content=\"text/css; charset=utf-8\" />\n");
    fprintf(OutFile, "<meta http-equiv=\"Content-Language\" content=\"en\" />\n");
    fprintf(OutFile, "<meta name=\"description\" content=\"tsostats\" />\n");
    fprintf(OutFile, "<meta name=\"generator\" content=\"IFF Chunk Statistics (tsostats)\" />\n");
    fprintf(OutFile, "<title>IFF Chunk Statistics (tsostats)</title>\n");
    fprintf(OutFile, "<style type=\"text/css\" media=\"all\">\n");
    fprintf(OutFile, "html, body {\n");
    fprintf(OutFile, "    background: #fff;\n");
    fprintf(OutFile, "    color: #000;\n");
    fprintf(OutFile, "    font-family: sans-serif;\n");
    fprintf(OutFile, "}\n");
    fprintf(OutFile, "\n");
    fprintf(OutFile, "a:link, a:visited, a:hover, a:active { color: #00f; }\n");
    fprintf(OutFile, "a:link, a:visited { text-decoration: none; }\n");
    fprintf(OutFile, "a:hover, a:active { text-decoration: underline; }\n");
    fprintf(OutFile, "\n");
    fprintf(OutFile, "#attributes {\n");
    fprintf(OutFile, "    border-left: 2px solid #888; padding-left: 4px; margin-bottom: 1em;\n");
    fprintf(OutFile, "}\n");
    fprintf(OutFile, "\n");
    fprintf(OutFile, "#toc {\n");
    fprintf(OutFile, "    display: table-cell;\n");
    fprintf(OutFile, "    margin-top: 1em;\n");
    fprintf(OutFile, "    background: #eee; border: 1px solid #bbb;\n");
    fprintf(OutFile, "    padding: .25em;\n");
    fprintf(OutFile, "}\n");
    fprintf(OutFile, "#toc div {\n");
    fprintf(OutFile, "    border-bottom: 1px solid #aaa;\n");
    fprintf(OutFile, "}\n");
    fprintf(OutFile, "#toc ul {\n");
    fprintf(OutFile, "    list-style-type: none;\n");
    fprintf(OutFile, "    padding: 0;\n");
    fprintf(OutFile, "}\n");
    fprintf(OutFile, "ul ul {\n");
    fprintf(OutFile, "    padding: 2em;\n");
    fprintf(OutFile, "}\n");
    fprintf(OutFile, "\n");
    fprintf(OutFile, "h2 {\n");
    fprintf(OutFile, "    border-bottom: 1px solid #888;\n");
    fprintf(OutFile, "    margin: 2em 0 0.25em 0;\n");
    fprintf(OutFile, "}\n");
    fprintf(OutFile, "h2 a {\n");
    fprintf(OutFile, "    font-size: 9pt;\n");
    fprintf(OutFile, "}\n");
    fprintf(OutFile, "\n");
    fprintf(OutFile, "table {\n");
    fprintf(OutFile, "    border: 1px #aaa solid;\n");
    fprintf(OutFile, "    border-collapse: collapse;\n");
    fprintf(OutFile, "}\n");
    fprintf(OutFile, "th, td {\n");
    fprintf(OutFile, "    border: 1px #aaa solid;\n");
    fprintf(OutFile, "    padding: 0.2em;\n");
    fprintf(OutFile, "    white-space: pre-wrap;\n");
    fprintf(OutFile, "}\n");
    fprintf(OutFile, "\n");
    fprintf(OutFile, ".center {\n");
    fprintf(OutFile, "    margin: auto auto;\n");
    fprintf(OutFile, "}\n");
    fprintf(OutFile, "\n");
    fprintf(OutFile, "#footer {\n");
    fprintf(OutFile, "    margin-top: 2em;\n");
    fprintf(OutFile, "    padding-bottom: 0.5em;\n");
    fprintf(OutFile, "    text-align: center;\n");
    fprintf(OutFile, "}\n");
    fprintf(OutFile, "</style>\n");
    fprintf(OutFile, "</head>\n");
    fprintf(OutFile, "<body>\n");
    fprintf(OutFile, "<h1>IFF Chunk Statistics (tsostats)</h1>\n");
    fprintf(OutFile, "<div id=\"attributes\">\n");
    fprintf(OutFile, "<table>\n");
    fprintf(OutFile, "<tr><td>Number of IFF files:</td><td>%u</td></tr>\n", Stats.FileCount);
    fprintf(OutFile, "<tr><td>Average chunk count:</td><td>%.1f</td></tr>\n", Stats.AverageChunkCount);
    fprintf(OutFile, "</table>\n");
    fprintf(OutFile, "</div>\n");

    fprintf(OutFile, "<div id=\"toc\"><div><b>Contents</b> &ndash; %u chunk types</div>\n", Stats.ChunkTypeCount);
    fprintf(OutFile, "<ul>\n");
    for(i=0; i<Stats.ChunkTypeCount; i++)
        fprintf(OutFile, "<li><a href=\"#type%u\">%u %s</a></li>\n", i, i+1, Stats.ChunkTypes[i].Type);
    fprintf(OutFile, "</ul>\n");
    fprintf(OutFile, "</div>\n");
    fprintf(OutFile, "\n");

    for(i=0; i<Stats.ChunkTypeCount; i++){
        ChunkStats *chunk = Stats.ChunkTypes+i;

        fprintf(OutFile, "<h2 id=\"type%u\">%u %s <a href=\"#type%u\">(Jump)</a></h2>\n", i, i+1, Stats.ChunkTypes[i].Type, i);
        fprintf(OutFile, "<div>\n");

        fprintf(OutFile, "<table>\n");
        fprintf(OutFile, "<tr><td>Number of occurrences:</td><td>%u</td></tr>\n", chunk->ChunkCount);
        if(chunk->VersionCount == 1 && chunk->Versions[0].Version == (unsigned)-1)
            fprintf(OutFile, "<tr><td>Number of versions:</td><td>N/A</td></tr>\n");
        else
            fprintf(OutFile, "<tr><td>Number of versions:</td><td>%u</td></tr>\n", chunk->VersionCount);
        fprintf(OutFile, "</table>\n");

        if(chunk->VersionCount > 1 ||
          (chunk->VersionCount == 1 && chunk->Versions[0].Version != (unsigned)-1)){
            fprintf(OutFile, "<table class=\"center\">\n");
            fprintf(OutFile, "<tr><th></th><th>Version</th><th>Count</th></tr>\n");
            for(version=0; version<chunk->VersionCount; version++){
                VersionInfo *verinfo = chunk->Versions+version;
                float percentage = (float)verinfo->Count / chunk->ChunkCount * 100;

                fprintf(OutFile, "<tr><td>%u</td><td>%u (<tt>0x%x</tt>)</td><td>%u (%.1f%%)</td></tr>\n",
                    version+1, verinfo->Version, verinfo->Version, verinfo->Count, percentage);
            }
            fprintf(OutFile, "</table>\n");
        }

        fprintf(OutFile, "</div>\n\n");
    }

    fprintf(OutFile,
        "<div id=\"footer\">This page was generated by the use of <a href=\"http://www.niotso.org/\">tsostats</a>.</div>\n");
    fprintf(OutFile, "</body>\n");
    fprintf(OutFile, "</html>");
    fclose(OutFile);

    printf("Generated statistics based on %u IFF files.\n", Stats.FileCount);
    cmd_delete(CmdArgs);
    stats_delete(&Stats);
    return 0;
}