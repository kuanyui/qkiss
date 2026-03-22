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

#include "ztreewidget.h"
#include <QEvent>
#include <QHelpEvent>
#include <QToolTip>


ZTreeWidget::ZTreeWidget(QWidget *parent) :
    QTreeWidget{parent}
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeWidget::customContextMenuRequested,
            this, &ZTreeWidget::showContextMenu);
}

bool ZTreeWidget::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent*>(event);
        QModelIndex index = indexAt(helpEvent->pos());
        if (index.isValid()) {
            QString text = index.data(Qt::DisplayRole).toString();
            QToolTip::showText(helpEvent->globalPos(), text, this);
            return true;
        }
    }
    return QTreeWidget::viewportEvent(event);
}

void ZTreeWidget::showContextMenu(const QPoint &pos) {
    QModelIndex index = indexAt(pos);
    if (!index.isValid()) { return; }
    QString text = index.data(Qt::DisplayRole).toString();
    if (text.isEmpty()) { return; }
    QMenu menu{this};
    static const char* strCopy { "&Copy" };  // Workaround to avoid being caught by `lupdate`, because `lupdate` cannot analyze dynamic string.
    QAction *copyAction = menu.addAction(QCoreApplication::tr(strCopy));
    copyAction->setIcon(QIcon::fromTheme("edit-copy"));
    QAction *selected = menu.exec(viewport()->mapToGlobal(pos));
    if (selected == copyAction) {
        QClipboard *cb = QGuiApplication::clipboard();
        cb->setText(text);
    }
}
