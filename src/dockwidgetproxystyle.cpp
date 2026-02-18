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

#include "dockwidgetproxystyle.h"

void DockWidgetProxyStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::CE_DockWidgetTitle) {
        const QStyleOptionDockWidget *opt = qstyleoption_cast<const QStyleOptionDockWidget *>(option);
        if (!opt) { goto fallback; }
        const QDockWidget *dockWidget = qobject_cast<const QDockWidget *>(widget);
        if (!dockWidget) { goto fallback; }
        QIcon icon = dockWidget->toggleViewAction()->icon();
        if (icon.isNull()) {
            icon = dockWidget->windowIcon();  // use windowIcon as icon
        }
        if (!icon.isNull()) {
            // Draw icon
            int iconSize = 16;
            int margin = 4;
            // (NOTE: CE_DockWidgetTitle's rect already includes the whold title area.)
            QRect iconRect(opt->rect.left() + margin,
                           opt->rect.center().y() - iconSize / 2,
                           iconSize, iconSize);

            icon.paint(painter, iconRect);

            QStyleOptionDockWidget modifiedOpt = *opt;
            modifiedOpt.rect.setLeft(modifiedOpt.rect.left() + iconSize + margin);

            QProxyStyle::drawControl(element, &modifiedOpt, painter, widget);
            return;
        }
    }
    // Draw remain parts (do original behaviors)
    fallback:
    QProxyStyle::drawControl(element, option, painter, widget);
}
