/*
    hitutils - The Sims HIT (dis)assembler and linker
    hitasm.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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
#include <errno.h>
#define HITASM_HEADERS
#include "hitutils.h"

static uint8_t ObjectHeader[] = {
    0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00,
    0x06, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x07, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x19, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00
};

enum {
    txt, out, filecount
};

typedef struct {
    uint8_t * Data;
    size_t Position;
    size_t Size;
    char * Name;
} ByteWriterContext;

static void bw_expand(ByteWriterContext *bwc){
    void * ptr;
    if(bwc->Size > SIZE_MAX/2 || !(ptr = realloc(bwc->Data, bwc->Size<<=1)))
        Shutdown_M("%sCould not allocate memory for %s section.\n", "hitasm: Error: ", bwc->Name);
    bwc->Data = ptr;
}

static void bw_write32(ByteWriterContext *bwc, uint32_t value){
    if(bwc->Size-bwc->Position < 4)
        bw_expand(bwc);
    write_uint32(bwc->Data+bwc->Position, value);
    bwc->Position += 4;
}

static void bw_write8(ByteWriterContext *bwc, uint8_t value){
    if(bwc->Size-bwc->Position < 1)
        bw_expand(bwc);
    bwc->Data[bwc->Position] = value;
    bwc->Position += 1;
}

static void bw_write_memory(ByteWriterContext *bwc, const uint8_t *ptr, size_t size){
    while(bwc->Size-bwc->Position < size)
        bw_expand(bwc);
    do bwc->Data[bwc->Position++] = *ptr++;
    while(--size);
}

static void bw_write_string(ByteWriterContext *bwc, const char *string){
    do bw_write8(bwc, *string);
    while(*string++);
}

typedef struct {
    char * Token;
    size_t Line, NextLine;
    size_t Col, NextCol;
    int GaveLastToken;
    ByteReaderContext brc;
} ParserContext;

enum {
    TK_CROSSLINES = 1,
    CN_LABELONLY = 1
};

enum Sections {
    Text, SymbolTable, StringTable, RelocationTable, SectionCount
};

static FILE *hFile = NULL;
static char *path[filecount] = {NULL};
static uint8_t *data[filecount] = {NULL};
static ByteWriterContext Section[] = {
    {0,0,1024,"text"},
    {0,0,1024,"symtab"},
    {0,0,1024,"strtab"},
    {0,0,1024,".rel.text"}
};

static uint8_t * add_symbol(const char * Name){
    bw_write32(&Section[SymbolTable], Section[StringTable].Position);
    bw_write32(&Section[SymbolTable], 0);
    bw_write32(&Section[SymbolTable], 0);
    bw_write32(&Section[SymbolTable], 18);
    bw_write_string(&Section[StringTable], Name);
    return Section[SymbolTable].Data + Section[SymbolTable].Position - 16;
}

static uint8_t * find_symbol_by_name(const char * Name, uint32_t * SymbolIndex){
    uint32_t p;
    for(p=48; p<Section[SymbolTable].Position; p+=16){
        if(!strcmp((char*) Section[StringTable].Data + read_uint32(Section[SymbolTable].Data + p), Name)){
            if(SymbolIndex) *SymbolIndex = p>>4;
            return Section[SymbolTable].Data + p;
        }
    }

    if(SymbolIndex) *SymbolIndex = p>>4;
    return NULL;
}

static __inline int whitespace(const uint8_t *Data){
    return (Data[0] == ' ' || Data[0] == '\t' || Data[0] == '\r' || Data[0] == '\n' || Data[0] == '\0');
}

static __inline int comment(const uint8_t *Data){
    return (Data[0] == ';' || (Data[0] == '#' && whitespace(Data+1)));
}

static int parser_next_token(ParserContext *pc, int CrossLines){
    if(!pc->brc.Size) return 0;

    if(pc->GaveLastToken){
        pc->GaveLastToken = 0;
        /* find the start of the next token */
        while(1){
            if(whitespace(pc->brc.Data)){
                if(pc->brc.Data[0] == '\n'){
                    pc->NextLine++; pc->NextCol = 1;
                }else if(pc->brc.Data[0] == '\t'){
                    pc->NextCol += 4 - ((pc->NextCol-1)%4);
                }else{
                    pc->NextCol++;
                }
            }else if(comment(pc->brc.Data)){
                /* skip to the end of the line */
                pc->NextLine++; pc->NextCol = 1;
                do {
                    if(!--pc->brc.Size) return 0;
                    pc->brc.Data++;
                } while(pc->brc.Data[0] != '\n');
            }else break;

            if(!--pc->brc.Size) return 0;
            pc->brc.Data++;
        }
    }
    if(!CrossLines && pc->Line != pc->NextLine) return 0;
    pc->Token = (char*)pc->brc.Data;
    pc->Line = pc->NextLine;
    pc->Col = pc->NextCol++;
    pc->GaveLastToken = 1;

    /* mark the end of this token with a null character */
    while(1){
        if(!--pc->brc.Size) return 1;
        pc->brc.Data++;

        if(whitespace(pc->brc.Data)){
            if(pc->brc.Data[0] == '\n'){
                pc->NextLine++; pc->NextCol = 1;
            }else if(pc->brc.Data[0] == '\t'){
                pc->NextCol += 4 - ((pc->NextCol-1)%4);
            }else{
                pc->NextCol++;
            }
            break;
        }else if(comment(pc->brc.Data)){
            /* skip to the end of the line */
            pc->NextLine++; pc->NextCol = 1;
            do {
                if(!--pc->brc.Size) return 1;
                pc->brc.Data++;
            } while(pc->brc.Data[0] != '\n');
            break;
        }else pc->NextCol++;
    }
    pc->brc.Size--;
    pc->brc.Data[0] = '\0';
    pc->brc.Data++;

    return 1;
}

