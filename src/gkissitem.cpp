#include "gkissitem.h"
using namespace Qt::Literals::StringLiterals;


GKissItem::GKissItem(ParsedCnfCel *cnfCel, ParsedCel *cel, ParsedKcf *kcf, uint16_t paletteNoInKcf, QGraphicsItem *parent):
    QGraphicsPixmapItem(parent),
    m_cnfCel(cnfCel),
    m_cel(cel),
    m_kcf(kcf),
    m_paletteNoInKcf(paletteNoInKcf)
{
    m_image = new QImage(cel->width(), cel->height(), QImage::Format_ARGB32);
    rerenderImage();
    m_fix = m_cnfCel->fix();
}

GKissItem::GKissItem(ParsedCnfCel *cnfCel, ParsedCel *cel, QGraphicsItem *parent):
    QGraphicsPixmapItem(parent),
    m_cnfCel(cnfCel),
    m_cel(cel)
{
    m_image = new QImage(cel->width(), cel->height(), QImage::Format_ARGB32);
    rerenderImage();
    m_fix = m_cnfCel->fix();
}

void GKissItem::rerenderImage()
{
    qDebug() << QString("[Item] generate image for CEL '%1'").arg(m_cnfCel->celFileName());
    switch (m_cel->celType()) {
    case ParsedCel::Type::PaletteBased: return renderPaletteBasedImage();
    case ParsedCel::Type::ColorBased: return renderColorBasedImage();
    }
}


void GKissItem::renderPaletteBasedImage()
{
    ParsedKcfPalette *palette = m_kcf->getPaletteInKcf(m_paletteNoInKcf);
    std::unique_ptr<QVector<QRgb>> celPixelsData = m_cel->getColorizedPaletteBasedPixels(*m_kcf, m_paletteNoInKcf);
    uint32_t i = 0;
    if (m_cel->celType() == ParsedCel::Type::PaletteBased) {
        for (int y = 0; y < m_cel->height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb *>(m_image->scanLine(y));
            for (int x = 0; x < m_cel->width(); ++x) {
                line[x] = (*celPixelsData)[i++];
                if (i == celPixelsData->length() - 1) {
                    qCritical() << "[WARNING][Item] %1: This should not happened."_L1.arg(m_cnfCel->celFileName());
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
                if (i == celPixelsData->length() - 1) {
                    qCritical() << "[WARNING][Item] %1: This should not happened."_L1.arg(m_cnfCel->celFileName());
                    goto end;
                }
            }
        }
    }
    end:
    setPixmap(QPixmap::fromImage(*m_image));
}

void GKissItem::setPaletteNoInKcf(uint8_t newPaletteNoInKcf)
{
    m_paletteNoInKcf = newPaletteNoInKcf;
    rerenderImage();
}
