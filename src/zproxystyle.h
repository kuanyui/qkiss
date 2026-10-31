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

#ifndef ZPROXYSTYLE_H
#define ZPROXYSTYLE_H
#include <QProxyStyle>
#include <QStyleOptionTitleBar>
#include <QPainter>
#include <QDockWidget>

/**
 * A `QProxyStyle` to override some default styles (sizes, icons...etc) of Qt Application.
 * (Usually, a Qt application should has only 1 `QProxStyle`)
 *
 * # Usage
 * ```cpp
 * QApplication app;
 * app.setStyle(new ZProxyStyle{});
 * ```
 *
 * # What does the "Proxy" mean?
 * When rendering a `QWidget`, then renderer will query the style according to the following ordering:
 *  1. QStyleSheetStyle    (<- QSS has the highest priority)
 *  2. QProxyStyle     (<- inherited by us, as a "proxy", to "intercept" the style query before the system style)
 *  3. QFusionStyle (GNOME) / Breeze:Style (KDE) / QWindows{,Vista,11}Style / QMacStyle     (<- according to the system, or overwritten via option `-style` or envvar `QT_STYLE_OVERRIDE`)
 *
 * # About MacOS
 * The default styles under MacOS have too many differences from other systems, so try to unify them:
 * - QDockWidget: Move the close button to right side (to avoid conflicting with our own added icon on title bar)
 * - QToolBar: Decrease the terribly large 32px icons to 24px
 * - QDialogButtonBox: Enable icons.
 *
 * > ATTENTION: Remember to enable icon displaying on QMenu via
 * > ```cpp
 * > QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);
 * > ```
 */
class ZProxyStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;
    /// Draws the icon of a dock widget on the left of its title. The icon comes from `toggleViewAction()`.
    /// When the action has no icon, the window icon is used.
    /// With neither icon, the base style draws the whole title bar.
    virtual void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
#ifdef Q_OS_MACOS
    /// Moves the close button of QDockWidget to the right of the title bar (with RTL support), to avoid conflicting with our own added icon on title bar
    virtual QRect subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const override;
    /// Returns the value of `QCommonStyle` for `PM_ToolBarIconSize`, which is 24 on macOS. The macOS style returns 32.
    virtual int pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override;
    /// Enables icons on the buttons of `QDialogButtonBox` (OK, Cancel, Apply...).
    virtual int styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const override;
#endif
private:
    /// Returns the largest of these sizes that is smaller than the height of the font: 48, 32, 24, 22, 16.
    /// Returns 16 when none of them is smaller.
    static int getMostFitIconSize(const QStyleOption &option);

};

#endif // ZPROXYSTYLE_H