static __inline void verify_eol(ParserContext *pc){
    if(parser_next_token(pc, 0))
        Shutdown_M("%s:%u:%u: error: expected newline before '%s'\n",
            path[txt], pc->Line, pc->Col, pc->Token);
}

static void verify_name(const ParserContext *pc, const char *thistoken, const char *nexttoken){
    const char *Name;
    for(Name = pc->Token; *Name; Name++)
        if((Name == pc->Token || *Name < '0' || *Name > '9')
            && (*Name < 'A' || *Name > 'Z') && (*Name < '_' || *Name > '_') && (*Name < 'a' || *Name > 'z'))
            Shutdown_M("%s:%u:%u: error: expected %s before '%c'\n",
                path[txt], pc->Line, pc->Col, (Name == pc->Token) ? thistoken : nexttoken, *Name);
}

static __inline void parser_add_symbol(const ParserContext *pc){
    uint8_t * Symbol;
    verify_name(pc, "label or integer constant", "newline");

    /* we're using the ELF st_size field (offset 8) to hold the line number temporarily */
    Symbol = find_symbol_by_name(pc->Token, NULL);
    if(Symbol){
        if(read_uint32(Symbol+4))
            Shutdown_M("%s:%u:%u: error: label '%s' redefined (previous definition at %s:%u:1)\n",
                path[txt], pc->Line, pc->Col, pc->Token, path[txt], read_uint32(Symbol+8));
    }else Symbol = add_symbol(pc->Token);

    write_uint32(Symbol+4, Section[Text].Position);
    write_uint32(Symbol+8, pc->Line);
}

static __inline void parser_add_reference(const ParserContext *pc){
    uint32_t SymbolIndex;
    verify_name(pc, "label", "operand or newline");

    if(!find_symbol_by_name(pc->Token, &SymbolIndex))
        add_symbol(pc->Token);

    bw_write32(&Section[RelocationTable], Section[Text].Position);
    bw_write32(&Section[RelocationTable], (SymbolIndex<<8)|0x02);
}

