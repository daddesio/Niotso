/*
    Niotso - The New Implementation of The Sims Online
    Client.cpp
    Copyright (c) 2012 Niotso Project <http://niotso.org/>
    Author(s): Fatbag <X-Fi6@phppoll.org>

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

#include "EngineInterface.hpp"

static void Shutdown();
Scene * CurrentScene;

#ifdef _WIN32

/* MinGW bonus feature; saves 66 kB in the binary :) */
/* */ void* __CRTDECL operator new(unsigned size){return malloc(size);}
/* */ void __CRTDECL operator delete(void *ptr){free(ptr);}
/* */ extern "C" void __CRTDECL __cxa_pure_virtual(){}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    int result;

    //Disallow multiple instances of the game from running
    CreateMutex(NULL, TRUE, "Global\\TSO_NIOTSO_MUTEX");
    if(GetLastError() == ERROR_ALREADY_EXISTS){
        HWND hWnd = FindWindow("TSO_NIOTSO", "The Sims Online");
        if(hWnd != NULL){
            ShowWindow(hWnd, SW_RESTORE);
            SetForegroundWindow(hWnd);
            SetFocus(hWnd);
        }
        return ERROR_INIT_ANOTHERINSTRUNNING;
    }

    Window::Width = 800;
    Window::Height = 600;
    Window::Fullscreen = false;
    System::hInst = hInstance;

    result = Window::Initialize();
    if(result != 0){
        Shutdown();
        return ERROR_INIT | ERROR_INIT_WINDOW | result;
    }

    result = System::Initialize();
    if(result != 0){
        Shutdown();
        return ERROR_INIT | ERROR_INIT_SYSTEM | result;
    }

    result = Graphics::Initialize();
    if(result != 0){
        Shutdown();
        return ERROR_INIT | ERROR_INIT_GRAPHICS | result;
    }

    result = Audio::Initialize();
    if(result != 0){
        Shutdown();
        return ERROR_INIT | ERROR_INIT_AUDIO | result;
    }

    CurrentScene = new LoginScreen();
    if(CurrentScene == NULL || System::SceneFailed){
        Shutdown();
        return ERROR_INIT | ERROR_INIT_LOGIC | ERROR_LOGIC_CREATE_SCENE;
    }

    ShowWindow(Window::hWnd, SW_SHOW);
    SetForegroundWindow(Window::hWnd);
    SetFocus(Window::hWnd);

    LARGE_INTEGER PreviousTime;
    QueryPerformanceCounter(&PreviousTime);

    while(true){
        LARGE_INTEGER CurrentTime;
        QueryPerformanceCounter(&CurrentTime);
        float TimeDelta = (float)(CurrentTime.QuadPart-PreviousTime.QuadPart)/System::ClockFreq.QuadPart;
        PreviousTime = CurrentTime;
        if(TimeDelta < 0 || TimeDelta >= 5) //Safe-guard in case of system delay
            continue;

        memcpy(&System::UserInput, (const void*) &System::UserInput_v, sizeof(System::UserInput_t));

        int result = CurrentScene->RunFor(TimeDelta);
        if(result == System::SHUTDOWN)
            break;
        if(result > 0){
            glClear(GL_COLOR_BUFFER_BIT);
            CurrentScene->Render();
            SwapBuffers(Graphics::hDC);
        }

        //Sleep for the remainder of the frame
        PreviousTime.QuadPart = CurrentTime.QuadPart;
        QueryPerformanceCounter(&CurrentTime);
        float SleepDuration =
            (System::FramePeriod - (float)(CurrentTime.QuadPart-PreviousTime.QuadPart)/System::ClockFreq.QuadPart) * 1000;
        if(SleepDuration > 1 && SleepDuration < 100) Sleep((unsigned) SleepDuration);
    }

    ShowWindow(Window::hWnd, SW_HIDE);
    delete CurrentScene;

    Shutdown();
    return 0;
}

static void Shutdown()
{
    Audio::Shutdown();
    Graphics::Shutdown();
    System::Shutdown();
    Window::Shutdown();
}

#endif