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

PlayableSound_t * LoadSound(const Sound_t * Sound){
    const WAVEFORMATEX wfx = {
        WAVE_FORMAT_PCM,     //wFormatTag
        Sound->Channels,     //nChannels
        Sound->SamplingRate, //nSamplesPerSec
        ((Sound->Channels * Sound->BitDepth) >> 3) * Sound->SamplingRate, //nAvgBytesPerSec
        ((Sound->Channels * Sound->BitDepth) >> 3), //nBlockAlign
        Sound->BitDepth,     //wBitsPerSample
        0                    //cbSize
    };
    
    const XAUDIO2_BUFFER buffer = {
        0, //Flags
        Sound->Duration * wfx.nBlockAlign, //AudioBytes
        Sound->Data, //pAudioData
        0, //PlayBegin
        0, //PlayLength
        0, //LoopBegin
        0, //LoopLength
        XAUDIO2_LOOP_INFINITE, //LoopCount
        NULL, //pContext
    };
    
    IXAudio2SourceVoice* pSourceVoice;
    if(FAILED(pXAudio2->CreateSourceVoice(&pSourceVoice, &wfx)))
        return NULL;
    if(FAILED(pSourceVoice->SubmitSourceBuffer(&buffer)))
        return NULL;
    
    PlayableSound_t * PlayableSound = (PlayableSound_t*) malloc(sizeof(PlayableSound_t));
    if(!PlayableSound)
        return NULL;
    PlayableSound->pSourceVoice = pSourceVoice;
    PlayableSound->Playing = false;
    PlayableSound->Data = Sound->Data;
    return PlayableSound;
}

bool PlaySound(PlayableSound_t * Sound){
    if(!Sound->Playing && !FAILED(Sound->pSourceVoice->Start(0))){
        Sound->Playing = true;
        return true;
    }
    return false;
}

bool StopSound(PlayableSound_t * Sound){
    int success = false;
    if(Sound->Playing && !FAILED(Sound->pSourceVoice->Stop(0)))
        success = true;
    Sound->Playing = false;
    return success;
}

void DeleteSound(PlayableSound_t * Sound){
    StopSound(Sound);
    //Sound->pSourceVoice->Release();
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