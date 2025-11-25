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

#ifndef ZTREEWIDGET_H
#define ZTREEWIDGET_H

#include <QTreeWidget>
#include <QApplication>
#include <QMenu>
#include <QClipboard>

class ZTreeWidget : public QTreeWidget
{
    using QTreeWidget::QTreeWidget;
public:
    ZTreeWidget(QWidget *parent = nullptr);
    /// Override `QAbstractItemView::viewportEvent` instead of `QWidget::event`;
    /// Quotes from Qt official doc: `This function is used to handle tool tips, and What's This? mode, if the given event is a QEvent::ToolTip,or a QEvent::WhatsThis. It passes all other events on to its base class viewportEvent() handler.`
    virtual bool viewportEvent(QEvent *event) override;

private:
    void showContextMenu(const QPoint &pos);
};

#endif // ZTREEWIDGET_H