static uint32_t read_integer(const ParserContext *pc, uint32_t maxval){
    unsigned long value;
    char * endptr;

    if(pc->Token[0] == '-')
        Shutdown_M("%s:%u:%u: error: integer constant '%s' may not be negative\n",
            path[txt], pc->Line, pc->Col, pc->Token);
    if(pc->Token[0] < '0' || pc->Token[0] > '9')
        Shutdown_M("%s:%u:%u: error: expected integer constant before '%s'\n",
            path[txt], pc->Line, pc->Col, pc->Token);
    if(pc->Token[0] == '0' && (pc->Token[1] == 'x' || pc->Token[1] == '0' || pc->Token[1] == 'o'))
        Shutdown_M("%s:%u:%u: error: illegal %s prefix '%s%c' on integer constant '%s'\n",
            path[txt], pc->Line, pc->Col, pc->Token[1] == 'x' ? "hexadecimal" : "octal",
            pc->Token[1] != '0' ? "0" : "", pc->Token[1], pc->Token);

    value = strtoul(pc->Token, &endptr, 10);
    if(*endptr == '.')
        Shutdown_M("%s:%u:%u: error: illegal floating point constant '%s'\n",
            path[txt], pc->Line, pc->Col, endptr, pc->Token);
    if(*endptr != '\0')
        Shutdown_M("%s:%u:%u: error: illegal suffix '%s' on integer constant '%s'\n",
            path[txt], pc->Line, pc->Col, endptr, pc->Token);
    if(value > (unsigned long) maxval || errno == ERANGE)
        Shutdown_M("%s:%u:%u: error: integer constant '%s' is out of range\n",
            path[txt], pc->Line, pc->Col, pc->Token);

    return (uint32_t)value;
}

static uint32_t read_constant(ParserContext *pc, int Type, uint32_t maxval){
    unsigned i;
    if(pc->Token[0] == '#') /* note that the tokens "#" and "" will never be returned by the parser */
        pc->Token++;

    if((pc->Token[0] >= '0' && pc->Token[0] <= '9') || pc->Token[0] == '-' || pc->Token[0] == '.'){
        if(Type == CN_LABELONLY)
            Shutdown_M("%s:%u:%u: error: explicit address '%s' is forbidden\n",
                path[txt], pc->Line, pc->Col, pc->Token);
        return read_integer(pc, maxval);
    }

    verify_name(pc, "constant or label", "operand or newline");
    for(i=0; i<ConstantCount; i++){
        if(!strcmp(Constants[i].Name, pc->Token)){
            if(Constants[i].Value > maxval)
                Shutdown_M("%s:%u:%u: error: integer constant '%s' (%u) is out of range\n",
                    path[txt], pc->Line, pc->Col, pc->Token, Constants[i].Value);
            return Constants[i].Value;
        }
    }

    if(maxval != 0xFFFFFFFF)
        Shutdown_M("%s:%u:%u: error: address referenced by label '%s' is out of range\n",
            path[txt], pc->Line, pc->Col, pc->Token);
    parser_add_reference(pc);
    return 0;
}

static __inline uint8_t read_instruction(const ParserContext *pc, uint32_t *operands){
    uint8_t i;
    verify_name(pc, "instruction", "operand or newline");

    for(i=0; i<InstructionCount; i++){
        if(!strcmp(pc->Token, Instructions[i].Name)){
            *operands = Instructions[i].Operands;
            return i+1;
        }
    }

    return 0;
}

static __inline uint8_t read_variable(const ParserContext *pc, const variable_t *Variables, size_t VariableCount){
    unsigned i;
    verify_name(pc, "variable", "operand or newline");

    for(i=0; i<VariableCount; i++)
        if(!strcmp(Variables[i].Name, pc->Token))
            return Variables[i].Value;

    Shutdown_M("%s:%u:%u: error: unrecognized variable '%s'\n",
        path[txt], pc->Line, pc->Col, pc->Token);
    return 0;
}

static void Shutdown(){
    unsigned i;
    for(i=0; i<filecount; i++){
        free(path[i]);
        free(data[i]);
    }
    for(i=0; i<SectionCount; i++)
        free(Section[i].Data);
    if(hFile)
        fclose(hFile);
}

