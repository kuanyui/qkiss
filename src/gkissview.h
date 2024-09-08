#ifndef GKISSVIEW_H
#define GKISSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QObject>
#include <QWidget>
#include "gkissscene.h"
#include "gkissitem.h"

class GKissView : public QGraphicsView
{
    Q_OBJECT;
public:
    GKissView(QWidget *parent=nullptr);
    GKissView(ArchivedFile *archivedFile, QWidget *parent=nullptr);
    bool loadArchivedFile(ArchivedFile *archivedFile);
    void unloadArchivedFile();
    void deleteAllScenes();
    bool setCurCnf(QString cnfFileName);
    uint8_t scenesAmount() const;
    bool showSceneByIndex(uint8_t setIndex);
    GKissScene *curScene();
    /** create scenes & items according to current CNF. */
    bool renderCurCnfAllSets();
    // bool renderCurCnfSet(uint8_t setNo);
    void bindItemsToObjects(GKissScene *scene, ParsedCnfSet *cnfSet); // TODO: Move this function to GKissScene

signals:
    void hoveredItemChanged(GKissItem *item, GKissScene *scene);
protected:
    virtual void mouseMoveEvent(QMouseEvent* event) override;
private:
    ArchivedFile *m_archivedFile;
    QVector<GKissScene *> m_scenes;
    uint8_t m_curSceneIdx = 0;
    GKissItem *m_curHoveredCelItem;
};

#endif // GKISSVIEW_H
