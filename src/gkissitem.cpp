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

#include "gkissitem.h"
#include "gkissscene.h"
#include <QSet>
using namespace Qt::Literals::StringLiterals;


GKissItem::GKissItem(ParsedCnfCel *cnfCel, ParsedCel *cel, ParsedKcf *kcf, uint16_t paletteNoInKcf, QGraphicsItem *parent):
    QGraphicsPixmapItem(parent),
    m_cnfCel(cnfCel),
    m_cel(cel),
    m_kcf(kcf),
    m_paletteNoInKcf(paletteNoInKcf)
{
    init();
}

GKissItem::GKissItem(ParsedCnfCel *cnfCel, ParsedCel *cel, QGraphicsItem *parent):
    QGraphicsPixmapItem(parent),
    m_cnfCel(cnfCel),
    m_cel(cel)
{
    init();
}

void GKissItem::init()
{
    // setFlag(QGraphicsItem::ItemIsMovable, true);
    // setFlag(QGraphicsItem::ItemIsSelectable);
    // setFlag(QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);
    // setShapeMode(ShapeMode::BoundingRectShape);   // Default is 0.

    m_image = new QImage(m_cel->width(), m_cel->height(), QImage::Format_ARGB32);
    rerenderImage();
    m_fix = m_cnfCel->fix();
}


void GKissItem::rerenderImage()
{
    // qDebug() << QStringLiteral("[Item] generate image for CEL '%1' size(%2, %3)  pos(%4, %5)")
    //             .arg(m_cnfCel->celFileName())
    //             .arg(m_cel->width())
    //             .arg(m_cel->height())
    //             .arg(m_cel->x())
    //             .arg(m_cel->y())
    //             ;
    switch (m_cel->celType()) {
    case ParsedCel::Type::PaletteBased: return renderPaletteBasedImage();
    case ParsedCel::Type::FullColor: return renderColorBasedImage();
    }
}


void GKissItem::renderPaletteBasedImage()
{
    std::unique_ptr<QVector<QRgb>> celColorizedPixelsData = m_cel->getColorizedPaletteBasedCelPixels(*m_kcf, m_paletteNoInKcf);
    uint32_t i = 0;
    if (m_cel->celType() == ParsedCel::Type::PaletteBased) {
        for (int y = 0; y < m_cel->height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb *>(m_image->scanLine(y));
            for (int x = 0; x < m_cel->width(); ++x) {
                if (i == celColorizedPixelsData->length()) {
                    qCritical() << "[WARNING][Item] %1: Pixels length and WxH are not matched. This should not happened."_L1.arg(m_cnfCel->celFileName().original());
                    goto end;
                }
                line[x] = celColorizedPixelsData->at(i++);
                // line[x] = qRgb(255,255,0);
            }
        }
    }
    end:
    setPixmap(QPixmap::fromImage(*m_image));
    setKissTranslucency(m_cnfCel->translucency());
}


void GKissItem::renderColorBasedImage()
{
    // if (m_cel->bitsPerPixel() != 32) {
    //     qWarning().noquote() << "[WARNING][Item] \"%1\" claims it is a palette-based CEL file, but it said each pixel is %2 bits (expected: 32 bits), this is contradicted. "_L1.arg(m_cel->fileName()).arg(QString::number(m_cel->bitsPerPixel()));
    //     return renderPaletteBasedImage();
    // }
    QVector<QRgb> *celPixelsData = m_cel->getColorBasedPixels();
    if (celPixelsData == nullptr) { return; }
    uint32_t i = 0;
    if (m_cel->celType() == ParsedCel::Type::FullColor) {
        for (int y = 0; y < m_cel->height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb *>(m_image->scanLine(y));
            for (int x = 0; x < m_cel->width(); ++x) {
                if (i == celPixelsData->length()) {
                    qCritical() << "[WARNING][Item] %1: This should not happened."_L1.arg(m_cnfCel->celFileName().original());
                    goto end;
                }
                line[x] = celPixelsData->at(i++);
            }
        }
    }
    end:
    setPixmap(QPixmap::fromImage(*m_image));
    setKissTranslucency(m_cnfCel->translucency());
}

