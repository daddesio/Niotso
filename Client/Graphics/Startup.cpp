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

namespace Graphics {

HDC hDC;
HGLRC hRC;

int Initialize(){
    hDC = GetDC(Window::hWnd);
    if(hDC == NULL){
        MessageBox(Window::hWnd, "Failed to obtain the device context.", NULL, MB_OK | MB_ICONERROR);
        Shutdown();
        return ERROR_GRAPHICS_OBTAIN_DC;
    }
    
    const PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1, //Size and version
        PFD_DRAW_TO_WINDOW |              //dwFlags
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                    //iPixelType
        24,                               //cColorBits
        0, 0, 0, 0, 0, 0, 0, 0,           //R,G,B,A bits
        0, 0, 0, 0, 0,                    //Accumulation buffer bits
        16,                               //cDepthBits
        0,                                //cStencilBits
        0,                                //cAuxBuffers
        PFD_MAIN_PLANE,                   //iLayerType
        0,                                //Reserved
        0, 0, 0                           //Masks
    };
    
    unsigned PixelFormat = ChoosePixelFormat(hDC, &pfd);
    if(!PixelFormat){
        MessageBox(Window::hWnd, "Failed to find a suitable pixel format for the device context.", NULL, MB_OK | MB_ICONERROR);
        Shutdown();
        return ERROR_GRAPHICS_FIND_PIXELFORMAT;
    }

    if(!SetPixelFormat(hDC, PixelFormat, &pfd)){
        MessageBox(Window::hWnd, "Failed to set the pixel format for the device context.", NULL, MB_OK | MB_ICONERROR);
        Shutdown();
        return ERROR_GRAPHICS_SET_PIXELFORMAT;
    }
    
    hRC = wglCreateContext(hDC);
    if(!hRC){
        MessageBox(Window::hWnd, "Failed to create an OpenGL rendering context.", NULL, MB_OK | MB_ICONERROR);
        Shutdown();
        return ERROR_GRAPHICS_CREATE_GLRC;
    }
    
    if(!wglMakeCurrent(hDC, hRC)){
        MessageBox(Window::hWnd, "Failed to activate the OpenGL device context.", NULL, MB_OK | MB_ICONERROR);
        Shutdown();
        return ERROR_GRAPHICS_ACTIVATE_GLRC;
    }
    
    BOOL (WINAPI *wglSwapIntervalEXT)(int) = (BOOL (WINAPI *)(int)) wglGetProcAddress("wglSwapIntervalEXT");
    if(wglSwapIntervalEXT) wglSwapIntervalEXT(1);
    int (WINAPI *wglGetSwapIntervalEXT)(void) = (int (WINAPI *)(void)) wglGetProcAddress("wglGetSwapIntervalEXT");
    if(wglGetSwapIntervalEXT) wglGetSwapIntervalEXT(); //Seems necessary on some cards
    
    int result = InitGL();
    if(result != 0){
        Shutdown();
        return ERROR_GRAPHICS_INIT_GLSCENE | result;
    }
    
    ResizeViewport(Window::Width, Window::Height);
    return 0;
}

int InitGL(){
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_RESCALE_NORMAL);
    glDisable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    return 0;
}

void Shutdown(){
    if(Graphics::hRC){
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(Graphics::hRC);
        Graphics::hRC = NULL;
    }
    if(Graphics::hDC){
        ReleaseDC(Window::hWnd, Graphics::hDC);
        Graphics::hDC = NULL;
    }
}

}