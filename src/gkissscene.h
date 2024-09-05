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
    // explicit GKissScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);
    ~GKissScene();
    /** Call this */
    void setEssentialContext(ArchivedFile *archivedFile, ParsedCnf* cnf, ParsedCnfSet *cnfSet);
    void drawCurSetBackground();
    void addCelItem(GKissItem *item);
    QVector<GKissItem*>* getItemsByObjId(int32_t objId);
private:
    QHash<int32_t, QVector<GKissItem*>*> m_objIdToItemsMap;
    ArchivedFile *m_archivedFile;
    ParsedCnf* m_cnf;
    ParsedCnfSet *m_cnfSet;
};

#endif // GKISSSCENE_H
