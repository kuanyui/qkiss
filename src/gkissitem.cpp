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
                    qCritical() << "[WARNING][Item] %1: Pixels length and WxH are not matched. This should not happened."_L1.arg(m_cnfCel->celFileName());
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
    QVector<QRgb> *celPixelsData = m_cel->getColorBasedPixels();
    if (celPixelsData == nullptr) { return; }
    uint32_t i = 0;
    if (m_cel->celType() == ParsedCel::Type::FullColor) {
        for (int y = 0; y < m_cel->height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb *>(m_image->scanLine(y));
            for (int x = 0; x < m_cel->width(); ++x) {
                if (i == celPixelsData->length()) {
                    qCritical() << "[WARNING][Item] %1: This should not happened."_L1.arg(m_cnfCel->celFileName());
                    goto end;
                }
                line[x] = (*celPixelsData)[i++];
            }
        }
    }
    end:
    setPixmap(QPixmap::fromImage(*m_image));
    setKissTranslucency(m_cnfCel->translucency());
}

uint8_t GKissItem::kissTranslucency() const
{
    return m_kissTranslucency;
}

void GKissItem::setKissTranslucency(int newKissTranscluency)
{
    /// QGraphicsItem::setOpacity(qreal opacity)  0.0 (transparent) ~ 1.0 (opaque)
    /// FKiSS transparent(int)                    0 (opaque) ~ 255 (transparent)
    if (newKissTranscluency > 255) {
        newKissTranscluency = 255;
    } else if (newKissTranscluency < 0) {
        newKissTranscluency = 0;
    }
    if (newKissTranscluency == 0) {   // totally visible
        setOpacity(1);
    } else if (newKissTranscluency == 255) {  // totally transparent
        setOpacity(0);
    }
    m_kissTranslucency = newKissTranscluency;
    qreal inversedKissTranscluency = 255 - newKissTranscluency;
    qreal newOpacity = inversedKissTranscluency / 255;
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

QString GKissItem::celFileName() const
{
    return m_cnfCel->celFileName();
}

int32_t GKissItem::fix() const
{
    return m_fix;
}



QDebug operator<<(QDebug dbg, const GKissItem &item) {
    dbg.nospace().noquote() << QString("GKissItem { objId: #%1, \tcelFileName: '%2', \tpos: (%3, %4), \tvisible: %5 }")
                           .arg(item.objId())
                           .arg(item.celFileName())
                           .arg(item.pos().x())
                           .arg(item.pos().y())
                           .arg(item.isVisible());
    return dbg;
};


