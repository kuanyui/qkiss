/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2024 - 2025 ono ono (kuanyui) <azazabc123@gmail.com>
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

// fruit.cpp
#include "enums.h"
#include <QMetaType>

// Register enum classes, so that they can be used in QVariant<T>
static bool registered = []() {
    qRegisterMetaType<QKiSS::Case>("QKiSS::Case");
    qRegisterMetaType<QKiSS::Encoding>("QKiSS::Encoding");
    qRegisterMetaType<QKiSS::LzhDecompressorBackend>("QKiSS::LzhDecompressorBackend");
    return true;
}();
