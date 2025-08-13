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

#ifndef ZGROUPBOX_H
#define ZGROUPBOX_H

#include <QGroupBox>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ZGroupBox : public QGroupBox
{
public:
    explicit ZGroupBox(const QString &title, const QIcon &icon, QWidget *parent = nullptr);
    explicit ZGroupBox(const QString &title, QWidget *parent = nullptr);
    void setIcon(const QIcon &icon);
    void setContentLayout(QLayout *layout);
    template<typename T = QLayout> T *contentLayout();

private:
    QLabel *m_iconLabel;
    QHBoxLayout *m_rootLayout;
    QLayout *m_contentLayout;
    QIcon m_icon;
    int m_iconSize = 48;
};

#endif // ZGROUPBOX_H
