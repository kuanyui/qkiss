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

#ifndef DOCKWIDGETPROXYSTYLE_H
#define DOCKWIDGETPROXYSTYLE_H
#include <QProxyStyle>
#include <QStyleOptionTitleBar>
#include <QPainter>
#include <QDockWidget>

class DockWidgetProxyStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;
    virtual void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
};

#endif // DOCKWIDGETPROXYSTYLE_H
