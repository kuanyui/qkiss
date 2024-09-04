#ifndef GKISSSCENE_H
#define GKISSSCENE_H

#include <QGraphicsScene>
#include <QHash>
#include <QVector>
#include "archivedfile.h"
#include "gkissitem.h"

class GKissScene : public QGraphicsScene
{
public:
    explicit GKissScene (QObject *parent = nullptr);
    explicit GKissScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);
    ~GKissScene();
    void drawBackaground(ArchivedFile *archivedFile, ParsedCnfSet *cnfSet);
    void addCelItem(GKissItem *item);
    QVector<GKissItem*>* getItemsByObjId(int32_t objId);
private:
    QHash<int32_t, QVector<GKissItem*>*> m_objIdToItemsMap;
};

#endif // GKISSSCENE_H
