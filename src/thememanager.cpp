/**************************************************************************
* QKiSS - A modern implementation of KISS (KISekae Set system) viewer in Qt 6.
*
* Copyright (C) 2024 ono ono (kuanyui) <azazabc123@gmail.com>
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
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>

#include "thememanager.h"

using namespace Qt::Literals::StringLiterals;

ThemeManager &ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeType ThemeManager::themeType() const
{
    // OPTIMIZE: Cache this value
    // Fucking useless. Use another way.
    // switch (QGuiApplication::styleHints()->colorScheme()) {
    // case Qt::ColorScheme::Dark: return ThemeType::Dark;
    // case Qt::ColorScheme::Light: return ThemeType::Light ;
    // default: return ThemeType::Light;
    // }
    const QPalette defaultPalette;
    const QColor text = defaultPalette.color(QPalette::WindowText);
    const QColor window = defaultPalette.color(QPalette::Window);
    return text.lightness() < window.lightness() ? ThemeType::Light : ThemeType::Dark;
}

const ThemeManager::SyntaxFormat &ThemeManager::syntaxFormatPalette() const
{
    return themeType() == ThemeType::Light ? m_pltLight : m_pltDark;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject{parent}
{
    qDebug() << "ThemeManager ctor()";
    m_pltDark.magenta.setForeground(QColor::fromString("#ff4ea3"_L1));
    m_pltDark.red.setForeground(QColor::fromString("#ef2929"_L1));
    m_pltDark.orange.setForeground(QColor::fromString("#ff8700"_L1));
    m_pltDark.yellow.setForeground(QColor::fromString("#ffd700"_L1));
    m_pltDark.green.setForeground(QColor::fromString("#afff00"_L1));
    m_pltDark.cyan.setForeground(QColor::fromString("#87ffff"_L1));
    m_pltDark.blue.setForeground(QColor::fromString("#5fafd7"_L1));
    m_pltDark.purple.setForeground(QColor::fromString("#af5fff"_L1));
    m_pltDark.gray.setForeground(QColor::fromString("#767676"_L1));
    m_pltDark.curLineHlBg = QColor::fromString("#3a3a3a"_L1);
    m_pltDark.lineNumBg = QColor::fromString("#4e4e4e"_L1);
    m_pltDark.lineNumFg = QColor::fromString("#c6c6c6"_L1);

    m_pltLight.magenta.setForeground(QColor::fromString("#ff1f8b"_L1));
    m_pltLight.red.setForeground(QColor::fromString("#cc0000"_L1));
    m_pltLight.orange.setForeground(QColor::fromString("#ff5d17"_L1));
    m_pltLight.yellow.setForeground(QColor::fromString("#c4a000"_L1));
    m_pltLight.green.setForeground(QColor::fromString("#008700"_L1));
    m_pltLight.cyan.setForeground(QColor::fromString("#18b2b2"_L1));
    m_pltLight.blue.setForeground(QColor::fromString("#005faf"_L1));
    m_pltLight.purple.setForeground(QColor::fromString("#b218b2"_L1));
    m_pltLight.gray.setForeground(QColor::fromString("#b2b2b2"_L1));
    m_pltLight.curLineHlBg = QColor::fromString("#3a3a3a"_L1);
    m_pltLight.lineNumBg = QColor::fromString("#d7d7af"_L1);
    m_pltLight.lineNumFg = QColor::fromString("#87875f"_L1);
}

ThemeManager::~ThemeManager()
{

}
