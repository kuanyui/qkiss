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

#include "abstractarchiveprocessor.h"
#include "logging.h"
#include <QDir>

AbstractArchiveProcessor::AbstractArchiveProcessor(QObject *parent)
    : QObject{parent}
{
    instanceCount++;
    qkDebug << "ctor of AbstractArchiveProcessor" << instanceCount;
}


AbstractArchiveProcessor::~AbstractArchiveProcessor() {
    instanceCount--;
    qkDebug << "dtor of AbstractArchiveProcessor" << instanceCount;
}

QString AbstractArchiveProcessor::generateTempDirPath()
{
    return QDir(QDir::tempPath()).filePath("qtkiss-extracted-XXXXXX");
}

int AbstractArchiveProcessor::instanceCount = 0;
