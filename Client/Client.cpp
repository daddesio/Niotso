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

#include "EngineInterface.hpp"

namespace Window {
    unsigned Width, Height;
    bool Fullscreen = false;
    HWND hWnd = NULL;
}

int CreateWindowInvisible(HINSTANCE hInst, unsigned Width, unsigned Height, bool Fullscreen);
void Shutdown();

Scene * CurrentScene;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    int result;
    
    //Disallow multiple instances of the game from running
    CreateMutex(NULL, TRUE, "TSO_NIOTSO_MUTEX");
    if(GetLastError() == ERROR_ALREADY_EXISTS){
        HWND hWnd = FindWindow("TSO_NIOTSO", "The Sims Online");
        if(hWnd != NULL){
            ShowWindowAsync(hWnd, SW_RESTORE);
            SetForegroundWindow(hWnd);
            SetFocus(hWnd);
        }
        return ERROR_INIT_ANOTHERINSTRUNNING;
    }
    
    result = CreateWindowInvisible(hInstance, 800, 600, false);
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
        MSG msg;
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        LARGE_INTEGER CurrentTime;
        QueryPerformanceCounter(&CurrentTime);
        float TimeDelta = (float)(CurrentTime.QuadPart-PreviousTime.QuadPart)/System::ClockFreq.QuadPart;
        if(TimeDelta < 0 || TimeDelta >= 5) //Invalid TimeDelta
            continue;
        
        int result = CurrentScene->RunFor(TimeDelta);
        if(result == System::SHUTDOWN)
            break;
        if(result > 0){
            glClear(GL_COLOR_BUFFER_BIT);
            CurrentScene->Render();
            SwapBuffers(Graphics::hDC);
        }
        
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

int CreateWindowInvisible(HINSTANCE hInst, unsigned Width, unsigned Height, bool Fullscreen)
{
    const WNDCLASS wc = {
        CS_OWNDC,       //style
        WndProc,        //lpfnWndProc
        0,              //cbClsExtra
        0,              //cbWndExtra
        hInst,          //hInstance
        (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_TSO), //hIcon
            IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE),
        NULL,           //hCursor
        NULL,           //hbrBackground
        NULL,           //lpszMenuName
        "TSO_NIOTSO"    //lpszClassName
    };

    if(!RegisterClass(&wc)){
        MessageBox(NULL, "Failed to register the window class.", NULL, MB_OK | MB_ICONERROR);
        return ERROR_REGISTER_CLASS;
    }
    SetCursor(NULL);

    HWND hWnd = NULL;
    if(Fullscreen){
        DEVMODE dmScreenSettings;
        dmScreenSettings.dmSize         = sizeof(dmScreenSettings);
        dmScreenSettings.dmFields       = DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL;
        dmScreenSettings.dmPelsWidth    = Width;
        dmScreenSettings.dmPelsHeight   = Height;
        dmScreenSettings.dmBitsPerPel   = 32;

        if(ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL){
            MessageBox(NULL, "Fullscreen mode is not supported by your setup. It has been turned off.", NULL,
                MB_OK | MB_ICONERROR);
            Fullscreen = 0;
        }else{
            hWnd = CreateWindowEx(WS_EX_APPWINDOW, "TSO_NIOTSO", "The Sims Online", WS_POPUP,
                0, 0, Width, Height, 0, 0, hInst, NULL);
        }
    }
    if(hWnd == NULL){
        Fullscreen = false;
        RECT WindowRect = {0, 0, Width, Height};
        
        //Use a style of WS_OVERLAPPEDWINDOW to allow resizing
        AdjustWindowRectEx(&WindowRect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE,
            WS_EX_APPWINDOW);   //This finds the dimensions of a window with a client area of our specified dimensions

        //Note: Windows can be positioned anywhere, even outside the visible workspace,
        //but their sizes are limited to the size of the workspace on the primary display.
        unsigned WindowWidth = WindowRect.right-WindowRect.left, WindowHeight = WindowRect.bottom-WindowRect.top;
        RECT WorkspaceRect;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkspaceRect, 0);
        
        hWnd = CreateWindowEx(WS_EX_APPWINDOW, "TSO_NIOTSO", "The Sims Online",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            ((WorkspaceRect.right-WorkspaceRect.left - WindowWidth)>>1) + WorkspaceRect.left,
            ((WorkspaceRect.bottom-WorkspaceRect.top - WindowHeight)>>1) + WorkspaceRect.top,
            WindowWidth, WindowHeight, 0, 0, hInst, NULL);
    }
    
    if(hWnd == NULL){
        MessageBox(NULL, "Failed to create the window.", NULL, MB_OK | MB_ICONERROR);
        return ERROR_CREATE_WINDOW;
    }
    
    Window::hWnd = hWnd;
    Window::Width = Width;
    Window::Height = Height;
    Window::Fullscreen = Fullscreen;
    System::hInst = hInst;
    return 0;
}

void Shutdown()
{
    Audio::Shutdown();
    Graphics::Shutdown();
    System::Shutdown();

    if(Window::hWnd){
        DestroyWindow(Window::hWnd);
        Window::hWnd = NULL;
    }
    UnregisterClass("TSO_NIOTSO", System::hInst);
}