/*
    Niotso - The New Implementation of The Sims Online
    Audio/Audio.hpp
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

#include "windows/xaudio2.hpp"
#ifdef PlaySound //defined by the Windows API
 #undef PlaySound
#endif

struct PlayableSound_t {
    bool Playing;
    uint8_t * Data;
    IXAudio2SourceVoice* pSourceVoice;
};

namespace Audio {
    int Initialize();
    PlayableSound_t * LoadSound(const Sound_t * Sound);
    bool PlaySound(PlayableSound_t * Sound);
    bool StopSound(PlayableSound_t * Sound);
    void DeleteSound(PlayableSound_t * Sound);
    void Shutdown();
}