int main(int argc, char *argv[]){
    unsigned i;
    int SimsVersion = 0;
    int overwrite = 0;
    size_t filesize[filecount-1];
    unsigned slash;
    const variable_t * Variables;
    size_t VariableCount;
    ParserContext pc;
    int InBinary = 0;
    uint8_t * SectionHeader;
    size_t SectionOffset;

    if(argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        printf("Usage: hitasm [-ts1|-tso] [-f] [-o outfile.o] infile.txt\n"
            "Assemble a HIT source file to an intermediary object file\n"
            "which can be linked using hitld.\n"
            "\n"
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
            if(!strcmp(argv[i], "-o")) path[out] = argv[++i];
            else break;
        }
        else break;
    }
    path[txt] = argv[i];
    for(i=0; i<filecount; i++)
        if(path[i])
            path[i] = strdup(path[i]); /* necessary for free(path[i]) in Shutdown_M */

    if(!SimsVersion)
        Shutdown_M("%sSims version not specified. (Use -ts1 or -tso.)\n", "hitasm: Error: ");

    if(SimsVersion == VERSION_TS1){
        Variables = TS1Variables;
        VariableCount = TS1VariableCount;
    }else{
        Variables = TSOVariables;
        VariableCount = TSOVariableCount;
    }

    if(path[out] == NULL){
        int length = strlen(path[txt]);
        path[out] = malloc(max(length+1, 3));
        strcpy(path[out], path[txt]);
        strcpy(path[out] + max(length-4, 0), ".o");
    }

    /****
    ** Read all of the requested files
    */

    for(i=0; i<filecount-1; i++){
        size_t bytestransferred;

        hFile = fopen(path[i], "rb");
        if(hFile == NULL)
            Shutdown_M("%sCould not open file: %s.\n", "hitasm: Error: ", path[i]);

        fseek(hFile, 0, SEEK_END);
        filesize[i] = ftell(hFile);
        if(filesize[i] == 0 || filesize[i] == SIZE_MAX)
            Shutdown_M("%sFile is invalid: %s.\n", "hitasm: Error: ", path[i]);

        data[i] = malloc(filesize[i]+1);
        if(data[i] == NULL)
            Shutdown_M("%sCould not allocate memory for file: %s.\n", "hitasm: Error: ", path[i]);

        fseek(hFile, 0, SEEK_SET);
        bytestransferred = fread(data[i], 1, filesize[i], hFile);
        fclose(hFile); hFile = NULL;
        if(bytestransferred != filesize[i])
            Shutdown_M("%sCould not read file: %s.\n", "hitasm: Error: ", path[i]);
        data[i][filesize[i]++] = '\0'; /* add a null character to the end of the file */
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
            fprintf(stderr, "%sFile \"%s\" exists.\nContinue anyway? (y/n) ", "hitasm: ", path[out]);
            c = getchar();
            if(c != 'y' && c != 'Y')
                Shutdown_M("\nAborted.\n");
        }
    }
    hFile = fopen(path[out], "wb");
    if(hFile == NULL)
        Shutdown_M("%sCould not open file: %s.\n", "hitasm: Error: ", path[out]);

    /****
    ** Perform the assembly
    */

    for(i=0; i<SectionCount; i++){
        Section[i].Data = malloc(Section[Text].Size);
        if(!Section[i].Data)
            Shutdown_M("%sCould not allocate memory for %s section.\n", "hitasm: Error: ", Section[i].Name);
    }
    pc.NextLine = 1;
    pc.NextCol = 1;
    pc.GaveLastToken = 1;
    pc.brc.Data = data[txt];
    pc.brc.Size = filesize[txt];

    bw_write_memory(&Section[SymbolTable], SymbolTableHeader, sizeof(SymbolTableHeader));

    for(i=slash=0; path[txt][i]; i++)
        if(path[txt][i] == '/' || path[txt][i] == '\\') slash = i+1;
    bw_write8(&Section[StringTable], '\0');
    bw_write_string(&Section[StringTable], path[txt] + slash);

    while(parser_next_token(&pc, TK_CROSSLINES)){
        /* Unimplemented commands */
        if(!strcmp(pc.Token, "BASEID_TRACKDATA") || !strcmp(pc.Token, "INCLUDE")
            || !strcmp(pc.Token, "include") || !strcmp(pc.Token, "INIFILE")
            || !strcmp(pc.Token, "LIST") || !strcmp(pc.Token, "SYMBOLFILE")){
            if(InBinary)
                Shutdown_M("%s:%u:%u: error: invalid use of '%s' inside BINARY section\n",
                    path[txt], pc.Line, pc.Col, pc.Token);
            while(parser_next_token(&pc, 0)); /* skip to the end of the line */
            continue;
        }

        if(!strcmp(pc.Token, "BINARY") && !InBinary){
            InBinary++;
            if(!parser_next_token(&pc, TK_CROSSLINES) || strcmp(pc.Token, "["))
                Shutdown_M("%s:%u:%u: error: expected '[' for beginning of BINARY section before %s%s%s\n",
                    path[txt], pc.Line, pc.Col,
                    pc.Token ? "'" : "", pc.Token ? pc.Token : "end of file", pc.Token ? "'" : "");
        }

        else if(!strcmp(pc.Token, "]") && InBinary)
            InBinary--;

        else if(!InBinary)
            Shutdown_M("%s:%u:%u: error: '%s' is not a valid command\n",
                path[txt], pc.Line, pc.Col, pc.Token);

        /****
        ** Inside a BINARY section
        */

        else if(pc.Col == 1) /* no indent */
            parser_add_symbol(&pc);

        else{ /* indent */
            uint8_t opcode;
            uint32_t operands;

            if((pc.Token[0] >= '0' && pc.Token[0] <= '9') || pc.Token[0] == '-' || pc.Token[0] == '.'
                || pc.Token[0] == '#' || !(opcode = read_instruction(&pc, &operands))){
                /* declare bytes (db and dd pseudo-instructions) */
                do {
                    if(pc.Token[0] != '#')
                        bw_write8(&Section[Text], read_integer(&pc, 0x000000FF));
                    else
                        bw_write32(&Section[Text], read_constant(&pc, 0, 0xFFFFFFFF));
                } while(parser_next_token(&pc, 0));
                continue;
            }else{
                const char * InstructionName = pc.Token;
                bw_write8(&Section[Text], opcode);

                for(i=0; (operands >>= 4) != 0; i++){
                    int type = operands & 15;
                    const char *position[] = {"one","two","three","four"};

                    if(!parser_next_token(&pc, 0)){
                        int j;
                        for(j=i+1; (operands >>= 4) != 0; j++);
                        Shutdown_M("%s:%u:%u: error: instruction '%s' wants %s operands but only %s supplied\n",
                            path[txt], pc.Line, pc.Col, pc.Token, InstructionName, position[j], position[i]);
                    }

                    if(type == o_byte)
                        bw_write8(&Section[Text], read_constant(&pc, 0, 0x000000FF));
                    else if(type == o_dword)
                        bw_write32(&Section[Text], read_constant(&pc, 0, 0xFFFFFFFF));
                    else if(type == o_address)
                        bw_write32(&Section[Text], read_constant(&pc, CN_LABELONLY, 0xFFFFFFFF));
                    else if(type == o_variable)
                        bw_write8(&Section[Text], read_variable(&pc, Variables, VariableCount));
                    else if(type == o_jump){
                        /* TODO: Change this */
                        bw_write32(&Section[Text], read_constant(&pc, CN_LABELONLY, 0xFFFFFFFF));
                    }
                }
            }
        }

        verify_eol(&pc);
    }

    if(InBinary)
        Shutdown_M("%s:%u:%u: error: expected ']' for end of BINARY section before end of file\n",
            path[txt], pc.Line, pc.Col);

    /****
    ** Prepare and write out the ELF object header and all sections
    */

    for(i=48+8; i<Section[SymbolTable].Position; i+=16)
        write_uint32(Section[SymbolTable].Data + i, 0); /* clear the st_size field we used temporarily */

    if(SimsVersion == VERSION_TSO)
        ObjectHeader[36]++; /* set the lsb of the processor flags to indicate that this code is for TSO */

    for(i = 0, SectionHeader = ObjectHeader + 120, SectionOffset = 304; i < SectionCount; i++){
        write_uint32(SectionHeader + 0, SectionOffset);
        write_uint32(SectionHeader + 4, Section[i].Position);
        SectionHeader += 40;
        SectionOffset += Section[i].Position;

        if(i == 0){
            write_uint32(SectionHeader + 0, SectionOffset);
            write_uint32(SectionHeader + 4, sizeof(SHStringTable));
            SectionHeader += 40;
            SectionOffset += sizeof(SHStringTable);
        }
    }

    fwrite(ObjectHeader, 1, sizeof(ObjectHeader), hFile);
    fwrite(Section[Text].Data, 1, Section[Text].Position, hFile);
    fwrite(SHStringTable, 1, sizeof(SHStringTable), hFile);
    for(i=1; i<SectionCount; i++)
        fwrite(Section[i].Data, 1, Section[i].Position, hFile);

    Shutdown();

    return 0;
}