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

#ifndef GKISSSCENE_H
#define GKISSSCENE_H

#include <QGraphicsScene>
#include <QHash>
#include <QVector>
#include "archivefile.h"
#include "gkissitem.h"

class GKissScene : public QGraphicsScene
{
    Q_OBJECT   // For qobject_cast
public:
    explicit GKissScene (ArchiveFile *archiveFile, ParsedCnf *cnf, int setIdx, QObject *parent = nullptr);
    // explicit GKissScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);
    ~GKissScene();
    std::optional<ArchiveFile *> archiveFile() const;
    void drawCurSetBackground();
    void addCelItem(GKissItem *item);
    std::optional<QVector<GKissItem *> *> getItemsByObjId(int32_t objId) const;
    std::optional<GKissItem *> getItemByCelName(const QString &celFileName);
    void setItemsVisibility(const FkissVariant &target, bool visibility);
    void setItemVisibilityByCelName(const QString &celFileName, bool visibility);
    void setItemsVisibilityByObjId(int32_t objId, bool visibility);
    /// toggle the visibility of each item separately, for `altmap()`
    void toggleItemsVisibility(const FkissVariant &target);
    void toggleItemVisibilityByCelName(const QString &celFileName);
    void toggleItemsVisibilityByObjId(int32_t objId);

    void bindItemsToObjects();
    /// Get object's "current" position, instead of the position defined in CNF's set.
    std::optional<QPoint > getObjectCurrentPos(size_t objId) const;
    /// moveto() in FKiSS2
    bool moveObjectTo(size_t objId, int x, int y);
    /// move() in FKiSS1
    bool moveObject(size_t objId, int dx, int dy);
    /// movebyx() in FKiSS2
    bool moveObjectByX(size_t targetObjId, size_t anchorObjId, int dx);
    /// movebyy() in FKiSS2
    bool moveObjectByY(size_t targetObjId, size_t anchorObjId, int dy);
    void executeTransparent(const FkissVariant &target, int delta);
    void executeTransparentOnCel(QString celFileName, int delta);
    void executeTransparentOnObj(int32_t objId, int delta);
    void receiveFkissUserEventFromSceneItem(FkissUserMovementEvent evtType, QVector<FkissVariant> vars);
    QList<GKissItem *> allKissItems();
    /// `${PALETTE_NO_IN_KCF} ...` in CNF by default, can be switched by user.
    uint8_t paletteNoInKcf() const;
    void setPaletteNoInKcfAndRender(uint8_t newPaletteNoInKcf);
    void unfixObjectByItem(GKissItem *item);   /// REFACTOR: Move to GKissItem?
    void resetObjectByItem(GKissItem *item);
signals:
    /// The Item which is being pressed / dragged... etc.
    void highlightedItemChanged(GKissItem *item);

protected:
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    /// pass nullptr means clear
    void storeCurrentOverlappedItems(GKissItem *targetItem);
    /// For collide(), apart(): the items whose pixels overlap each of `targetItems`, in the same order.
    QList<QSet<GKissItem *>> getPixelOverlappedItems(const QVector<GKissItem *> &targetItems) const;
    /// For in(), out(): the other objects which have a cel whose bounding rectangle overlaps any of `targetItems`.
    QSet<int> getBoundingRectOverlappedObjIds(int32_t objId, const QVector<GKissItem *> &targetItems);
    void emitCollideEvents(GKissItem *movedItem, const QSet<GKissItem *> &originalPixelOverlappedItems, const QSet<GKissItem *> &currentPixelOverlappedItems);
    void emitApartEvents(GKissItem *movedItem, const QSet<GKissItem *> &originalPixelOverlappedItems, const QSet<GKissItem *> &currentPixelOverlappedItems);
    void emitBoundingRectOverlapEvents(int32_t objId, const QSet<int> &originalOverlappedObjIds, const QSet<int> &currentOverlappedObjIds, const QSet<int> &currentNonOverlappedObjIds);
    /// FKiSS movement triggers apart(), in(), stillin(), out(), stillout(), collide() in this order, before the next
    /// FKiSS action runs. See "FKiSS Event Order" (vendor/references/otakuworld.com/kiss/kisshelp/DIRECTKISSFKiSS_Event_Order.htm).
    /// mouseReleaseEvent() follows its section "User movement". This follows the section above it, "FKiSS movement":
    ///     "When an FKiSS action causes an object to move, the following sequence occurs:"
    ///         => move(), moveto(), movebyx(), movebyy() trigger these events too, not only the mouse release.
    ///     "FKiSS actions continue executing after the FKiSS action which started this sequence"
    ///         => These events run immediately, not queued.
    /// Not implemented: the first step of that section, the collide() check before movement. The page doesn't say what to
    /// compare that position with.
    void emitFkissMovementEvents(int32_t objId, const QVector<GKissItem *> &movedItems, const QList<QSet<GKissItem *>> &originalPixelOverlappedItems, const QSet<int> &originalBoundingRectOverlappedObjIds);
    QHash<int32_t, QVector<GKissItem*>*> m_objIdToItemsMap;
    QHash<CPString, GKissItem *> m_celNameToItemMap;
    ArchiveFile *m_archiveFile;
    /// Each set will create a new scene. This is the setIndex (setNo, counted from 0) of this scene.
    int m_setIndex;
    ParsedCnf* m_cnf;
    ParsedCnfSet *m_cnfSet;
    /// Read from CNF file ($), and can be overritten from UI.
    uint8_t m_paletteNoInKcf = 0;
    // ==========================================================================
    // Interactive
    // ==========================================================================
    /// For Press / Drag / Release
    GKissItem *m_draggingItem = nullptr;
    QPointF m_dragStartMouseScenePos;
    /// catch(), fixcatch(), unfix() should be emitted only once in a continuous mouseMoveEvent(s).
    bool m_dragRelatedFkissEventsAreAlreadyEmitted = false;
    /// According to FKiSS spec, `fix` can be decreased by mouse-move.
    /// However, I feel this is too annoying, so makes mouse-click (press + release) also able to decrease `fix`.
    bool m_fixDecreasedWhileDraggingItem = false;
    /// @borrowed
    /// For collide(), apart()
    QSet<GKissItem*> m_originalPixelOverlappedItems;
    /// @borrowed
    /// For in(), stillin(), out(), stillout()
    // QSet<GKissItem*> m_originalBoundingRectOverlappedItems;
    QSet<int> m_originalBoundingRectOverlappedObjIds;
    /// force drag cel instead of object.
    bool m_draggingWithCtrl = false;
    /// pause any fkiss events triggered by user press/drag.
    bool m_draggingWithShift = false;
    /// FKiSS movement events can move objects again, so they can nest. Some script may do this forever.
    int m_fkissMovementDepth = 0;
    static constexpr int MAX_FKISS_MOVEMENT_DEPTH = 16;
};

#endif // GKISSSCENE_H
