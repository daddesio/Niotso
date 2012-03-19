/*
    TSOSimulatorClient.hpp - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>

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

DECLARE_INTERFACE(UnknownObject1_t)
{
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

DECLARE_INTERFACE(GZCOMDirector)
{
    void * vtable2;
    void * vtable1;
    UnknownObject1_t Object1;
    void * vtable4;
    void * vtable3;
    UnknownObject1_t Object2;
    UnknownObject1_t Object3;
    
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