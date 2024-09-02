#include "gkissview.h"
#include "parsedcnf.h"
#include "parsedkcf.h"
#include "parsedcel.h"
#include <QString>
using namespace Qt::Literals::StringLiterals;

GKissView::GKissView(ArchivedFile *archivedFile, QWidget *parent):
    QGraphicsView(parent),
{
    setArchivedFile(archivedFile);
}


bool GKissView::setArchivedFile(ArchivedFile *archivedFile)
{
    m_archivedFile = archivedFile;
    QStringList cnfFileNameList = m_archivedFile->getCnfNameList();
    if (cnfFileNameList.isEmpty()) {
        qCritical() << "[ERROR] No CNF file found in this archivedFile.";
        return false;
    }
    setCurCnf(cnfFileNameList.first());
    return true;
}

bool GKissView::setCurCnf(QString cnfFileName)
{
    bool ok = m_archivedFile->setCurCnf(cnfFileName);
    if (!ok) {
        qCritical() << "[ERROR] No CNF file named " << cnfFileName << " found in this archivedFile.";
        return false;
    }
}

bool GKissView::renderCurCnfAllSets()
{
    m_scenes.clear();
    ParsedCnf &cnf = *m_archivedFile->curCnf();
    QStringList kcfFileNamesInCnf = cnf.kcfFileNames();
    for (ParsedCnfSet *cnfSet : cnf.sets()) {
        for (ParsedCnfCel *cnfCel : cnf.cels()) {
            auto _cel = m_archivedFile->getParsedCel(cnfCel->celFileName());
            if (!_cel.has_value()) {
                qCritical() << "[ERROR] CNF requires a CEL, but no such CEL file found in this archived file: "_L1 << cnfCel->celFileName();
                continue;
            }
            ParsedCel &cel = *_cel.value();

            if (cel.celType() == ParsedCel::Type::PaletteBased) {   // TODO: If cel is PaletteBased, don't need to get ParsedKcf
                ParsedKcf *kcf = m_archivedFile->getParsedKcfByOrderedIndexInCurrentCnf(cnfCel)
                const QVector<QRgb> &palette = kcf.getPaletteInKcf(cnfSet.paletteNoInKcf());

            }
        }
    }
}
