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
    GKissView(QWidget *parent=nullptr);
    GKissView(ArchivedFile *archivedFile, QWidget *parent=nullptr);
    bool setArchivedFile(ArchivedFile *archivedFile);
    bool setCurCnf(QString cnfFileName);
    bool showSceneByIndex(uint8_t setIndex);
    /** create scenes & items according to current CNF. */
    bool renderCurCnfAllSets();
    // bool renderCurCnfSet(uint8_t setNo);
private:
    ArchivedFile *m_archivedFile;
    QVector<GKissScene *> m_scenes;
};

#endif // GKISSVIEW_H
