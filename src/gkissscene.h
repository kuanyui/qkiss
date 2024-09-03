#ifndef GKISSSCENE_H
#define GKISSSCENE_H

#include <QGraphicsScene>
#include "archivedfile.h"

class GKissScene : public QGraphicsScene
{
public:
    explicit GKissScene (QObject *parent = nullptr);
    explicit GKissScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);

private:

};

#endif // GKISSSCENE_H
