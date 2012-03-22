/*
    Niotso - Copyright (C) 2012 Fatbag <X-Fi6@phppoll.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../EngineInterface.hpp"

namespace Audio {

HANDLE Thread;

IXAudio2 *pXAudio2 = NULL;
IXAudio2MasteringVoice *MasterVoice = NULL;

int Initialize(){
    HRESULT result;
    
    result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
    if(result != S_OK){
        MessageBox(Window::hWnd, "Failed to initialize Microsoft COM.", NULL, MB_OK | MB_ICONERROR);
        Shutdown();
        return ERROR_AUDIO_INIT_COM;
    }

    result = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if(result != S_OK){
        MessageBox(Window::hWnd, "Failed to initialize XAudio2. Please download the latest DirectX runtime for your system.",
            NULL, MB_OK | MB_ICONERROR);
        Shutdown();
        return ERROR_AUDIO_INIT_XAUDIO2;
    }
    
    result = pXAudio2->CreateMasteringVoice(&MasterVoice, 2, 44100, 0, 0, NULL);
    if(result != S_OK){
        MessageBox(Window::hWnd, "Failed to create the mastering voice for XAudio2.", NULL, MB_OK | MB_ICONERROR);
        Shutdown();
        return ERROR_AUDIO_CREATE_VOICE;
    }
    return 0;
}

void Shutdown(){
    if(MasterVoice){
        MasterVoice->DestroyVoice();
        MasterVoice = NULL;
    }
    if(pXAudio2){
        pXAudio2->Release();
        pXAudio2 = NULL;
    }
}

}