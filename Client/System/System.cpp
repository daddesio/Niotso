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
    float FramePeriod;
    UserInput_t UserInput;
    bool SceneFailed = false;
    
    int Initialize(){
        memset(&UserInput, 0, sizeof(UserInput));
        
        QueryPerformanceFrequency(&ClockFreq);
        
        DEVMODE dm;
        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
        System::FramePeriod = 1.0f/dm.dmDisplayFrequency;

        return 0;
    }
}