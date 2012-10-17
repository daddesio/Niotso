/*
    inputtest - Windows input testing
    inputtest.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch(uMsg){
    
    case WM_KEYDOWN:
        printf("[WM_KEYDOWN] key = %u, lParam = %lu\n", wParam, lParam);
        return 0;
    case WM_KEYUP:
        printf("[WM_KEYUP] key = %u, lParam = %lu\n", wParam, lParam);
        return 0;
    case WM_CHAR:
        printf("[WM_CHAR] key = %u, lParam = %lu\n", wParam, lParam);
        return 0;
    case WM_DEADCHAR:
        printf("[WM_DEADCHAR] key = %u, lParam = %lu\n", wParam, lParam);
        return 0;
    case WM_SYSKEYDOWN:
        printf("[WM_SYSKEYDOWN] key = %u, lParam = %lu\n", wParam, lParam);
        return 0;
    case WM_SYSKEYUP:
        printf("[WM_SYSKEYUP] key = %u, lParam = %lu\n", wParam, lParam);
        return 0;
    case WM_SYSDEADCHAR:
        printf("[WM_SYSDEADCHAR] key = %u, lParam = %lu\n", wParam, lParam);
        return 0;
    case WM_HOTKEY:
        printf("[WM_HOTKEY] keys1 = %u, keys2 = %lu\n", wParam, lParam);
        return 0;
    case WM_APPCOMMAND:
        printf("[WM_APPCOMMAND] cmd = %u, device = %u, keys = %u\n", GET_APPCOMMAND_LPARAM(lParam), GET_DEVICE_LPARAM(lParam), GET_KEYSTATE_LPARAM(lParam));
        return 0;

    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;

    }
    
    printf("Received message %u with wParam = %u, lParam = %lu\n", uMsg, wParam, lParam);
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    const WNDCLASS wc = {
        0,              //style
        WndProc,        //lpfnWndProc
        0,              //cbClsExtra
        0,              //cbWndExtra
        hInstance,      //hInstance
        (HICON) LoadImage(NULL, IDI_APPLICATION, //hIcon
            IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE),
        (HCURSOR) LoadImage(NULL, IDC_ARROW,     //hCursor
            IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE),
        (HBRUSH) COLOR_WINDOW,                   //hbrBackground
        NULL,           //lpszMenuName
        "INPUTTEST"     //lpszClassName
    };
    
    RegisterClass(&wc);

    // Create the main window.
    const HWND hWnd = CreateWindowEx(WS_EX_APPWINDOW, "INPUTTEST", "Input test", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, 0, 0, hInstance, NULL);
    
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}