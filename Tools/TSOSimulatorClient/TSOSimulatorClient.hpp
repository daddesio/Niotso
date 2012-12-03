/*
    TSOSimulatorClient - TSOSimulatorClientD.dll injector
    TSOSimulatorClient.hpp - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>
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
    //Base classes: cRZString, std::char_traits, ?$_String_base@DV?$__default_alloc_template@$00$0A@@std@@@std, cIGZString, cIGZUnknown
    DWORD   Zero1;
    DWORD   Zero2;
    void *  vtable5;
    char ** Strings1;
    char ** Strings2;
    char ** Strings3;
    DWORD   Zero3;
    DWORD   Zero4;
    DWORD   Zero5;
    void ** Pointer1; //12 bytes
    void ** Pointer2; //4 bytes
    void ** Pointer3;
    DWORD   Flags;
    DWORD * Pointer4; //4 bytes
    void *  Pointer5;
    void *  Pointer6;
    DWORD   Unknown11;
    DWORD   Unknown12;
};

DECLARE_INTERFACE(cTSOSimulatorClientDCOMDirector)
{
    //Base classes: cTSOSimulatorClientDCOMDirector, cRZCOMDllDirector, cIGZCOMDirector, cIGZUnknown, cIGZFrameWorkHooks, cIGZUnknown
    void * vtable2;
    void * vtable1;
    cRZString String1;
    void * vtable4;
    void * vtable3;
    cRZString String2;
    cRZString String3;
    
    DWORD Zero1;
    DWORD Zero2;
    DWORD Zero3;
    DWORD Zero4;
    DWORD Zero5;
    DWORD Zero6;
    DWORD Zero7;
    DWORD Zero8;
    DWORD Zero9;
    DWORD Zero10;
    DWORD Unknown1;
    DWORD Pointer1;
    DWORD Pointer2;
    DWORD Zero11;
};