/*
    libgldemo - General-purpose OpenGL demo backend
    wgl.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include "libgldemo.h"

static HWND hWnd = NULL;
static HDC hDC = NULL;
static HGLRC hRC = NULL;
static HINSTANCE hInst;

static uint8_t keys[256] = {0};
static uint16_t ResWidth, ResHeight, ResDepth;
static int fullscreen = 0;
static float FramePeriod;

static void KillGLWindow()
{
    if(fullscreen){
        ShowCursor(1);
    }

    if(hRC){
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hRC);
        hRC = NULL;
    }

    if(hDC){
        ReleaseDC(hWnd, hDC);
        hDC = NULL;
    }

    if(hWnd){
        DestroyWindow(hWnd);
        hWnd = NULL;
    }
}

static int CreateGLWindow(const char *__restrict title, uint16_t width, uint16_t height)
{
    const PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1, /* Size and version */
        PFD_DRAW_TO_WINDOW |              /* dwFlags */
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                    /* iPixelType */
        0,                                /* cColorBits */
        0, 0, 0, 0, 0, 0, 0, 0,           /* R,G,B,A bits */
        0, 0, 0, 0, 0,                    /* Accumulation buffer bits */
        16,                               /* cDepthBits */
        0,                                /* cStencilBits */
        0,                                /* cAuxBuffers */
        PFD_MAIN_PLANE,                   /* iLayerType */
        0,                                /* Reserved */
        0, 0, 0                           /* Masks */
    };
    DWORD dwStyle, dwExStyle;
    RECT WindowRect;
    int PixelFormat;
    BOOL (WINAPI *wglSwapIntervalEXT)(int);
    int (WINAPI *wglGetSwapIntervalEXT)(void);

    if(fullscreen){
        dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
        dwStyle = WS_POPUP;

        width  = ResWidth;
        height = ResHeight;
        ShowCursor(0);
    }else{
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_OVERLAPPEDWINDOW;

        WindowRect.left   = 0;
        WindowRect.right  = width;
        WindowRect.top    = 0;
        WindowRect.bottom = height;
        AdjustWindowRectEx(&WindowRect, dwStyle, 0, dwExStyle);
        width = WindowRect.right - WindowRect.left;
        height = WindowRect.bottom - WindowRect.top;
    }

    if(!(hWnd = CreateWindowEx(dwExStyle, "OpenGL",
        title,
        dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        (ResWidth - width)>>1,
        (ResHeight - height)>>1,
        width,
        height,
        NULL, NULL, hInst, NULL))
    ){
        KillGLWindow();
        DemoErrorBox("Window creation error.");
        return 0;
    }

    hDC = GetDC(hWnd);
    if(!hDC){
        KillGLWindow();
        DemoErrorBox("Failed to create an OpenGL device context.");
        return 0;
    }

    PixelFormat = ChoosePixelFormat(hDC, &pfd);
    if(!PixelFormat){
        KillGLWindow();
        DemoErrorBox("Can't find a suitable PixelFormat.");
        return 0;
    }

    if(!SetPixelFormat(hDC, PixelFormat, &pfd)){
        KillGLWindow();
        DemoErrorBox("Can't set the PixelFormat.");
        return 0;
    }

    hRC = wglCreateContext(hDC);
    if(!hRC){
        KillGLWindow();
        DemoErrorBox("Failed to create an OpenGL rendering context.");
        return 0;
    }

    if(!wglMakeCurrent(hDC, hRC)){
        KillGLWindow();
        DemoErrorBox("Failed to activate the OpenGL device context.");
        return 0;
    }

    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);

    if(!Demo.InitGL()){
        KillGLWindow();
        DemoErrorBox("Initialization failed.");
        return 0;
    }
    if(!Demo.ResizeScene(width&&height ? width : 1, width&&height ? height : 1)){
        KillGLWindow();
        DemoErrorBox("Scene resize failed.");
        return 0;
    }

    wglSwapIntervalEXT = (BOOL (WINAPI *)(int)) wglGetProcAddress("wglSwapIntervalEXT");
    if(wglSwapIntervalEXT) wglSwapIntervalEXT(1);
    wglGetSwapIntervalEXT = (int (WINAPI *)(void)) wglGetProcAddress("wglGetSwapIntervalEXT");
    if(wglGetSwapIntervalEXT) wglGetSwapIntervalEXT(); /* Seems necessary on some cards */

    return 1;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_KEYDOWN:
        if(wParam == VK_ESCAPE){
            PostQuitMessage(0);
        }else if(wParam == VK_F11 && !keys[VK_F11]){
            KillGLWindow();
            fullscreen = !fullscreen;
            if(!CreateGLWindow(Demo.Title, Demo.Width, Demo.Height))
                PostQuitMessage(0);
        }
    case WM_KEYUP:
        keys[wParam] = (uMsg == WM_KEYDOWN);
        return 0;

    case WM_DEVMODECHANGE: {
        DEVMODE dm;
        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
        ResWidth = dm.dmPelsWidth;
        ResHeight = dm.dmPelsHeight;
        ResDepth = dm.dmBitsPerPel;
        FramePeriod = 1.0f/dm.dmDisplayFrequency;
    }
    case WM_SIZE:
        Demo.ResizeScene(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = {
        CS_HREDRAW | CS_VREDRAW | CS_OWNDC, /* style */
        (WNDPROC) WndProc,   /* lpfnWndProc */
        0,                   /* cbClsExtra */
        0,                   /* cbWndExtra */
        NULL,                /* hInstance */
        NULL,                /* hIcon */
        NULL,                /* hCursor */
        NULL,                /* hbrBackground */
        NULL,                /* lpszMenuName */
        "OpenGL"             /* lpszClassName */
    };
    DEVMODE dm;
    LARGE_INTEGER ClockFreq, PreviousTime;
    
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    wc.hInstance = hInst = hInstance;
    wc.hIcon = (HICON) LoadImage(NULL, IDI_WINLOGO, IMAGE_ICON, 0, 0, LR_SHARED);
    wc.hCursor = (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
    ResWidth = dm.dmPelsWidth;
    ResHeight = dm.dmPelsHeight;
    ResDepth = dm.dmBitsPerPel;
    FramePeriod = 1.0f/dm.dmDisplayFrequency;
    QueryPerformanceFrequency(&ClockFreq);

    if(!RegisterClass(&wc)){
        MessageBox(NULL, "Failed to register the window class.", NULL, MB_OK | MB_ICONERROR);
        return 0;
    }

    if(Demo.Startup && !Demo.Startup())
        return -1;

    if(!CreateGLWindow(Demo.Title, Demo.Width, Demo.Height))
        return -1;

    QueryPerformanceCounter(&PreviousTime);

    while(1){
        MSG msg;
        LARGE_INTEGER CurrentTime;
        float TimeDelta;

        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if(msg.message == WM_QUIT){
                KillGLWindow();
                UnregisterClass("OpenGL", hInstance);
                return (!Demo.Shutdown || Demo.Shutdown()) ? 0 : -1;
            }
        }

        /* Find the timedelta */
        QueryPerformanceCounter(&CurrentTime);
        TimeDelta = (float)(CurrentTime.QuadPart-PreviousTime.QuadPart)/ClockFreq.QuadPart;
        PreviousTime = CurrentTime;
        if(TimeDelta < 0 || TimeDelta > 5) /* Safe-guard in case of system delay */
            continue;

        /* Draw */
        Demo.DrawScene(TimeDelta, keys);
        SwapBuffers(hDC);

        /* Sleep for the remainder of the frame */
        QueryPerformanceCounter(&CurrentTime);
        TimeDelta = (float)(CurrentTime.QuadPart-PreviousTime.QuadPart)/ClockFreq.QuadPart;
        TimeDelta = (FramePeriod - TimeDelta) * 1000;
        if(TimeDelta > 1 && TimeDelta < 100) Sleep((unsigned) TimeDelta);
    }
}
