#include "gkissscene.h"


GKissScene::GKissScene(QObject *parent):
    QGraphicsScene(parent)
{

}

// GKissScene::GKissScene(qreal x, qreal y, qreal width, qreal height, QObject *parent):
//     QGraphicsScene(x, y, width, height, parent)
// {
//
// }

GKissScene::~GKissScene()
{
    for (QVector<GKissItem*> *vec : m_objIdToItemsMap.values()) {
        delete vec;
    }
}

void GKissScene::setEssentialContext(ArchivedFile *archivedFile, ParsedCnf *cnf, ParsedCnfSet *cnfSet)
{
    m_archivedFile = archivedFile;
    m_cnf = cnf;
    m_cnfSet = cnfSet;
    auto pf = m_cnf->playFieldSize();
    setSceneRect(0, 0, pf.width(), pf.height());
}

void GKissScene::drawCurSetBackground()
{
    auto _firstKcf = m_archivedFile->getParsedKcfByOrderedIndexInCurrentCnf(0);
    if (!_firstKcf.has_value()) {
        qCritical() << "[ERROR][Scene] No CKF found in this CNF, abort.";
        return;
    }
    ParsedKcf *firstKcfInCnf = _firstKcf.value();
    // Border color (outside play field)
    ParsedKcfPalette *firstPaletteInFirstKcfInCnf = firstKcfInCnf->getPaletteInKcf(0);
    if (firstPaletteInFirstKcfInCnf == nullptr) {
        qCritical() << "[ERROR][Scene] No palette in this KCF. Abort.";
        return;
    }
    QRgb borderColor = (*firstPaletteInFirstKcfInCnf)[m_archivedFile->curCnf()->outsideBorderColor()];
    setBackgroundBrush(QBrush(borderColor));
    // play field background
    ParsedKcfPalette *paletteOfCnfSet = firstKcfInCnf->getPaletteInKcf(m_cnfSet->paletteNoInKcf());
    if (paletteOfCnfSet == nullptr) {
        qCritical() << "[ERROR][Scene] WTF? In CNF's set definition, not found palette in first KCF of this CNF. Abort.";
        return;
    }
    QRgb playFieldColor = (*paletteOfCnfSet)[0];
    QColor playFieldBorderColor = QColor(playFieldColor).darker(130);
    this->addRect(this->sceneRect(), QPen(playFieldBorderColor), QBrush(playFieldColor));
}

void GKissScene::addCelItem(GKissItem *item)
{
    addItem(item);
    if (m_objIdToItemsMap.contains(item->objId())) {
        QVector<GKissItem*> * itemsOfObj = m_objIdToItemsMap[item->objId()];
        foreach (GKissItem* sibling, *itemsOfObj) {
            sibling->addSibling(item);
            item->addSibling(sibling);
        }
        itemsOfObj->append(item);
    } else {
        QVector<GKissItem*> * itemsOfObj = new QVector<GKissItem*>;
        itemsOfObj->append(item);
        m_objIdToItemsMap.insert(item->objId(), itemsOfObj);
    }
}

QVector<GKissItem *> *GKissScene::getItemsByObjId(int32_t objId)
{
    return m_objIdToItemsMap.value(objId);
}

