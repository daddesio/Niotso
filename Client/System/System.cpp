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

namespace System {

HINSTANCE hInst = NULL;
HANDLE Process;
HANDLE ProcessHeap;
LARGE_INTEGER ClockFreq;
volatile float FramePeriod;
UserInput_t UserInput = {0};
volatile UserInput_t UserInput_v;
bool SceneFailed = false;

int Initialize(){
    QueryPerformanceFrequency(&ClockFreq);
    
    DEVMODE dm;
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
    System::FramePeriod = 1.0f/dm.dmDisplayFrequency;
    
    if(FT_Init_FreeType(&Graphics::FreeTypeLibrary)){
        MessageBox(Window::hWnd, "Failed to initialize FreeType.", NULL, MB_OK | MB_ICONERROR);
        Graphics::FreeTypeLibrary = NULL;
        Shutdown();
        return ERROR_SYSTEM_INIT_FREETYPE;
    };
    if(FT_New_Face(Graphics::FreeTypeLibrary, "simdialogue-uni-game.ttf", 0, &Graphics::FontFace)){
        MessageBox(Window::hWnd, "simdialogue-uni-game.ttf does not exist or is corrupt or invalid.",
            NULL, MB_OK | MB_ICONERROR);
        Graphics::FontFace = NULL;
        Shutdown();
        return ERROR_SYSTEM_MISSING_FONT;
    }

    return 0;
}

void Shutdown(){
    if(Graphics::FontFace){
        FT_Done_Face(Graphics::FontFace);
        Graphics::FontFace = NULL;
    }
    if(Graphics::FreeTypeLibrary){
        FT_Done_FreeType(Graphics::FreeTypeLibrary);
        Graphics::FreeTypeLibrary = NULL;
    }
}

}