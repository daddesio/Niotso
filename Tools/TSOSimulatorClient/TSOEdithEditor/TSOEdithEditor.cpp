/*
    TSOEdithEditor - TSOEdithEditorD.dll injector
    TSOEdithEditor.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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
#include <windows.h>
#include "TSOEdithEditor.hpp"

int main(){
    HMODULE dllmodule = LoadLibrary("TSOEdithEditorD.dll");
    if(dllmodule == NULL){
        printf("TSOEdithEditor: Error: Failed to load DLL \"TSOEdithEditorD.dll\".");
        return -1;
    }

    cTSOEdithEditorDCOMDirector * (__stdcall *GZDllGetGZCOMDirector)(void) =
        (cTSOEdithEditorDCOMDirector * (__stdcall *)(void)) GetProcAddress(dllmodule, "GZDllGetGZCOMDirector");
    if(GZDllGetGZCOMDirector == NULL){
        printf("TSOEdithEditor: Error: Failed to find GZDllGetGZCOMDirector() in TSOEdithEditorD.dll.");
        return -1;
    }

    printf("TSOEdithEditor: Calling GZDllGetGZCOMDirector() ...\n");
    cTSOEdithEditorDCOMDirector * Edith = GZDllGetGZCOMDirector();
    printf("TSOEdithEditor: Finished calling GZDllGetGZCOMDirector().\nThe value returned was: %p.\n", (void *) Edith);

    while(true){
        char buffer[8];
        printf("\nCall a function (0, 1, 2, ...) or q to exit. ");
        //fgets(buffer, 8, stdin);
        //if(buffer[0] == 'q') break;
        //Edith->Object1.vtable5[atoi(buffer)]();
    }

    printf("TSOEdithEditor: Exiting.\n");
    FreeLibrary(dllmodule);
    return 0;
}