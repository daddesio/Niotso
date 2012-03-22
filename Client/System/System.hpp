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

//System/System.cpp
namespace System {
    int Initialize();
    extern bool Shutdown;
    extern HINSTANCE hInst;
    extern HANDLE Process;
    extern HANDLE ProcessHeap;
    extern LARGE_INTEGER ClockFreq;
    extern float FramePeriod;
    extern bool Keys[256];
    
    struct UserInput_t {
        bool Keys[256];
        bool MouseDown;
        bool CloseWindow;
    };
    extern UserInput_t UserInput;
    
    //Constants
    enum {
        SHUTDOWN = 0
    };
}

/****
** Program exit codes
** (Return 0 for success)
*/

#define ERROR_INIT 0x1000
#define ERROR_INIT_ANOTHERINSTRUNNING 0x1001

//Engine errors
//Use: return ERROR_INIT | result;
#define ERROR_INIT_WINDOW 0x0100
    enum {
        ERROR_REGISTER_CLASS = 1,
        ERROR_CREATE_WINDOW
    };
#define ERROR_INIT_SYSTEM 0x0200
#define ERROR_INIT_GRAPHICS 0x0300
    enum {
        ERROR_GRAPHICS_OBTAIN_DC = 1,
        ERROR_GRAPHICS_FIND_PIXELFORMAT,
        ERROR_GRAPHICS_SET_PIXELFORMAT,
        ERROR_GRAPHICS_CREATE_GLRC,
        ERROR_GRAPHICS_ACTIVATE_GLRC,
        ERROR_GRAPHICS_INIT_GLSCENE
    };
#define ERROR_INIT_AUDIO 0x0400
    enum {
        ERROR_AUDIO_INIT_COM = 1,
        ERROR_AUDIO_INIT_XAUDIO2,
        ERROR_AUDIO_CREATE_VOICE
    };
#define ERROR_INIT_LOGIC 0x0500
    enum {
        ERROR_LOGIC_CREATE_SCENE = 1
    };