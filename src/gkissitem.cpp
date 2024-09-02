#include "gkissitem.h"



GKissItem::GKissItem(ParsedCnfCel *cnfCel, ParsedCel *cel, ParsedKcf *kcf, uint16_t paletteNoInKcf, QGraphicsItem *parent):
    QGraphicsItem(parent),
    m_cnfCel(cnfCel),
    m_cel(cel),
    m_kcf(kcf),
    m_paletteNoInKcf(paletteNoInKcf)
{
    m_image = new QImage(cel->width(), cel->height(), QImage::Format_ARGB32);
    renderImage();
    m_fix = m_cnfCel->fix();
}

void GKissItem::renderImage()
{

}

void GKissItem::renderPaletteBasedImage()
{
    const ParsedCel &cel = *m_cel;
    const ParsedKcfPalette &palette = m_kcf->getPaletteInKcf(m_paletteNoInKcf);
    const QVector<QRgb> &celPixelsData = cel.getAs32BitsPixels();
    if (m_cel->celType() == ParsedCel::Type::PaletteBased) {
        const ParsedKcfPalette &palette = m_kcf->getPaletteInKcf(m_paletteNoInKcf);
        for (int y = 0; y < cel.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb *>(m_image->scanLine(y));
            for (int x = 0; x < cel.width(); ++x) {
                line[x] =
            }
        }
    }
}

void GKissItem::renderFullColorImage()
{

}
