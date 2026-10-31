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

#include "zproxystyle.h"

void ZProxyStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
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
            // Draw original background & border seperately, to avoid breaking the original background & border.
            // Because a modified``QStyleOption.rect` breaks the original behavior of `QProxyStyle::drawControl()`.
            // So manually draw:
            //   - background + border
            //   - icon
            //   - text
            // separately here.

            // 1. Backgroun + Border
            QStyleOptionDockWidget bgOpt = *opt;
            bgOpt.title = "";
            QProxyStyle::drawControl(element, &bgOpt, painter, widget);

            // 2. Icon
            int iconSize = getMostFitIconSize(*opt);
            int margin = 4;
            // (NOTE: CE_DockWidgetTitle's rect already includes the whold title area.)
            QRect iconRect(opt->rect.left() + margin,
                           opt->rect.center().y() - iconSize / 2,
                           iconSize, iconSize);

            icon.paint(painter, iconRect);

            // 3. Text
            QRect textRect = opt->rect;
            textRect.setLeft(iconRect.right() + margin);
            drawItemText(painter,
                         textRect,
                         Qt::AlignLeft | Qt::AlignVCenter,
                         opt->palette,
                         opt->state & QStyle::State_Enabled,
                         opt->title,
                         QPalette::WindowText);
            return;
        }
    }
    // (do original behaviors)
    fallback:
    QProxyStyle::drawControl(element, option, painter, widget);
}

int ZProxyStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    // On MacOS and Windows, Qt shows no icons on the buttons of `QDialogButtonBox` (OK, Cancel, Apply...). Force shows them,
    if (hint == QStyle::SH_DialogButtonBox_ButtonsHaveIcons) { return 1; }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

void ZProxyStyle::polish(QApplication *app)
{
    QProxyStyle::polish(app);
    // Qt shows no icons in menus on some versions of Mac OS. Force shows them.
    // `QApplication` sets this attribute only in its constructor.
    // `QApplication::setStyle()` calls `polish()` later, so Qt does not overwrite this value.
    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);
}

#ifdef Q_OS_MACOS
QRect ZProxyStyle::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    // The macOS style puts the close & float buttons on the left of the dock title, where we draw the icon.
    // It mirrors them with `visualRect()`, so with the opposite direction they go to the right, as in other styles.
    switch (element) {
    case QStyle::SE_DockWidgetCloseButton:
    case QStyle::SE_DockWidgetFloatButton:
    case QStyle::SE_DockWidgetTitleBarText:
    case QStyle::SE_DockWidgetIcon:
        if (const QStyleOptionDockWidget *opt = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {
            QStyleOptionDockWidget mirroredOpt = *opt;
            mirroredOpt.direction = (opt->direction == Qt::RightToLeft) ? Qt::LeftToRight : Qt::RightToLeft;
            return QProxyStyle::subElementRect(element, &mirroredOpt, widget);
        }
        break;
    default:
        break;
    }
    return QProxyStyle::subElementRect(element, option, widget);
}

int ZProxyStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    // The macOS style makes toolbar icons 32x32. Fusion, the style on Windows, makes toolbar icons 24x24.
    // Fusion gets this size from `QCommonStyle`, the base class of Fusion and the macOS style.
    // So call `QCommonStyle::pixelMetric()` directly, not the one in the macOS style.
    if (metric == QStyle::PM_ToolBarIconSize) { return QCommonStyle::pixelMetric(metric, option, widget); }
    return QProxyStyle::pixelMetric(metric, option, widget);
}
#endif

int ZProxyStyle::getMostFitIconSize(const QStyleOption &option)
{
    static const QList<int> AVAILABLE_SIZE_LIST = {48, 32, 24, 22, 16};
    int textHeight = option.fontMetrics.height();
    for (const int candidate : AVAILABLE_SIZE_LIST) {
        if (candidate < textHeight) {
            return candidate;
        }
    }
    return AVAILABLE_SIZE_LIST.last();
}
