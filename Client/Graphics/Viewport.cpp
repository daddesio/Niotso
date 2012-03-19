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

void Graphics::ResizeViewport(unsigned width, unsigned height){
	if(width == 0) width++;
	if(height == 0) height++;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, width, 0, height, 0.0, 1.0);
}