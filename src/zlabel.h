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

#ifndef ZLABEL_H
#define ZLABEL_H

#include <QLabel>
#include <QObject>

class ZLabel : public QLabel
{
    Q_OBJECT
public:
    using QLabel::QLabel;
    ZLabel(std::function<QString()> textGetter, QWidget* parent = nullptr);
    void setText(std::function<QString()> textGetter);
    void setText(const QString &text);

protected:
    void changeEvent(QEvent* event) override;
    void zUpdateText();

private:
    std::function<QString()> m_textGetter = nullptr;
    bool m_isDynamic = false;
};

#endif // ZLABEL_H
