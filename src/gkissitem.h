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
    /// CEL file itself, which define (x,y) in the Object it belongs to.
    ParsedCel *cel() const;
    /// A CEL in a CNF.
    ParsedCnfCel *cnfCel() const;
    void addSibling(GKissItem* item);
    QVector<GKissItem*>* siblings();
    void setFix(int32_t newFix);
    void decreaseFix();
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

    QPointF initialPos() const;
    void setInitialPos(QPointF newInitialPos);

    // QGraphicsItem interface
protected:
    virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

private:
    void init();
    void renderPaletteBasedImage();
    void renderColorBasedImage();
    ParsedCnfCel *m_cnfCel;
    ParsedCel *m_cel;
    ParsedKcf *m_kcf = nullptr;
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

    /// Remember the initial position of this item (object pos + cel pos). Used for:
    /// 1. resetting object's state.
    /// 2. FKiSS's `ifmoved()`
    QPointF m_initialPos;

};

QDebug operator<<(QDebug dbg, const GKissItem &item);

#endif // GKISSITEM_H
