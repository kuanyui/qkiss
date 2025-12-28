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

const ThemeManager::GeneralTextColorPalette &ThemeManager::generalTextColorPalette() const
{
    return themeType() == ThemeType::Light ? m_lightTextPlt : m_darkTextPlt;
}

const ThemeManager::SyntaxHighlightFormatPalette &ThemeManager::syntaxHighlightFormatPalette() const
{
    return themeType() == ThemeType::Light ? m_lightSyntaxPlt : m_darkSyntaxPlt;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject{parent}
{
    // ====================================================================
    // Light
    // ====================================================================

    m_lightTextPlt.magenta = QColor::fromString("#ff1f8b"_L1);
    m_lightTextPlt.red = QColor::fromString("#cc0000"_L1);
    m_lightTextPlt.orange = QColor::fromString("#ff5d17"_L1);
    m_lightTextPlt.yellow = QColor::fromString("#c4a000"_L1);
    m_lightTextPlt.green = QColor::fromString("#008700"_L1);
    m_lightTextPlt.cyan = QColor::fromString("#18b2b2"_L1);
    m_lightTextPlt.blue = QColor::fromString("#005faf"_L1);
    m_lightTextPlt.purple = QColor::fromString("#b218b2"_L1);
    m_lightTextPlt.gray = QColor::fromString("#b2b2b2"_L1);

    m_lightSyntaxPlt.magenta.setForeground(m_lightTextPlt.magenta);
    m_lightSyntaxPlt.red.setForeground(m_lightTextPlt.red);
    m_lightSyntaxPlt.orange.setForeground(m_lightTextPlt.orange);
    m_lightSyntaxPlt.yellow.setForeground(m_lightTextPlt.yellow);
    m_lightSyntaxPlt.green.setForeground(m_lightTextPlt.green);
    m_lightSyntaxPlt.cyan.setForeground(m_lightTextPlt.cyan);
    m_lightSyntaxPlt.blue.setForeground(m_lightTextPlt.blue);
    m_lightSyntaxPlt.purple.setForeground(m_lightTextPlt.purple);
    m_lightSyntaxPlt.gray.setForeground(m_lightTextPlt.gray);
    m_lightSyntaxPlt.curLineHlBg = QColor::fromString("#e7e7df"_L1);
    m_lightSyntaxPlt.lineNumBg = QColor::fromString("#d7d7af"_L1);
    m_lightSyntaxPlt.lineNumFg = QColor::fromString("#87875f"_L1);

    // ====================================================================
    // Dark
    // ====================================================================

    m_darkTextPlt.magenta = QColor::fromString("#ff4ea3"_L1);
    m_darkTextPlt.red = QColor::fromString("#ef2929"_L1);
    m_darkTextPlt.orange = QColor::fromString("#ff8700"_L1);
    m_darkTextPlt.yellow = QColor::fromString("#ffd700"_L1);
    m_darkTextPlt.green = QColor::fromString("#afff00"_L1);
    m_darkTextPlt.cyan = QColor::fromString("#87ffff"_L1);
    m_darkTextPlt.blue = QColor::fromString("#5fafd7"_L1);
    m_darkTextPlt.purple = QColor::fromString("#af5fff"_L1);
    m_darkTextPlt.gray = QColor::fromString("#767676"_L1);

    m_darkSyntaxPlt.magenta.setForeground(m_darkTextPlt.magenta);
    m_darkSyntaxPlt.red.setForeground(m_darkTextPlt.red);
    m_darkSyntaxPlt.orange.setForeground(m_darkTextPlt.orange);
    m_darkSyntaxPlt.yellow.setForeground(m_darkTextPlt.yellow);
    m_darkSyntaxPlt.green.setForeground(m_darkTextPlt.green);
    m_darkSyntaxPlt.cyan.setForeground(m_darkTextPlt.cyan);
    m_darkSyntaxPlt.blue.setForeground(m_darkTextPlt.blue);
    m_darkSyntaxPlt.purple.setForeground(m_darkTextPlt.purple);
    m_darkSyntaxPlt.gray.setForeground(m_darkTextPlt.gray);
    m_darkSyntaxPlt.curLineHlBg = QColor::fromString("#3a3a3a"_L1);
    m_darkSyntaxPlt.lineNumBg = QColor::fromString("#4e4e4e"_L1);
    m_darkSyntaxPlt.lineNumFg = QColor::fromString("#c6c6c6"_L1);

}

ThemeManager::~ThemeManager()
{

}