QPointF GKissItem::initialPos() const
{
    return m_initialPos;
}

void GKissItem::setInitialPos(QPointF newInitialPos)
{
    m_initialPos = newInitialPos;
}

uint8_t GKissItem::kissTranslucency() const
{
    return m_kissTranslucency;
}

void GKissItem::setKissTranslucency(int newKissTranscluency)
{
    /// QGraphicsItem::setOpacity(qreal opacity)  0.0 (transparent) ~ 1.0 (opaque)
    /// FKiSS action transparent(int)             0   (opaque)      ~ 255 (transparent)
    if (newKissTranscluency >= 255) {
        newKissTranscluency = 255;    // totally transparent
    } else if (newKissTranscluency <= 0) {
        newKissTranscluency = 0;      // totally opacity
    }
    m_kissTranslucency = newKissTranscluency;
    qreal inversedKissTranscluency = 255 - newKissTranscluency;
    qreal newOpacity = inversedKissTranscluency / 255;
    // FIXME: Dirty workaround. If `QGraphicsPixmapItem::setOpacity(0), it will become totally unable be pressed. So set it to 0.01.
    // https://stackoverflow.com/questions/25206220/how-do-i-enable-a-transparent-qgraphicspixmapitem-to-receive-mouse-click-events
    // https://doc.qt.io/qt-6/qgraphicspixmapitem.html#ShapeMode-enum
    if (newOpacity == 0) {
        newOpacity = 0.01;
    }
    setOpacity(newOpacity);
}

void GKissItem::addKissTranslucency(int delta)
{
    setKissTranslucency(m_kissTranslucency + delta);
}

void GKissItem::setFix(int32_t newFix)
{
    m_fix = newFix;
}

void GKissItem::decreaseFix()
{
    if (m_fix <= 0) { return; }
    m_fix--;
}

QVariant GKissItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == GraphicsItemChange::ItemSceneChange) {
        m_cachedKissScene = value.value<GKissScene *>();
    }
    return value;
}

GKissScene *GKissItem::kissScene()
{
    return m_cachedKissScene;
}

void GKissItem::dispatchFkissUserEventToScene(FkissUserMovementEvent evtType, QVector<FkissVariant> vars)
{
    if (!m_cachedKissScene) { return; }
    m_cachedKissScene->receiveFkissUserEventFromSceneItem(evtType, vars);
}


ParsedCnfCel *GKissItem::cnfCel() const
{
    return m_cnfCel;
}

void GKissItem::addSibling(GKissItem *item)
{
    m_siblings.append(item);
}

QVector<GKissItem *> *GKissItem::siblings()
{
    return &m_siblings;
}

ParsedCel *GKissItem::cel() const
{
    return m_cel;
}

void GKissItem::setPaletteNoInKcfAndRender(uint8_t newPaletteNoInKcf)
{
    m_paletteNoInKcf = newPaletteNoInKcf;
    rerenderImage();
}

int32_t GKissItem::objId() const
{
    return m_cnfCel->objId();
}

CPString GKissItem::celFileName() const
{
    return m_cnfCel->celFileName();
}

int32_t GKissItem::fix() const
{
    return m_fix;
}



QDebug operator<<(QDebug dbg, const GKissItem &item) {
    dbg.nospace().noquote() << QString("GKissItem { objId: %1, \tcelFileName: '%2', \tpos: (%3, %4), \ttranslucency: %5, \tvisible: %6}")
                           .arg(item.objId())
                           .arg(item.celFileName().original())
                           .arg(item.pos().x())
                           .arg(item.pos().y())
                           .arg(item.kissTranslucency())
                           .arg(item.isVisible())
                               ;
    return dbg;
};


