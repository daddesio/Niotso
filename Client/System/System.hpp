/*
    Niotso - The New Implementation of The Sims Online
    System/System.hpp
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

namespace System {
    int Initialize();
    void Shutdown();

    extern HINSTANCE hInst;
    extern LARGE_INTEGER ClockFreq;
    extern volatile float FramePeriod;

    struct UserInput_t {
        bool Keys[256];
        bool MouseDown;
        bool CloseWindow;
    };
    extern UserInput_t UserInput;
    extern volatile UserInput_t UserInput_v;

    extern bool SceneFailed;

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
        ERROR_WINDOW_SYNCOBJECT = 1,
        ERROR_WINDOW_CREATE_THREAD,
        ERROR_WINDOW_SYNCHRONIZE,
        ERROR_WINDOW_REGISTER_CLASS,
        ERROR_WINDOW_CREATE,
        ERROR_WINDOW_HANDLE
    };
#define ERROR_INIT_SYSTEM 0x0200
    enum {
        ERROR_SYSTEM_INIT_FREETYPE = 1,
        ERROR_SYSTEM_MISSING_FONT
    };
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