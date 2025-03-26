#include "gkissscene.h"
#include "fkissruntime.h"
using namespace Qt::Literals::StringLiterals;
using FV = FkissVariant;


GKissScene::GKissScene(ArchiveFile* archiveFile, ParsedCnf* cnf, int setIdx, QObject *parent):
    QGraphicsScene(parent)
{
    qDebug() << "[scene] ctor!" << m_setIndex;
    qDebug() << "[DEBUG] Scene::setEssentialContext" << archiveFile << cnf << m_cnfSet;
    m_archiveFile = archiveFile;
    m_cnf = cnf;
    m_setIndex = setIdx;
    m_cnfSet = cnf->sets()[setIdx];

    auto pf = m_cnf->playFieldSize();
    setSceneRect(0, 0, pf.width(), pf.height());
}

// GKissScene::GKissScene(qreal x, qreal y, qreal width, qreal height, QObject *parent):
//     QGraphicsScene(x, y, width, height, parent)
// {
//
// }

GKissScene::~GKissScene()
{
    qDebug() << "[scene] dtor!" << m_setIndex;
    for (QVector<GKissItem*> *vec : m_objIdToItemsMap.values()) {
        delete vec;
    }
}

std::optional<ArchiveFile *> GKissScene::archiveFile() const
{
    if (m_archiveFile == nullptr) { return std::nullopt; }
    return m_archiveFile;
}


void GKissScene::drawCurSetBackground()
{
    auto _firstKcf = m_archiveFile->getParsedKcfByOrderedIndexInCurrentCnf(0);
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
    // Some wrong CNF declared a wrong border bolor which index out of "palette_colors_count - 1".
    int borderColorIdx = qMin(static_cast<uint32_t>(firstPaletteInFirstKcfInCnf->length() - 1),
                              m_archiveFile->curCnf()->outsideBorderColor());
    QRgb borderColor = firstPaletteInFirstKcfInCnf->at(borderColorIdx);
    setBackgroundBrush(QBrush(borderColor));
    // play field background
    uint16_t paletteNoInKcf = m_cnfSet->paletteNoInKcf();
    if (paletteNoInKcf + 1 > firstKcfInCnf->paletteAmounts()) {
        qWarning().noquote() << "[Scene] In this CNF, setNo %1 specifies its default paletteNo as %2, but the first KCF contains only %3 palettes. Fallback to paletteNo 0 in the first KCF."_L1
                      .arg(QString::number(m_setIndex))
                      .arg(QString::number((int)paletteNoInKcf))
                      .arg(QString::number((int)firstKcfInCnf->paletteAmounts()));
        paletteNoInKcf = 0;
    }
    ParsedKcfPalette *paletteOfCnfSet = firstKcfInCnf->getPaletteInKcf(paletteNoInKcf);
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
    QList<GKissItem *> *tmp = m_objIdToItemsMap.value(objId, nullptr);
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

void GKissScene::setItemsVisibility(const FkissVariant &target, bool visibility)
{
    switch (target.type()) {
    case FkissVariant::Type::Object: return setItemsVisibilityByObjId(target.getObjectId(), visibility);
    case FkissVariant::Type::Int: return;
    case FkissVariant::Type::String: return setItemVisibilityByCelName(target.getString(), visibility);
    case FkissVariant::Type::Cel: return setItemVisibilityByCelName(target.getCelName(), visibility);
        break;
    }
}

void GKissScene::setItemVisibilityByCelName(const QString &celFileName, bool visibility)
{
    GKissItem *item = m_celNameToItemMap.value(celFileName, nullptr);
    if (item == nullptr) { return; }
    item->setVisible(visibility);
}

void GKissScene::setItemsVisibilityByObjId(int32_t objId, bool visibility)
{
    QList<GKissItem *> *items = m_objIdToItemsMap.value(objId, nullptr);
    if (items == nullptr) { return; }
    foreach (GKissItem *item, *items) {
        item->setVisible(visibility);
    }
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
    for (int objId = 0; objId < objPosList.length(); objId ++) {
        auto _objPos = objPosList[objId];
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
    }
    return true;
}

bool GKissScene::moveObjectBy(size_t objId, int dx, int dy)
{
    qDebug() << "moveObjectBy()" << objId << dx << dy;
    auto _items = getItemsByObjId(objId);
    if (!_items.has_value()) {
        qWarning() << QStringLiteral("[Scene] WTF, objectId %1 has no item??? Skip.").arg(objId);
        return false;
    }
    auto items = _items.value();
    foreach (GKissItem *item, *items) {
        item->setPos(dx + item->x(),
                     dy + item->y());
    }
    return true;
}

void GKissScene::executeTransparent(const FkissVariant &target, int delta)
{
    if (target.isObjectId()) {
        executeTransparentOnObj(target.getObjectId(), delta);
    } else if (target.isCelName()) {
        executeTransparentOnCel(target.getCelName(), delta);
    } else if (target.isString()) {
        executeTransparentOnCel(target.getString(), delta);
    }
}

void GKissScene::executeTransparentOnCel(QString celFileName, int delta)
{
    GKissItem *item = m_celNameToItemMap.value(celFileName, nullptr);
    if (item == nullptr) { return; }
    item->addKissTranslucency(delta);
}

void GKissScene::executeTransparentOnObj(int32_t objId, int delta)
{
    QList<GKissItem *> *items = m_objIdToItemsMap.value(objId, nullptr);
    if (items == nullptr) { return; }
    foreach (GKissItem *item, *items) {
        item->addKissTranslucency(delta);
    }
}

void GKissScene::receiveFkissUserEventFromSceneItem(FkissUserMovementEvent evtType, QVector<FkissVariant> vars)
{
    if (m_draggingWithShift) { return; }
    m_cnf->fkiss()->runtime()->receiveUserEventFromGraphicsItem(evtType, vars);

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

void GKissScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    // In FKiSS, there's no double click event. So just make this repeat press event 2 times.
    mousePressEvent(event);
    mousePressEvent(event);
}

void GKissScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();  // NOTE: Important!!! Otherwise mouseMoveEvent will not be triggered!
    QGraphicsItem *_item = itemAt(event->scenePos(), QTransform());
    if (!_item) { return; }
    GKissItem *item = qgraphicsitem_cast<GKissItem *>(_item);
    if (!item) { return; }
    m_draggingWithCtrl = event->modifiers() & Qt::ControlModifier;
    m_draggingWithShift = event->modifiers() & Qt::ShiftModifier;
    m_draggingItem = item;
    storeCurrentOverlappedItems(item);
    qDebug() << "[DEBUG] mousePressEvent " << item << event->scenePos();
    m_dragStartMouseScenePos = event->scenePos();
    m_fixDecreasedWhileDraggingItem = false;
    receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Press, { FV::fromCelName(m_draggingItem->celFileName()) });
    receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Press, { FV::fromObjectId(m_draggingItem->objId()) }); // press(target) - Triggered when the user clicks on the target.
    if (event->button() == Qt::RightButton) {
        m_draggingItem->setFix(0);
        foreach (GKissItem *sib, *m_draggingItem->siblings()) {
            sib->setFix(0);
        }
    }
    emit highlightedItemChanged(m_draggingItem);
    // QGraphicsItem::mousePressEvent(event);
}

void GKissScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_draggingItem == nullptr) { return; }
    // bool hasCtrl = event->modifiers() & Qt::ControlModifier;
    // bool hasShift = event->modifiers() & Qt::ShiftModifier;
    if (!m_dragRelatedFkissEventsAreAlreadyEmitted) {
        m_dragRelatedFkissEventsAreAlreadyEmitted = true;
        // catch(target) - The catch event is triggered when the user first begins to move the target.
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Catch, { FV::fromCelName(m_draggingItem->celFileName()) });
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Catch, { FV::fromObjectId(m_draggingItem->objId()) });
        if (m_draggingItem->fix() > 1) {
            // fixcatch(target) - Triggered when the user first begins to move a target which has a non-zero fix value.
            receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Fixcatch, { FV::fromCelName(m_draggingItem->celFileName()) });
            receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Fixcatch, { FV::fromObjectId(m_draggingItem->objId()) });
        } else if (m_draggingItem->fix() == 1) {
            // unfix(target) - Triggered when the target's fix value reaches 0.
            receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Unfix, { FV::fromCelName(m_draggingItem->celFileName()) });
            receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Unfix, { FV::fromObjectId(m_draggingItem->objId()) });
        }
        m_draggingItem->decreaseFix();
        m_fixDecreasedWhileDraggingItem = true;
    }
    if (m_draggingItem->fix() > 0) { return; }
    QPointF mouseDelta = event->scenePos() - event->lastScenePos();
    m_draggingItem->setPos(m_draggingItem->pos() + mouseDelta);
    if (!m_draggingWithCtrl) {
        foreach (GKissItem *sib, *m_draggingItem->siblings()) {
            sib->setPos(sib->pos() + mouseDelta);
        }
    }
    emit highlightedItemChanged(m_draggingItem);
    // QGraphicsItem::mouseMoveEvent(event);
}
void GKissScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    if (m_draggingItem == nullptr) { return; }
    m_dragRelatedFkissEventsAreAlreadyEmitted = false;
    // =======================================================================
    // Pixel overlapping checking (collide / apart)
    // =======================================================================
    QSet<GKissItem *> currentPixelOverlappedItems;
    foreach (QGraphicsItem *x, m_draggingItem->collidingItems(Qt::IntersectsItemShape)) {
        GKissItem *item = qgraphicsitem_cast<GKissItem *>(x);
        if (item == nullptr) { continue; }
        currentPixelOverlappedItems.insert(item);
    }
    QSet<GKissItem *> newlyPixelOverlappedItems = currentPixelOverlappedItems;
    newlyPixelOverlappedItems.subtract(m_originalPixelOverlappedItems);
    foreach (auto item, newlyPixelOverlappedItems) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be a cel.
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Collide, { FV::fromCelName(m_draggingItem->celFileName()), FV::fromCelName(item->celFileName()) });
    }
    QSet<GKissItem *> newlyPixelApartedItems = m_originalPixelOverlappedItems;
    newlyPixelApartedItems.subtract(currentPixelOverlappedItems);
    foreach (auto item, newlyPixelApartedItems) {
         // In FKiSS2 to FKiSS3, both target1 and target2 must be a cel.
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Apart, { FV::fromCelName(m_draggingItem->celFileName()), FV::fromCelName(item->celFileName()) });
    }

    // =======================================================================
    // Bounding rectangle overlapping checking (in / stillin / out / stillout)
    // =======================================================================
    QSet<int> currentBoundingRectOverlappedObjectIds;
    QSet<int> currentBoundingRectNonOverlappedObjectIds;
    foreach (GKissItem *item, allKissItems()) {
        if (item->objId() == m_draggingItem->objId()) { continue; }
        if (item->collidesWithItem(m_draggingItem, Qt::IntersectsItemBoundingRect)) {
            currentBoundingRectOverlappedObjectIds.insert(item->objId());
        } else {
            currentBoundingRectNonOverlappedObjectIds.insert(item->objId());
        }
    }
    QSet<int> newlyBoundingRectInObjectIds = currentBoundingRectOverlappedObjectIds;
    newlyBoundingRectInObjectIds.subtract(m_originalBoundingRectOverlappedObjIds);
    foreach (auto otherObjId, newlyBoundingRectInObjectIds) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be an object number.
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::In, { FV::fromObjectId(m_draggingItem->objId()), FV::fromObjectId(otherObjId) });
    }
    foreach (auto otherObjId, currentBoundingRectOverlappedObjectIds) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be an object number.
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Stillin, { FV::fromObjectId(m_draggingItem->objId()), FV::fromObjectId(otherObjId) });
    }
    QSet<int> newlyBoundingRectOutObjectIds = m_originalBoundingRectOverlappedObjIds;
    newlyBoundingRectOutObjectIds.subtract(currentBoundingRectOverlappedObjectIds);
    foreach (auto otherObjId, newlyBoundingRectOutObjectIds) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be an object number.
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Out, { FV::fromObjectId(m_draggingItem->objId()), FV::fromObjectId(otherObjId) });
    }
    foreach (auto otherObjId, currentBoundingRectNonOverlappedObjectIds) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be an object number.
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Stillout, { FV::fromObjectId(m_draggingItem->objId()), FV::fromObjectId(otherObjId) });
    }
    // =======================================================================
    // Other events
    // =======================================================================
    receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Release, { FV::fromCelName(m_draggingItem->celFileName()) });
    receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Release, { FV::fromObjectId(m_draggingItem->objId()) });
    if (!m_fixDecreasedWhileDraggingItem) {
        m_draggingItem->decreaseFix();
    }
    if (m_draggingItem->fix() == 0) {
        // drop(target) event is triggered, if fix value of object is equal to 0
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Drop, { FV::fromCelName(m_draggingItem->celFileName()) });
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Drop, { FV::fromObjectId(m_draggingItem->objId()) });
    } else {
        // fixdrop(target) event is triggered, if fix value of object is greater than 0
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Fixdrop, { FV::fromCelName(m_draggingItem->celFileName()) });
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Fixdrop, { FV::fromObjectId(m_draggingItem->objId()) });
    }

    foreach (GKissItem *sib, *m_draggingItem->siblings()) {
        sib->setFix(m_draggingItem->fix());
    }
    emit highlightedItemChanged(m_draggingItem);
    // =======================================================================
    // update all overlapped items
    // =======================================================================
    m_draggingItem = nullptr;
    storeCurrentOverlappedItems(nullptr);
    // QSet<int> allEffectedObjIds;
    // allEffectedObjIds.unite(newlyBoundingRectInObjectIds);
    // allEffectedObjIds.unite(newlyBoundingRectOutObjectIds);
    // QSet<GKissItem *> allEffectedItems;
    // foreach (auto item, allKissItems()) {
    //     if (item == m_draggingItem) { continue; }
    //     if (allEffectedObjIds.contains(item->objId())) {
    //         allEffectedItems.insert(item);
    //     }
    // }
    // allEffectedItems.unite(m_originalPixelOverlappedItems);
    // allEffectedItems.unite(currentPixelOverlappedItems);
}

void GKissScene::storeCurrentOverlappedItems(GKissItem *targetItem)
{
    m_originalPixelOverlappedItems.clear();
    if (targetItem == nullptr) { return; }
    foreach (QGraphicsItem *x, targetItem->collidingItems(Qt::IntersectsItemShape)) {
        GKissItem *item = qgraphicsitem_cast<GKissItem *>(x);
        if (item == nullptr) { continue; }
        m_originalPixelOverlappedItems.insert(item);
    }
    m_originalBoundingRectOverlappedObjIds.clear();
    foreach (QGraphicsItem *x, targetItem->collidingItems(Qt::IntersectsItemBoundingRect)) {
        GKissItem *item = qgraphicsitem_cast<GKissItem *>(x);
        if (item == nullptr) { continue; }
        m_originalBoundingRectOverlappedObjIds.insert(item->objId());
    }
}


