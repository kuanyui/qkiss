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
        qCritical() << "[ERROR][Scene] No KCF found in this CNF, abort.";
        return;
    }
    ParsedKcf *firstKcfInCnf = _firstKcf.value();
    // Border color (outside play field)
    ParsedKcfPalette *firstPaletteInFirstKcfInCnf = firstKcfInCnf->getPaletteInKcf(0);
    if (firstPaletteInFirstKcfInCnf == nullptr) {
        qCritical() << "[ERROR][Scene] No palette in this KCF. Abort.";
        return;
    }
    QRgb borderColor = firstPaletteInFirstKcfInCnf->at(m_archivedFile->curCnf()->outsideBorderColor());
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
    if (!m_celNameToItemMap.contains(item->celFileName())) {
        m_celNameToItemMap.insert(item->celFileName(), item);
    }
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

std::optional<QVector<GKissItem *> *>GKissScene::getItemsByObjId(int32_t objId)
{
    QList<GKissItem *> *tmp = m_objIdToItemsMap.value(objId);
    if (tmp == nullptr) {
        return std::nullopt;
    }
    return tmp;
}

std::optional<GKissItem *> GKissScene::getItemByCelName(const QString &celFileName)
{
    GKissItem *tmp = m_celNameToItemMap.value(celFileName);
    return tmp == nullptr ? std::nullopt : std::optional<GKissItem *>(tmp);
}

void GKissScene::bindItemsToObjects()
{
    // /** (In the CEL list (#)) ordered by the value of ObjectID . Length is always larger than object potitions list ($) */
    // const QList<int32_t> &sortedObjIdList = m_cnf->sortedObjIdList();
    /** (In the object potitions list ($)) Ordered by the value of ObjectID. */
    const QList<ParsedCnfSetObjPos> &objPosList = m_cnfSet->getObjPosList();
    // qDebug() << "BIND FOR SCENE ====================================";
    // qDebug() << QStringLiteral("[DEBUG][Scene] sortedObjIdList (Length = %1)").arg(sortedObjIdList.length()) << sortedObjIdList;  // FIXME: Misunderstood. Unneeded anymore. Should remove it.
    // qDebug() << QStringLiteral("[DEBUG][Scene] ObjPosList (Length = %1)").arg(objPosList.length());
    // Iterate by objId (small to large)
    for (size_t objId = 0; objId < objPosList.length(); objId ++) {
        auto _objPos = objPosList[objId ];
        QPoint objPos = _objPos.has_value() ? _objPos.value() : QPoint(0, 0);
        // qDebug() << QStringLiteral("[DEBUG][Scene] index: %1,   objId: %2,   objPos: (%3, %4)").arg(i).arg(objId).arg(objPos.x()).arg(objPos.y());
        auto _items = this->getItemsByObjId(objId);
        if (!_items.has_value()) {
            // qDebug() << "[DEBUG][Scene] No items";
            continue;
        }
        auto items = _items.value();
        int32_t maxFix = items->isEmpty() ? 0 : items->at(0)->fix();
        foreach (GKissItem *item, *items) {
            maxFix = qMax(item->fix(), maxFix);
            item->setPos(objPos.x() + item->cel()->x(),
                         objPos.y() + item->cel()->y());
            item->storeCurrentOverlappedItems();
        }
        // The fix value for an object is the maximum value for all member cels.
        foreach (GKissItem *item, *items) {
            item->setFix(maxFix);
        }
    }
}

bool GKissScene::moveObjectTo(size_t objId, int x, int y)
{
    // const auto _objPos = m_cnfSet->getObjPosByObjId(objId);
    // QPoint objPos = _objPos.has_value() ? _objPos.value() : QPoint(0, 0);
    auto _items = getItemsByObjId(objId);
    if (!_items.has_value()) {
        qWarning() << QStringLiteral("[Scene] WTF, objectId %1 has no item??? Skip.").arg(objId);
        return false;
    }
    auto items = _items.value();
    foreach (GKissItem *item, *items) {
        item->setPos(x + item->cel()->x(),
                     y + item->cel()->y());
        item->storeCurrentOverlappedItems();
    }
}

void GKissScene::receiveFkissUserEventFromSceneItem(FkissUserMovementEvent evtType, QVector<FkissVariant> vars)
{
    m_cnf->fkiss()->receiveUserEventFromGraphicsItem(evtType, vars);

}

QList<GKissItem *> GKissScene::allKissItems()
{
    return m_celNameToItemMap.values();
}

uint8_t GKissScene::paletteNoInKcf() const
{
    return m_paletteNoInKcf;
}

void GKissScene::setPaletteNoInKcf(uint8_t newPaletteNoInKcf)
{
    m_paletteNoInKcf = newPaletteNoInKcf;
    foreach (QGraphicsItem *_item, items()) {
        GKissItem *item = qgraphicsitem_cast<GKissItem *>(_item);
        if (item == nullptr) { continue; }
        item->setPaletteNoInKcf(newPaletteNoInKcf);
    }
}

