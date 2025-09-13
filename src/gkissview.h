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

#ifndef GKISSVIEW_H
#define GKISSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QObject>
#include <QWidget>
#include "gkissscene.h"
#include "gkissitem.h"

class GKissView : public QGraphicsView
{
    Q_OBJECT;
public:
    GKissView(QWidget *parent=nullptr);
    GKissView(ArchiveFile *archiveFile, QWidget *parent=nullptr);
    bool setArchiveFileAndRenderFirstCnf(ArchiveFile *archiveFile);
    void deleteAllScenes();
    bool setCurCnfAndRenderScenes(CPString cnfFileName);
    uint8_t scenesAmount() const;
    bool showKissSceneByIndex(uint8_t setIndex);
    void scaleAbs(qreal targetScale);
    void setScaleModeForAllItems();
    GKissScene *curKissScene();
    /** create scenes & items according to current CNF. */
    bool renderCurCnfAllSets();
    // bool renderCurCnfSet(uint8_t setNo);

signals:
    /// e.g. hovered item changed, no longer hovering any item (pass nullptr)... etc.
    void highlightedItemChanged(GKissItem *item);
    /// 1 means go right-ward; -1 means left.
    void requestSwitchingSet(int steps);

protected:

    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
private:
    ArchiveFile *m_archiveFile = nullptr;
    /// Each set is stored in its scene.
    QVector<GKissScene *> m_scenes;
    /// REFACTOR: Wait... Why not just use `ArchiveFile::curCnfSetNo` ???
    uint8_t m_curSceneIdx = 0;
    GKissItem *m_curHoveredCelItem;
    QList<QMetaObject::Connection> m_connectionsOfArchiveFile;

};

#endif // GKISSVIEW_H
