#include "gkissscene.h"
#include "parsedcnf.h"
#include "parsedkcf.h"
#include "parsedcel.h"

GKissScene::GKissScene(ArchivedFile *archivedFile, QObject *parent)
    : QGraphicsScene{parent},
      m_archivedFile(archivedFile)
{

}

bool GKissScene::loadArchivedFile(ArchivedFile *archivedFile)
{
    QStringList cnfFileNameList = m_archivedFile->getCnfNameList();
    if (cnfFileNameList.isEmpty()) {
        qCritical() << "[ERROR] No CNF file found in this archivedFile.";
        return false;
    }
    setCurrentCnf(cnfFileNameList.first());
    return true;
}

bool GKissScene::setCurrentCnf(QString cnfFileName)
{
    clear();
    bool ok = m_archivedFile->setCurCnf(cnfFileName);
    if (!ok) {
        qCritical() << "[ERROR] No CNF file named " << cnfFileName << " found in this archivedFile.";
        return false;
    }
}

bool GKissScene::renderCurCnfAllSets()
{
    ParsedCnf &cnf = *m_archivedFile->curCnf();
    for (ParsedCnfCel cnfCel : cnf.cels()) {
        auto _cel = m_archivedFile->getParsedCel(cnfCel.celFileName());
        if (!_cel.has_value()) {
            qCritical() << "[ERROR] CNF requires a CEL, but no such CEL file found in this archived file: " << cnfCel.celFileName();
            continue;
        }
        // m_archivedFile->getParsedKcf()
        // ParsedCel &cel = *_cel.value();

    }
}
