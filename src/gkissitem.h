#ifndef GKISSITEM_H
#define GKISSITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QWidget>
#include <QImage>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QDebug>
#include <QSet>
#include "parsedkcf.h"
#include "parsedcel.h"
#include "parsedcnf.h"
#include "fkiss.h"

class GKissScene;

class GKissItem : public QGraphicsPixmapItem
{
public:
    enum { Type = UserType + 1 };
    virtual int type() const override { return Type; }

    GKissItem(ParsedCnfCel *cnfCel, ParsedCel *cel, ParsedKcf *kcf, uint16_t paletteNoInKcf = 0, QGraphicsItem *parent = nullptr);
    GKissItem(ParsedCnfCel *cnfCel, ParsedCel *cel, QGraphicsItem *parent = nullptr);
    void rerenderImage();
    void setPaletteNoInKcf(uint8_t newPaletteNoInKcf);

    int32_t objId() const;
    QString celFileName() const;
    int32_t fix() const;
    ParsedCel *cel() const;
    ParsedCnfCel *cnfCel() const;
    void addSibling(GKissItem* item);
    QVector<GKissItem*>* siblings();
    void setFix(int32_t newFix);
    void decreaseFix();
    virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;
    /// May be a nullptr.
    GKissScene *kissScene();
    /// Emit event: Item -> Scene -> Scene.cnf().fkiss()
    void dispatchFkissUserEventToScene(FkissUserMovementEvent evtType, QVector<FkissVariant> vars);
    void storeCurrentOverlappedItems();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    void init();
    void renderPaletteBasedImage();
    void renderColorBasedImage();
    ParsedCnfCel *m_cnfCel;
    ParsedCel *m_cel;
    ParsedKcf *m_kcf;
    uint8_t m_paletteNoInKcf;
    // ==========================
    QImage *m_image;
    int32_t m_fix;
    QPointF m_dragStartPosition;
    QVector<GKissItem*> m_siblings;
    /// cache of casted scene()
    GKissScene *m_cachedKissScene = nullptr;
    bool m_fixDecreasedWhileMouseMoveEvent = false;
    /// @borrowed
    /// For collide(), apart()
    QSet<GKissItem*> m_originalPixelOverlappedItems;
    /// @borrowed
    /// For in(), stillin(), out(), stillout()
    // QSet<GKissItem*> m_originalBoundingRectOverlappedItems;
    QSet<int> m_originalBoundingRectOverlappedObjIds;
};

QDebug operator<<(QDebug dbg, const GKissItem &item);

#endif // GKISSITEM_H
