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

#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include "ztreewidget.h"

class LogViewer : public QWidget
{
    Q_OBJECT
public:
    explicit LogViewer(QWidget *parent = nullptr);
    void appendLog(QtMsgType type, const QString &msg);
    void clearAllLogs();
signals:

private:
    void setupLayout();
    ZTreeWidget *m_treeWidget;
};

#endif // LOGVIEWER_H
