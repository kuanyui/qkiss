#include "gkissscene.h"


GKissScene::GKissScene(QObject *parent):
    QGraphicsScene(parent)
{

}

GKissScene::GKissScene(qreal x, qreal y, qreal width, qreal height, QObject *parent):
    QGraphicsScene(x, y, width, height, parent)
{

}

