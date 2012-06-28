/*
    Niotso - The New Implementation of The Sims Online
    EngineInterface.hpp
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

//Compiler/platform constants
#ifdef _WIN32
 #define WINVER 0x0502
 #define _WIN32_WINNT 0x0502
 #define NTDDI_VERSION 0x05010300
 #define _CRT_SECURE_NO_WARNINGS
#endif

//Standard libraries
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef _WIN32
 #define WIN32_LEAN_AND_MEAN
 #include <windows.h>
 #undef NULL
 #define NULL 0
#endif

//Codebase libraries
#include "FileHandler.hpp"
#include "ft2build.h"
#include FT_FREETYPE_H

//Macro definitions
#ifndef min
 #define min(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef max
 #define max(x,y) ((x)>(y)?(x):(y))
#endif

//Codebase version, branding, linker resources
#include "version.h"
#include "Resources/Resource.h"

//Components
#include "Audio/Audio.hpp"
#include "Graphics/Graphics.hpp"
#include "System/System.hpp"
#include "Window/Window.hpp"

#include "Scene/Scene.hpp"