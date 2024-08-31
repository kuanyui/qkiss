#ifndef GKISSVIEW_H
#define GKISSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QObject>
#include <QWidget>
#include "gkissscene.h"

class GKissView : public QGraphicsView
{
public:
    GKissView(GKissScene *scene, QWidget *parent=nullptr);
private:
    GKissScene *m_scene;
};

#endif // GKISSVIEW_H
