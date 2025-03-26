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
    void setPaletteNoInKcfAndRender(uint8_t newPaletteNoInKcf);

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
    /// FKiSS: 0 (opaque) ~ 255 (transparent).
    uint8_t kissTranslucency() const;
    /// FKiSS: 0 (opaque) ~ 255 (transparent). Value will be automatically-trimmed to 0~255 if out-of-range.
    void setKissTranslucency(int newKissTranscluency);
    /// -255 ~ 255
    void addKissTranslucency(int delta);
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
    /// The translucency of a cel is in the range 0 to 255. 0 is fully opaque (the default). 255 is fully invisible.
    uint8_t m_kissTranslucency = 0;
    // QPointF m_dragStartPosition;
    /// CEL items with same Object ID
    QVector<GKissItem*> m_siblings;
    /// cache of casted scene()
    GKissScene *m_cachedKissScene = nullptr;


};

QDebug operator<<(QDebug dbg, const GKissItem &item);

#endif // GKISSITEM_H
