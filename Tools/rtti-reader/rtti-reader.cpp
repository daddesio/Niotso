/*
    rtti-reader - The Sims Online MSVC RTTI Class Hierarchy Extractor
    rtti-reader.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

//For information about MSVC RTTI, read:
//<http://www.openrce.org/articles/full_view/23>
//<https://www.blackhat.com/presentations/bh-dc-07/Sabanal_Yason/Paper/bh-dc-07-Sabanal_Yason-WP.pdf>

//For information about the Windows PE header, read:
//<https://en.wikibooks.org/wiki/X86_Disassembly/Windows_Executable_Files#File_Format>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#ifndef read_int32
 #define read_uint32(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)) | ((x)[3]<<(8*3)))
 #define read_uint16(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)))
#endif
#ifndef write_int32
 #define write_uint32(dest, src) do { \
    (dest)[0] = ((src)&0x000000FF)>>(8*0); \
    (dest)[1] = ((src)&0x0000FF00)>>(8*1); \
    (dest)[2] = ((src)&0x00FF0000)>>(8*2); \
    (dest)[3] = ((src)&0xFF000000)>>(8*3); \
    } while(0)
#endif

static void Shutdown_M(const char * Message);

struct Segment {
    size_t size, offset;
    Segment() : size(0) {}
};

struct ByteReaderContext {
    size_t start, position, end;
    bool seek(size_t pos){
        if(pos > end)
            return false;
        position = pos;
        return true;
    }
    bool skip(int pos = 1){
        if(position + pos > end)
            return false;
        position += pos;
        return true;
    }
};

struct PaddingTest {
    uint32_t A;
    uint32_t B;
};

template <class T>
struct RTTIVector {
    size_t Count, SizeAllocated;
    T * Buffer;

    void init(){
        Count = 0, SizeAllocated = sizeof(T);
        if(!(Buffer = (T*) malloc(sizeof(T))))
            Shutdown_M("Failed to allocate memory");
    }
    T& add(){
        if((Count+1)*sizeof(T) > SizeAllocated){
            void * ptr;
            if(SizeAllocated > SIZE_MAX/2 || !(ptr = (T*) realloc(Buffer, SizeAllocated<<=1)))
                Shutdown_M("Failed to allocate memory");
            Buffer = (T *) ptr;
        }

        return Buffer[Count++];
    }
};

struct RTTITypeDescriptor {
    struct {
        uint32_t Address;
        uint32_t VTableAddress;
        uint32_t Reserved;
    } Fields;
    char * Name;
    char * UnmangledName;
};

struct RTTIBaseClassDescriptor {
    struct {
        uint32_t Address;
        uint32_t TypeDescriptorAddress;
        uint32_t BaseClassCount;
        uint32_t MemberOffset;
        uint32_t COLAddressOffset;
        uint32_t VTableOffset;
        uint32_t Attributes;
    } Fields;
    RTTITypeDescriptor TD;
};

struct RTTIClassHierarchyDescriptor {
    struct {
        uint32_t Address;
        uint32_t Reserved;
        uint32_t Attributes;
        uint32_t BaseClassCount;
        uint32_t BaseClassListAddress;
    } Fields;
    RTTIVector<RTTIBaseClassDescriptor> BCDL;
};

struct RTTICompleteObjectLocator {
    struct {
        uint32_t Address;
        uint32_t Reserved;
        uint32_t Offset;
        uint32_t CDOffset;
        uint32_t TypeDescriptorAddress;
        uint32_t ClassDescriptorAddress;
    } Fields;
    uint32_t VTableAddress;
};

struct RTTIClass {
    RTTIVector<RTTICompleteObjectLocator> COLL;
    RTTITypeDescriptor TD;
    RTTIClassHierarchyDescriptor CHD;
    void init(){
        COLL.init();
        CHD.BCDL.init();
    }
    bool DependsOn(const RTTIClass& X) const {
        for(uint32_t i=1; i<CHD.BCDL.Count; i++)
            if(CHD.BCDL.Buffer[i].TD.Fields.Address == X.TD.Fields.Address)
                return true;
        return false;
    }
    static int Compare(const void * Aptr, const void * Bptr){
        const RTTIClass& A = *reinterpret_cast<const RTTIClass*>(Aptr);
        const RTTIClass& B = *reinterpret_cast<const RTTIClass*>(Bptr);

        if(A.DependsOn(B)) return 1;  //If A depends on B, A > B
        if(B.DependsOn(A)) return -1; //If B depends on A, B > A
        return strcmp(A.TD.UnmangledName, B.TD.UnmangledName);
    }
};

struct PEFile {
    static PEFile * ptr;
    FILE * hFile;
    uint8_t * Data;
    Segment rdata, data;
    ByteReaderContext brc;

    PEFile(const char * filename) : Data(NULL) {
        PEFile::ptr = this;

        hFile = fopen(filename, "rb");
        if(!hFile)
            Shutdown_M("The specified input file does not exist or could not be opened for reading");

        fseek(hFile, 0, SEEK_END);
        size_t FileSize = ftell(hFile);
        if(FileSize < 64)
            Shutdown_M("Not a valid Windows PE file");
        fseek(hFile, 0, SEEK_SET);

        Data = (uint8_t*) malloc(FileSize);
        if(!Data)
            Shutdown_M("Failed to allocate memory");
        if(fread(Data, 1, FileSize, hFile) != FileSize)
            Shutdown_M("Failed to read input file");

        fclose(hFile);

        brc.start = brc.position = 0;
        brc.end = FileSize;
    }
    ~PEFile(){
        if(hFile)
            fclose(hFile);
        free(Data);
    }

    inline bool seek(size_t pos, int offset = 0){
        return brc.seek(pos + offset);
    }

    inline bool skip(size_t pos = 1, int offset = 0){
        return brc.skip(pos + offset);
    }
    int nextchar(){
        if(!brc.skip())
            return EOF;
        return Data[brc.position-1];
    }
    void lookat(Segment& segment){
        brc.start = brc.position = segment.offset;
        brc.end = segment.offset + segment.size;
    }

    uint32_t read32(){
        return brc.skip(4) ? read_uint32(Data+brc.position-4) : -1;
    }
    uint16_t read16(){
        return brc.skip(2) ? read_uint16(Data+brc.position-2) : -1;
    }
    size_t strlen(){
        size_t i = (size_t)-1;
        int byte;
        do {
            byte = nextchar();
            if(byte == EOF)
                return -1;
            i++;
        } while(byte);
        skip(-(int)i-1); //Seek back
        return i;
    }
    bool strcpy(char * dest){
        int i = 0;
        do {
            int byte = nextchar();
            if(byte == EOF)
                return false;
            *dest = (char) byte;
            i--;
        } while(*dest++);
        skip(i); //Seek back
        return true;
    }
    int strcmp(const char * data){
        int i = 0;
        int byte;
        do {
            byte = nextchar();
            if(byte == EOF)
                return -1;
            i--;
        } while(*data++ && (char)byte == *(data-1));
        skip(i); //Seek back
        return byte - *(data-1);
    }
    enum { Parse_QuestionMark = 1};
    bool memfind(const char * data, size_t size, int MemParse = 0){
        size_t i = 0;
        do {
            int byte = nextchar();
            if(byte == EOF)
                return false;
            else if((char)byte != data[i] && (!MemParse || data[i] != '?')){
                skip(-(int)i);
                i = 0;
            } else i++;
        } while(i<size);
        skip(-(int)i); //Seek back
        return true;
    }

    bool find32(uint32_t address){
        char buffer[4];
        write_uint32(buffer, address);
        return memfind(buffer, 4);
    }

    bool GenericFill(uint8_t *ptr, size_t count){
        const size_t padding = offsetof(PaddingTest,B)-offsetof(PaddingTest,A);
        count -= padding;
        if(count > brc.end - brc.position)
            return false;

        uint32_t *field = reinterpret_cast<uint32_t*>(ptr);
        *field = brc.position; //The Address field always comes first

        do {
            ptr += padding; count -= padding;
            field = reinterpret_cast<uint32_t*>(ptr);
            *field = read32();
        } while(count);

        return true;
    }

    template <class T>
    inline bool Fill(T& context) {
        return GenericFill(reinterpret_cast<uint8_t*>(&context.Fields), sizeof(context.Fields));
    }
};

PEFile * PEFile::ptr;

static void Shutdown_M(const char * Message){
    fprintf(stderr, "rtti-reader: error: %s.\n", Message);
    if(PEFile::ptr)
        PEFile::ptr->~PEFile();
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){
    unsigned i;
    const char * InFile, * BaseName;

    if(argc != 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        printf("Usage: rtti-reader infile\n"
        "Extract class information from an EXE or DLL using MSVC RTTI.\n"
        "\n"
        "Report bugs to <X-Fi6@phppoll.org>.\n"
        "rtti-reader is maintained by the Niotso project.\n"
        "Home page: <http://www.niotso.org/>\n");
        return 0;
    }

    InFile = argv[1];

    int slash;
    for(i=0, slash=-1; InFile[i]; i++)
        if(InFile[i] == '/' || InFile[i] == '\\') slash = i;
    BaseName = InFile + slash + 1;

    PEFile DLL(InFile);
    if(DLL.read16() != 0x5A4D) //"MZ"
        Shutdown_M("Not a valid Windows PE file");

    DLL.seek(60);
    DLL.seek(DLL.read32(), 6); unsigned SegmentCount = DLL.read16();
    DLL.skip(12);              unsigned OptionalHeaderSize = DLL.read16();
    DLL.skip(30);              unsigned ImageBase = DLL.read32();
    DLL.skip(OptionalHeaderSize, -32);

    for(i=0; i<SegmentCount; i++){
        if(!DLL.strcmp(".rdata")){
            DLL.skip(16); DLL.rdata.size = DLL.read32();
                          DLL.rdata.offset = DLL.read32();
            DLL.skip(16);
        } else if(!DLL.strcmp(".data")){
            DLL.skip(16); DLL.data.size = DLL.read32();
                          DLL.data.offset = DLL.read32();
            DLL.skip(16);
        } else DLL.skip(40);
    }
    if(DLL.rdata.size == 0)
        Shutdown_M("Missing .rdata segment");
    else if(DLL.data.size == 0)
        Shutdown_M("Missing .data segment");
    else if(DLL.rdata.size > UINT_MAX-DLL.rdata.offset || DLL.rdata.size+DLL.rdata.offset > DLL.brc.end)
        Shutdown_M(".rdata segment is invalid");
    else if(DLL.data.size > UINT_MAX-DLL.data.offset || DLL.data.size+DLL.data.offset > DLL.brc.end)
        Shutdown_M(".data segment is invalid");

    printf("\n****\n** [ 1 of 2] RTTI Report for %s\n****\n", BaseName);

    RTTIVector<RTTIClass> RCL;
    RCL.init();

    DLL.lookat(DLL.data);
    unsigned TotalClassCount = 0;
    while(DLL.skip(8) && DLL.memfind(".?AV", 4, PEFile::Parse_QuestionMark)){
        TotalClassCount++;
        size_t length = DLL.strlen();
        if(length == (unsigned)-1)
            Shutdown_M("Unexpectedly reached end of binary");

        size_t TDAddress = DLL.brc.position + ImageBase - 8, datapos = DLL.brc.position + length + 1;
        DLL.lookat(DLL.rdata);

        RTTIClass * RCPointer = NULL;

        for(size_t rdatapos = DLL.brc.position + 12;
            DLL.seek(rdatapos) && DLL.find32(TDAddress); rdatapos += 4, DLL.lookat(DLL.rdata)){
            //Find all complete object locators that belong to this class
            rdatapos = DLL.brc.position;
            if(!DLL.skip(4))
                continue;
            size_t CDAddress = DLL.read32() - ImageBase;
            if(CDAddress < DLL.brc.start || CDAddress > DLL.brc.end-4)
                continue; //This was a base class descriptor

            //Add this COL to our respective RTTIClass
            bool newclass = false;
            if(RCPointer == NULL){
                //This is a new class; add it to the RCL
                newclass = true;
                RTTIClass& RC = RCL.add();
                RCPointer = &RC;
                RC.init();
            }
            RTTIClass& RC = *RCPointer;

            RTTICompleteObjectLocator& COL = RC.COLL.add();

            DLL.seek(rdatapos,-12);
            size_t COLAddress = DLL.brc.position + ImageBase;
            if(!DLL.Fill(COL))
                Shutdown_M("Unexpectedly reached end of binary");

            DLL.lookat(DLL.rdata);
            COL.VTableAddress = (DLL.find32(COLAddress)) ? DLL.brc.position + ImageBase + 4 : (uint32_t)-1;

            if(newclass){
                if(!DLL.seek(COL.Fields.ClassDescriptorAddress - ImageBase))
                    Shutdown_M("Unexpectedly reached end of binary");
                RTTIClassHierarchyDescriptor& CHD = RC.CHD;
                if(!DLL.Fill(CHD))
                    Shutdown_M("Unexpectedly reached end of binary");

                if(!DLL.seek(CHD.Fields.BaseClassListAddress - ImageBase))
                    Shutdown_M("Unexpectedly reached end of binary");
                size_t bcdlpos;
                for(i=0, bcdlpos = DLL.brc.position; i<CHD.Fields.BaseClassCount; i++, bcdlpos+=4){
                    DLL.lookat(DLL.rdata);
                    if(!DLL.seek(bcdlpos))
                        Shutdown_M("Unexpectedly reached end of binary");
                    uint32_t BCDAddress = DLL.read32();
                    if(!DLL.seek(BCDAddress - ImageBase))
                        Shutdown_M("Unexpectedly reached end of binary");

                    RTTIBaseClassDescriptor& BCD = CHD.BCDL.add();
                    if(!DLL.Fill(BCD))
                        Shutdown_M("Unexpectedly reached end of binary");

                    DLL.lookat(DLL.data);
                    if(!DLL.seek(BCD.Fields.TypeDescriptorAddress - ImageBase))
                        Shutdown_M("Unexpectedly reached end of binary");
                    if(!DLL.Fill(BCD.TD))
                        Shutdown_M("Unexpectedly reached end of binary");

                    length = DLL.strlen();
                    if(length == (unsigned)-1)
                        Shutdown_M("Unexpectedly reached end of binary");
                    BCD.TD.Name = (char*) malloc(length+1);
                    BCD.TD.UnmangledName = (char*) malloc(length+1-6);
                    if(!BCD.TD.Name || !BCD.TD.UnmangledName)
                        Shutdown_M("Failed to allocate memory");
                    DLL.strcpy(BCD.TD.Name);
                    for(size_t j=0; j<length-6; j++){
                        char c = BCD.TD.Name[j+4];
                        BCD.TD.UnmangledName[j] = (
                            (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                            (c >= '0' && c <= '9') || c == '_')
                            ? c : ' ';
                    }
                    BCD.TD.UnmangledName[length-6] = '\0';

                    if(newclass){
                        newclass = false;
                        memcpy(&RC.TD, &BCD.TD, sizeof(RTTITypeDescriptor));
                    }
                }
            }
        }

        DLL.lookat(DLL.data);
        DLL.seek(datapos);
    }

    for (i=0; i<RCL.Count; i++){
        RTTIClass& RC = RCL.Buffer[i];
        RTTIClassHierarchyDescriptor& CHD = RC.CHD;
        printf("\nClass %s (mangled: %s)\n", RC.TD.UnmangledName, RC.TD.Name);
        printf(" * Class Hierarchy Descriptor: (Address %08X)\n", CHD.Fields.Address + ImageBase);
        printf("    * Reserved: %u\n", CHD.Fields.Reserved);
        printf("    * Attributes: %u\n", CHD.Fields.Attributes);
        printf("    * Base class count: %u\n", CHD.Fields.BaseClassCount);
        printf("    * Base class list address: %08X\n", CHD.Fields.BaseClassListAddress);
        for(uint32_t j=0; j<CHD.Fields.BaseClassCount; j++){
            RTTIBaseClassDescriptor& BCD = CHD.BCDL.Buffer[j];
            printf("       * Base class descriptor #%u: (Address %08X)\n", j, BCD.Fields.Address + ImageBase);
            printf("          * Base class count: %u\n", BCD.Fields.BaseClassCount);
            printf("          * Member offset: %d\n", BCD.Fields.MemberOffset);
            printf("          * COL address offset: %d\n", BCD.Fields.COLAddressOffset);
            printf("          * v-table offset: %d\n", BCD.Fields.VTableOffset);
            printf("          * Attributes: %u\n", BCD.Fields.Attributes);
            printf("            * Type descriptor: (Address %08X)\n", BCD.Fields.TypeDescriptorAddress);
            printf("               * v-table address: %08X\n", BCD.TD.Fields.VTableAddress);
            printf("               * Reserved: %u\n", BCD.TD.Fields.Reserved);
            printf("               * Name: %s (mangled: %s)\n", BCD.TD.UnmangledName, BCD.TD.Name);
        }
        for(uint32_t j=0; j<RC.COLL.Count; j++){
            RTTICompleteObjectLocator& COL = RC.COLL.Buffer[j];
            printf(" * Complete object locator #%u: (Address %08X)\n", j, COL.Fields.Address + ImageBase);
            printf("    * Reserved: %u\n", COL.Fields.Reserved);
            printf("    * Offset: %d\n", COL.Fields.Offset);
            printf("    * CD Offset: %d\n", COL.Fields.CDOffset);
            printf("    * Type descriptor address: %08X\n", COL.Fields.TypeDescriptorAddress);
            printf("    * Class descriptor address: %08X\n", COL.Fields.ClassDescriptorAddress);
            printf("    * v-table address: %08X\n", COL.VTableAddress);
        }
    }

    printf("\n****\n** [ 2 of 2 ] Class Hierarchy for %s\n****\n", BaseName);

    qsort(RCL.Buffer, RCL.Count, sizeof(RTTIClass), RTTIClass::Compare);

    for(i=0; i<RCL.Count; i++){
        RTTIClass& RC = RCL.Buffer[i];
        printf("\nclass %s", RC.TD.UnmangledName);
        if(RC.CHD.BCDL.Count > 1){
            //The first BCD always refers to the class itself, e.g. class A "depends on class A".
            printf(" : %s", RC.CHD.BCDL.Buffer[1].TD.UnmangledName);
            for(uint32_t j=2; j<RC.CHD.BCDL.Count; j++)
                printf(", %s", RC.CHD.BCDL.Buffer[j].TD.UnmangledName);
        }
        printf(";");
    }

    printf("\n\nCompleted RTTI report.\nDependencies provided for %u of %u classes.\n", (unsigned)RCL.Count, TotalClassCount);

    return 0;
}