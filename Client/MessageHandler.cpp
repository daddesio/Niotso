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

/****
    For the sake of performance **all throughout the game**,
    order the cases in order from most frequently occurred to less frequently occurred
    except making adjustments where tiny response latency is essential
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_SETCURSOR: //Occurs often
        SetCursor(NULL);
        break;
    
    case WM_SYSCOMMAND: //Occurs often
        switch(wParam){
        case SC_MONITORPOWER:
        case SC_SCREENSAVE:
            return 0;
        }
        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
        System::UserInput.Keys[wParam] = (uMsg == WM_KEYDOWN);
        return 0;
    
    case WM_CLOSE:
        System::UserInput.CloseWindow = true;
        PostQuitMessage(0);
        return 0;

    case WM_DEVMODECHANGE: {
        DEVMODE dm;
        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
        System::FramePeriod = 1.0f/dm.dmDisplayFrequency;
    } return 0;

    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}