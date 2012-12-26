/*
    TSOEdithEditor - TSOEdithEditorD.dll injector
    TSOEdithEditor.hpp - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>
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

#include <basetyps.h>
#pragma pack(0)

DECLARE_INTERFACE(cRZString)
{
    void * vtable1_cRZString;
    char * mpBegin;    //Pointer to beginning of string
    char * mpEnd;      //Pointer to null terminator
    char * mpCapacity; //mpEnd + 1
    DWORD mAllocator;  //0
    DWORD Zero1;       //0
};

struct stringstruct
{
    DWORD StringID;
    DWORD Unknown;
    char * PointerToBuffer; //Buffer
    DWORD SizeOfBuffer;     //256
    char Buffer[256];
};

DECLARE_INTERFACE(cEdithEditorCOMDirector)
{
    void * vtable_1_cEdithEditorCOMDirector;
    void * vtable_2_cEdithEditorCOMDirector;
    DWORD Zero1;
    DWORD Zero2;
    cRZString string;
    DWORD Zero5;
    DWORD Zero6;
    DWORD Zero7;
    DWORD Zero8;
    DWORD Zero9;
    DWORD Zero10;
    DWORD Zero11;
    void * ptr;
    DWORD Value1;           //2
    DWORD Value2;           //1
    float Value3;           //1.0f
    DWORD Value4;           //0x40000000
    DWORD Value5;           //2
    DWORD Value6;           //0
    DWORD Value7;           //1
    DWORD Value8;           //0
    stringstruct string0;   //StringID:0, Unknown:40,  value:"index"
    stringstruct string1;   //StringID:1, Unknown:40,  value:"value"
    stringstruct string2;   //StringID:2, Unknown:150, value:"Name"
    stringstruct string3;   //StringID:3, Unknown:200, value:"Description"
    DWORD Value9;           //0
    DWORD Value10;          //0
    stringstruct string4;   //StringID:0, Unknown:90,  value:"Calling Tree"
    stringstruct string5;   //StringID:1, Unknown:86,  value:"Type"
    stringstruct string6;   //StringID:2, Unknown:83,  value:"Title"
    stringstruct string7;   //StringID:3, Unknown:65,  value:"Yes"
    stringstruct string8;   //StringID:4, Unknown:65,  value:"No"
    stringstruct string9;   //StringID:5, Unknown:65,  value:"Cancel"
    stringstruct string10;  //StringID:6, Unknown:300, value:"Message"
    stringstruct string11;  //StringID:7, Unknown:45,  value:"Tree ID"
    stringstruct string12;  //StringID:8, Unknown:50,  value:"Node #"
};

DECLARE_INTERFACE(cTSOEdithEditorDCOMDirector)
{
    void * vtable1_cTSOEdithEditorDCOMDirector;
    void * vtable2_cTSOEdithEditorDCOMDirector;
    DWORD Zero1;
    DWORD Zero2;
    cRZString String1;
    DWORD Zero5;
    DWORD Zero6;
    cEdithEditorCOMDirector ** memptr_1;
    void ** memptr_2;
    void ** memptr_3; //Same as memptr_2
    DWORD Zero7;
    DWORD Zero8;
    void * dllptr_4_100B5834; //CMemoryException TD
    DWORD Value1; //1
    DWORD Value2; //0
    float Value3; //1.0f
    DWORD Value4; //0x40000000
    DWORD Value5; //0
    DWORD Value6; //0
    DWORD Value7; //1
    cRZString String2;
    cRZString String3;
    cRZString String4;
    cRZString String5;
    cRZString String6;
    cRZString String7;
    cRZString String8;
    cRZString String9;
    DWORD Zero9;
    DWORD Zero10;
    DWORD Zero11;
    DWORD Zero12;
    DWORD Zero13;
    DWORD Zero14;
    DWORD Zero15;
    cRZString String10;
    cRZString String11;
    cRZString String12;
    cRZString String13;
    cRZString String14;
    DWORD Zero16;
    DWORD Zero17;
    DWORD Zero18;
    DWORD Zero19;
    DWORD Zero20;
    cRZString String15;
    cRZString String16;
    cRZString String17;
    cRZString String18;
    cRZString String19;
    cRZString String20;
    cRZString String21;
    cRZString String22;
    cRZString String23;
    cRZString String24;
    cRZString String25;
    cRZString String26;
    cRZString String27;
    cRZString String28;
    cRZString String29;
    cRZString String30;
};