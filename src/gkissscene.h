#ifndef GKISSSCENE_H
#define GKISSSCENE_H

#include <QGraphicsScene>
#include <QHash>
#include <QVector>
#include "archivedfile.h"
#include "gkissitem.h"

class GKissScene : public QGraphicsScene
{
    Q_OBJECT   // For qobject_cast
public:
    explicit GKissScene (QObject *parent = nullptr);
    // explicit GKissScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);
    ~GKissScene();
    /** Call this */
    void setEssentialContext(ArchivedFile *archivedFile, ParsedCnf* cnf, ParsedCnfSet *cnfSet);
    void drawCurSetBackground();
    void addCelItem(GKissItem *item);
    std::optional<QVector<GKissItem *> *> getItemsByObjId(int32_t objId);
    std::optional<GKissItem *> getItemByCelName(const QString &celFileName);
    void setItemsVisibility(const FkissVariant &target, bool visibility);
    void setItemsVisibilityByObjId(int32_t objId, bool visibility);
    void setItemVisibilityByCelName(const QString &celFileName, bool visibility);
    void bindItemsToObjects();
    bool moveObjectTo(size_t objId, int x, int y);
    bool moveObjectBy(size_t objId, int dx, int dy);
    void receiveFkissUserEventFromSceneItem(FkissUserMovementEvent evtType, QVector<FkissVariant> vars);
    QList<GKissItem *> allKissItems();
    uint8_t paletteNoInKcf() const;
    void setPaletteNoInKcf(uint8_t newPaletteNoInKcf);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    /// pass nullptr means clear
    void storeCurrentOverlappedItems(GKissItem *targetItem);
    QHash<int32_t, QVector<GKissItem*>*> m_objIdToItemsMap;
    QHash<QString, GKissItem *> m_celNameToItemMap;
    ArchivedFile *m_archivedFile;
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
};

#endif // GKISSSCENE_H
