/*
    Niotso - The New Implementation of The Sims Online
    Window/Window.cpp
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

#include "../EngineInterface.hpp"

namespace Window {

unsigned Width, Height;
bool Fullscreen;
HWND hWnd;

static HANDLE Response = NULL;
static HANDLE Thread = NULL;
static DWORD ThreadID = NULL;
static volatile int Result = NULL;

static DWORD WINAPI Procedure(LPVOID);
static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static int CreateWindowInvisible(HINSTANCE hInst, unsigned Width, unsigned Height, bool Fullscreen);

int Initialize(){
    Response = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(Response == NULL){
        MessageBox(NULL, "Failed to create the message loop synchronization object.", NULL, MB_OK | MB_ICONERROR);
        Shutdown();
        return ERROR_WINDOW_SYNCOBJECT;
    }

    Thread = CreateThread(NULL, 1024 /* very tiny stack size is needed */, Window::Procedure, NULL, 0, &ThreadID);
    if(Thread == NULL){
        MessageBox(NULL, "Failed to create the message loop thread.", NULL, MB_OK | MB_ICONERROR);
        Shutdown();
        return ERROR_WINDOW_CREATE_THREAD;
    }

    if(WaitForSingleObject(Window::Response, INFINITE) != WAIT_OBJECT_0){
        MessageBox(NULL, "Failed to synchronize with the message loop thread.", NULL, MB_OK | MB_ICONERROR);
        Shutdown();
        return ERROR_WINDOW_SYNCHRONIZE;
    }

    if(Result != 0){
        Shutdown();
        return Result;
    }

    hWnd = FindWindow("TSO_NIOTSO", "The Sims Online");
    if(hWnd == NULL){
        MessageBox(NULL, "Failed to obtain a handle for the window.", NULL, MB_OK | MB_ICONERROR);
        Shutdown();
        return ERROR_WINDOW_HANDLE;
    }

    return 0;
}

void Shutdown(){
    if(hWnd){
        DestroyWindow(hWnd);
        hWnd = NULL;
    }
    if(Thread){
        DWORD value;
        while(GetExitCodeThread(Thread, &value) && value){
            PostThreadMessage(ThreadID, WM_QUIT, 0, 0);
            Sleep(1);
        }
        Thread = NULL;
    }
    if(Response){
        CloseHandle(Response);
        Response = NULL;
    }

    UnregisterClass("TSO_NIOTSO", System::hInst);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch(uMsg){

    case WM_SETCURSOR:
        SetCursor(NULL);
        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
        System::UserInput_v.Keys[wParam] = (uMsg == WM_KEYDOWN);
        return 0;

    case WM_CLOSE:
        System::UserInput_v.CloseWindow = true;
        return 0;

    case WM_DEVMODECHANGE: {
        DEVMODE dm;
        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
        System::FramePeriod = 1.0f/dm.dmDisplayFrequency;
    } return 0;

    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static DWORD WINAPI Procedure(LPVOID){
    int result = CreateWindowInvisible(System::hInst, Window::Width, Window::Height, Window::Fullscreen);
    if(result != 0){
        Window::Result = result;
        SetEvent(Window::Response);
        return 0;
    }
    SetEvent(Window::Response);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

static int CreateWindowInvisible(HINSTANCE hInst, unsigned Width, unsigned Height, bool Fullscreen)
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
        return ERROR_WINDOW_REGISTER_CLASS;
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
            WS_EX_APPWINDOW); //This finds the dimensions of a window with a client area of our specified dimensions

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
        return ERROR_WINDOW_CREATE;
    }

    return 0;
}

}