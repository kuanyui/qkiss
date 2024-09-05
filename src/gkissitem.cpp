#include "gkissitem.h"
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
    case ParsedCel::Type::ColorBased: return renderColorBasedImage();
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
                line[x] = celColorizedPixelsData->at(i++);
                // line[x] = qRgb(255,255,0);
                if (i == celColorizedPixelsData->length() - 1) {
                    qCritical() << "[WARNING][Item] %1: Pixels length and WxH are not matched. This should not happened."_L1.arg(m_cnfCel->celFileName());
                    goto end;
                }
            }
        }
    }
    end:
    setPixmap(QPixmap::fromImage(*m_image));
}


void GKissItem::renderColorBasedImage()
{
    QVector<QRgb> *celPixelsData = m_cel->getColorBasedPixels();
    uint32_t i = 0;
    if (m_cel->celType() == ParsedCel::Type::PaletteBased) {
        for (int y = 0; y < m_cel->height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb *>(m_image->scanLine(y));
            for (int x = 0; x < m_cel->width(); ++x) {
                line[x] = (*celPixelsData)[i++];
                if (i > celPixelsData->length()) {
                    qCritical() << "[WARNING][Item] %1: This should not happened."_L1.arg(m_cnfCel->celFileName());
                    goto end;
                }
            }
        }
    }
    end:
    setPixmap(QPixmap::fromImage(*m_image));
}

void GKissItem::setFix(int32_t newFix)
{
    m_fix = newFix;
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

void GKissItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();  // NOTE: Important!!! Otherwise mouseMoveEvent will not be triggered!
    qDebug() << "[DEBUG] mousePressEvent " << this;
    m_dragStartPosition = event->pos();
    // QGraphicsItem::mousePressEvent(event);
}

void GKissItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_fix > 0) { return; }
    QPointF delta = event->pos() - m_dragStartPosition;
    setPos(pos() + delta);
    foreach (GKissItem *sib, m_siblings) {
        sib->setPos(sib->pos() + delta);
    }
    // QGraphicsItem::mouseMoveEvent(event);
}

void GKissItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_fix <= 0) { return; }
    m_fix--;
    for (GKissItem* sib: m_siblings) {
        sib->setFix(m_fix);
    }
    // QGraphicsItem::mouseReleaseEvent(event);
}

ParsedCel *GKissItem::cel() const
{
    return m_cel;
}

void GKissItem::setPaletteNoInKcf(uint8_t newPaletteNoInKcf)
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

