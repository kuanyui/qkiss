#ifndef GKISSSCENE_H
#define GKISSSCENE_H

#include <QGraphicsScene>
#include "archivedfile.h"

class GKissScene : public QGraphicsScene
{
public:
    explicit GKissScene(ArchivedFile *archivedFile, QObject *parent = nullptr);
    bool loadArchivedFile(ArchivedFile *archivedFile);
    bool setCurrentCnf(QString cnfFileName);
    bool renderCurCnfAllSets();
private:
    ArchivedFile *m_archivedFile;

};

#endif // GKISSSCENE_H
