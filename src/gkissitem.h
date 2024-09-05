#ifndef GKISSITEM_H
#define GKISSITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QWidget>
#include <QImage>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include "parsedkcf.h"
#include "parsedcel.h"
#include "parsedcnf.h"

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
};

#endif // GKISSITEM_H
