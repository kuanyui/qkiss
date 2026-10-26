/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2026 ono ono (kuanyui) <azazabc123@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#version 330 core

// Draws one triangle that covers the whole viewport, without any vertex data. glDrawArrays(GL_TRIANGLES, 0, 3) runs
// this 3 times, for the corners (-1, -1), (3, -1) and (-1, 3). OpenGL cuts off the part outside the viewport.
void main()
{
    vec2 corner = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);   // (0, 0), (2, 0), (0, 2)
    gl_Position = vec4(corner * 2.0 - 1.0, 0.0, 1.0);
}
