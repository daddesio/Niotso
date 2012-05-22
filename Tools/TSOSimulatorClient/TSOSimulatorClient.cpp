/*
    TSOSimulatorClient - TSOSimulatorClientD.dll injector
    iff2html.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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
#include "TSOSimulatorClient.hpp"

int main(){
    HMODULE dllmodule = LoadLibrary("TSOSimulatorClientD.dll");
    if(dllmodule == NULL){
        printf("TSOSimulatorClient: Error: Failed to load DLL \"TSOSimulatorClientD.dll\".");
        return -1;
    }
    
    GZCOMDirector * (__stdcall *GZDllGetGZCOMDirector)(void) =
        (GZCOMDirector * (__stdcall *)(void)) GetProcAddress(dllmodule, "GZDllGetGZCOMDirector");
    if(GZDllGetGZCOMDirector == NULL){
        printf("TSOSimulatorClient: Error: Failed to find GZDllGetGZCOMDirector() in TSOSimulatorClientD.dll.");
        return -1;
    }
    
    printf("TSOSimulatorClient: Calling GZDllGetGZCOMDirector() ...\n");
    GZCOMDirector * Simulator = GZDllGetGZCOMDirector();
    printf("TSOSimulatorClient: Finished calling GZDllGetGZCOMDirector().\nThe value returned was: %p.\n", (void *) Simulator);
    
    printf("%s\n%s\n%s\n", Simulator->Object1.Strings1[0], Simulator->Object1.Strings2[0], Simulator->Object1.Strings3[0]);
    
    printf("TSOSimulatorClient: Exiting.\n");
    FreeLibrary(dllmodule);
    return 0;
}