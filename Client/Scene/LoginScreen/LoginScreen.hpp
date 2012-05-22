/*
    Niotso - The New Implementation of The Sims Online
    Scene/LoginScreen/LoginScreen.hpp
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

class LoginScreen : public Scene {
    enum {
        IMG_COPYRIGHT,
        IMG_STATUS, //value = 1..9
        IMG_COUNT = 10
    };

    enum {
        TEX_EAGAMES,
        TEX_MAXIS,
        TEX_SETUP,
        TEX_COPYRIGHT,
        TEX_STATUS, //value = 4..12
        TEX_COUNT = 13
    };

    enum {
        SND_LOADLOOP,
        SND_COUNT
    };

    enum { Screen_EAGames, Screen_Maxis, Screen_Setup } Screen;
    float Time;
    float ScrollPos;
    Image_t * image[IMG_COUNT];
    GLuint texture[TEX_COUNT];
    PlayableSound_t * sound[SND_COUNT];

  public:
    LoginScreen();
    ~LoginScreen();
    void Render();

  private:
    int Run(float TimeDelta);
};