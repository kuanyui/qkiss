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

#include "zlabel.h"
#include <QEvent>


ZLabel::ZLabel(std::function<QString ()> textGetter, QWidget *parent) :
    QLabel(parent)
{
    setText(textGetter);
}

void ZLabel::setText(std::function<QString ()> textGetter) {
    m_textGetter = std::move(textGetter);
    m_isDynamic = true;
    zUpdateText();
}

void ZLabel::setText(const QString &text)
{
    m_isDynamic = false;
    m_textGetter = nullptr;
    QLabel::setText(text);
}

void ZLabel::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange && m_isDynamic) {
        zUpdateText();
    }
    QLabel::changeEvent(event);
}

void ZLabel::zUpdateText() {
    if (m_isDynamic) {
        QLabel::setText(m_textGetter());
    }
}
