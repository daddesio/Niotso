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

#define WINVER 0x0502
#define _WIN32_WINNT 0x0502
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#undef NULL
#define NULL 0

#include "version.h"
#include "System/System.hpp"
#include "Resources/Resource.h"

#include "Audio/Audio.hpp"
#include "Graphics/Graphics.hpp"
#include "Scene/Scene.hpp"

//IsometricEngine.cpp
namespace Window {
    extern unsigned Width, Height;
    extern bool Fullscreen;
    extern HWND hWnd;
}

//MessageHandler.cpp
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);