#ifndef GKISSITEM_H
#define GKISSITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QWidget>
#include "parsedkcf.h"
#include "parsedcel.h"

class GKissItem : public QGraphicsItem
{
public:
    GKissItem(ParsedCel *cel, ParsedKcf *kcf = nullptr, QVector<QRgb> *palette = nullptr, QGraphicsItem *parent = nullptr)
    {

    }

private:
};

#endif // GKISSITEM_H
