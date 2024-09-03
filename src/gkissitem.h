#ifndef GKISSITEM_H
#define GKISSITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QWidget>
#include <QImage>
#include "parsedkcf.h"
#include "parsedcel.h"
#include "parsedcnf.h"

class GKissItem : public QGraphicsPixmapItem
{
public:
    GKissItem(ParsedCnfCel *cnfCel, ParsedCel *cel, ParsedKcf *kcf, uint16_t paletteNoInKcf = 0, QGraphicsItem *parent = nullptr);
    GKissItem(ParsedCnfCel *cnfCel, ParsedCel *cel, QGraphicsItem *parent = nullptr);
    void rerenderImage();
    void setPaletteNoInKcf(uint8_t newPaletteNoInKcf);

private:
    void renderPaletteBasedImage();
    void renderColorBasedImage();
    ParsedCnfCel *m_cnfCel;
    ParsedCel *m_cel;
    ParsedKcf *m_kcf;
    uint8_t m_paletteNoInKcf;
    // ==========================
    QImage *m_image;
    int16_t m_fix;
};

#endif // GKISSITEM_H
