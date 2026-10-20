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

#ifndef ITEMINSPECTOR_H
#define ITEMINSPECTOR_H

#include <QObject>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>


class GKissItem;

class ItemInspector : public QWidget
{
    Q_OBJECT
public:
    explicit ItemInspector(QWidget *parent = nullptr);
    void setInspectedItem(GKissItem *item);
signals:

private:
    void appendKvRow(QLabel **item, const QString &icon, std::function<QString()> &&titleGetter);
    GKissItem *m_curItem;
    QGridLayout *m_gridLayout;
    QLabel *m_twFix;
    QLabel *m_twObjId;
    QLabel *m_twCelPos;
    QLabel *m_twCelName;
    QLabel *m_twCelSize;
    QLabel *m_twCelColorDepth;
    QLabel *m_twCelComment;
    QLabel *m_twCelSiblings;
    QLabel *m_twCelPreview;

    // QWidget interface
protected:
};

#endif // ITEMINSPECTOR_H
