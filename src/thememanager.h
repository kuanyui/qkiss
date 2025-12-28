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
#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QSyntaxHighlighter>


class ThemeManager : public QObject
{
    Q_OBJECT
    struct GeneralTextColorPalette {
        QColor magenta;
        QColor red;
        QColor orange;
        QColor yellow;
        QColor green;
        QColor cyan;
        QColor blue;
        QColor purple;
        QColor gray;
    };
    struct SyntaxHighlightFormatPalette {
        QTextCharFormat magenta;
        QTextCharFormat red;
        QTextCharFormat orange;
        QTextCharFormat yellow;
        QTextCharFormat green;
        QTextCharFormat cyan;
        QTextCharFormat blue;
        QTextCharFormat purple;
        QTextCharFormat gray;
        QColor curLineHlBg;
        QColor lineNumBg;
        QColor lineNumFg;
    };
public:
    enum class ThemeType {
        Light,
        Dark
    };
    static ThemeManager &instance();
    ThemeType themeType() const;
    const GeneralTextColorPalette &generalTextColorPalette() const;
    const SyntaxHighlightFormatPalette &syntaxHighlightFormatPalette() const;

signals:

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager();
    GeneralTextColorPalette m_darkTextPlt;
    GeneralTextColorPalette m_lightTextPlt;
    SyntaxHighlightFormatPalette m_darkSyntaxPlt;
    SyntaxHighlightFormatPalette m_lightSyntaxPlt;
};

#endif // THEMEMANAGER_H
