#include "gkissview.h"

GKissView::GKissView(GKissScene *scene, QWidget *parent):
    QGraphicsView(scene, parent),
    m_scene(scene)
{

}
