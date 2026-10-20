/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2024 - 2025 ono ono (kuanyui) <azazabc123@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "gkissscene.h"
#include "fkissruntime.h"
#include "logging.h"
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QWidgetAction>
#include <QLabel>
#include <QScreen>
#include <QClipboard>

using namespace Qt::Literals::StringLiterals;
using FV = FkissVariant;


GKissScene::GKissScene(ArchiveFile* archiveFile, ParsedCnf* cnf, int setIdx, QObject *parent):
    QGraphicsScene(parent)
{
    qkDebug << "[scene] ctor!" << m_setIndex;
    qkDebug << "[DEBUG] Scene::setEssentialContext" << archiveFile << cnf << m_cnfSet;
    m_archiveFile = archiveFile;
    m_cnf = cnf;
    m_setIndex = setIdx;
    m_cnfSet = cnf->sets()[setIdx];
    m_paletteNoInKcf = m_cnfSet->paletteNoInKcf();    // Initialize a mutable paletteNo of this set (scene), because the set's paletteNo in parsed CNF is read-only.

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
    qkDebug << "[scene] dtor!" << m_setIndex;
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
        qkWarning.noquote() << tr("No KCF found in this CNF, don't draw set background.");
        return;
    }
    ParsedKcf *firstKcfInCnf = _firstKcf.value();
    // Border color (outside play field)
    ParsedKcfPalette *firstPaletteInFirstKcfInCnf = firstKcfInCnf->getPaletteInKcf(0);
    if (firstPaletteInFirstKcfInCnf == nullptr) {
        qkCritical.noquote() << tr("No palette in this KCF. Abort.");
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
        qkWarning.noquote() << tr("In this CNF, setNo %1 specifies its default paletteNo as %2, but the first KCF contains only %3 palettes. Fallback to paletteNo 0 in the first KCF.")
                      .arg(QString::number(m_setIndex))
                      .arg(QString::number((int)paletteNoInKcf))
                      .arg(QString::number((int)firstKcfInCnf->paletteAmounts()));
        paletteNoInKcf = 0;
    }
    ParsedKcfPalette *paletteOfCnfSet = firstKcfInCnf->getPaletteInKcf(paletteNoInKcf);
    if (paletteOfCnfSet == nullptr) {
        qkCritical.noquote() << tr("WTF? In CNF's set definition, not found palette in first KCF of this CNF. Abort.");
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

std::optional<QVector<GKissItem *> *>GKissScene::getItemsByObjId(int32_t objId) const
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
    case FkissVariant::Type::String:
        qkDebug << "[WARNING][Bug] This should not happen.";
        return setItemVisibilityByCelName(target.getOriginalString(), visibility);  // FIXME: Unreasonable
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

void GKissScene::toggleItemsVisibility(const FkissVariant &target)
{
    switch (target.type()) {
    case FkissVariant::Type::Object: return toggleItemsVisibilityByObjId(target.getObjectId());
    case FkissVariant::Type::Int: return;
    case FkissVariant::Type::String:
        qkDebug << "[WARNING][Bug] This should not happen.";
        return toggleItemVisibilityByCelName(target.getOriginalString());  // FIXME: Unreasonable
    case FkissVariant::Type::Cel: return toggleItemVisibilityByCelName(target.getCelName());
        break;
    }
}

void GKissScene::toggleItemVisibilityByCelName(const QString &celFileName)
{
    GKissItem *item = m_celNameToItemMap.value(celFileName, nullptr);
    if (item == nullptr) { return; }
    item->setVisible(!item->isVisible());
}

void GKissScene::toggleItemsVisibilityByObjId(int32_t objId)
{
    QList<GKissItem *> *items = m_objIdToItemsMap.value(objId, nullptr);
    if (items == nullptr) { return; }
    foreach (GKissItem *item, *items) {
        item->setVisible(!item->isVisible());
    }
}

#define argnum(num) arg(QString::number(num))

void GKissScene::bindItemsToObjects()
{
    // /** (In the CEL list (#)) ordered by the value of ObjectID . Length is always larger than object potitions list ($) */
    // const QList<int32_t> &sortedObjIdList = m_cnf->sortedObjIdList();
    const QList<ParsedCnfSetObjPos> &objPosList = m_cnfSet->getObjPosList();
    // qkDebug << "BIND FOR SCENE ====================================";
    // qkDebug << QStringLiteral("[DEBUG][Scene] sortedObjIdList (Length = %1)").arg(sortedObjIdList.length()) << sortedObjIdList;  // FIXME: Misunderstood. Unneeded anymore. Should remove it.
    // qkDebug << QStringLiteral("[DEBUG][Scene] ObjPosList (Length = %1)").arg(objPosList.length());
    // Iterate by objId (small to large)
    for (int objId = 0; objId < objPosList.length(); objId ++) {
        auto _objPos = objPosList[objId];
        QPoint objPos = _objPos.has_value() ? _objPos.value() : QPoint(0, 0);
        // qkDebug << "[DEBUG][Scene] sceneIndex: %1,\t\t\tobjId: %2, \t\tobjPos: (%3, %4)"_L1.argnum(m_setIndex).arg(objId).argnum(objPos.x()).argnum(objPos.y());
        auto _items = this->getItemsByObjId(objId);
        if (!_items.has_value()) {
            // qkDebug << "[DEBUG][Scene] No items";
            continue;
        }
        auto items = _items.value();

        foreach (GKissItem *item, *items) {
            item->setPos(objPos.x() + item->cel()->x(),
                         objPos.y() + item->cel()->y());
            item->setInitialPos(item->pos());
            // qkDebug.noquote().nospace() << "[DEBUG][Scene] bindItemsToObjects() ===> sceneIndex: %1,\t\t\tcelFileName: %2\t\t objId: %3, \t\t itemPos: (%4, %5)"_L1.argnum(m_setIndex).arg(item->celFileName()).argnum(objId).argnum(item->x()).argnum(item->y());
        }
    }
}

std::optional<QPoint> GKissScene::getObjectCurrentPos(size_t objId) const
{
    auto _objItems = getItemsByObjId(objId);
    if (!_objItems.has_value()) {
        qkWarning << QStringLiteral("[Scene] WTF, anchorObjId %1 has no item??? Skip.").arg(objId);
        return std::nullopt;
    }
    auto objItems = _objItems.value();
    if (objItems->empty()) {
        qkWarning << QStringLiteral("[Scene] WTF??? Impossible, the obj #%1 has no items???").arg(objId);
        return std::nullopt;
    }
    auto item = objItems->at(0);
    QPoint itemCurPosInScene = item->pos().toPoint();
    QPoint itemCelPos = item->cel()->pos();
    return std::optional<QPoint>(itemCurPosInScene - itemCelPos);
}


bool GKissScene::moveObjectTo(size_t objId, int x, int y)
{
    // const auto _objPos = m_cnfSet->getObjPosByObjId(objId);
    // QPoint objPos = _objPos.has_value() ? _objPos.value() : QPoint(0, 0);
    auto _items = getItemsByObjId(objId);
    if (!_items.has_value()) {
        qkWarning << QStringLiteral("[Scene] WTF, objectId %1 has no item??? Skip.").arg(objId);
        return false;
    }
    auto items = _items.value();
    emitCollideEventsBeforeFkissMovement(objId, *items);
    auto originalPixelOverlappedItems = getPixelOverlappedItems(*items);
    auto originalBoundingRectOverlappedObjIds = getBoundingRectOverlappedObjIds(objId, *items);
    foreach (GKissItem *item, *items) {
        item->setPos(x + item->cel()->x(),
                     y + item->cel()->y());
    }
    if (m_draggingItem != nullptr && m_draggingItem->objId() == static_cast<int32_t>(objId)) {
        m_draggingStoppedByFkissMoveto = true;
    }
    emitFkissMovementEvents(objId, *items, originalPixelOverlappedItems, originalBoundingRectOverlappedObjIds);
    return true;
}

bool GKissScene::moveObject(size_t objId, int dx, int dy)
{
    auto _items = getItemsByObjId(objId);
    if (!_items.has_value()) {
        qkWarning << QStringLiteral("[Scene] WTF, objectId %1 has no item??? Skip.").arg(objId);
        return false;
    }
    auto items = _items.value();
    emitCollideEventsBeforeFkissMovement(objId, *items);
    auto originalPixelOverlappedItems = getPixelOverlappedItems(*items);
    auto originalBoundingRectOverlappedObjIds = getBoundingRectOverlappedObjIds(objId, *items);
    restrictMovementToPlayField(*items, dx, dy);
    foreach (GKissItem *item, *items) {
        item->setPos(dx + item->x(),
                     dy + item->y());
    }
    emitFkissMovementEvents(objId, *items, originalPixelOverlappedItems, originalBoundingRectOverlappedObjIds);
    return true;
}
#define argnum(num) arg(QString::number(num))

bool GKissScene::moveObjectByX(size_t targetObjId, size_t anchorObjId, int dx)
{
    auto _targetObjPos = getObjectCurrentPos(targetObjId);
    if (!_targetObjPos.has_value()) {
        qkWarning << QStringLiteral("[Scene] WTF???");
        return false;
    }
    auto targetObjPos = _targetObjPos.value();

    auto _anchorObjPos = getObjectCurrentPos(anchorObjId);
    if (!_anchorObjPos.has_value()) {
        qkWarning << QStringLiteral("[Scene] WTF???");
        return false;
    }
    auto anchorObjPos = _anchorObjPos.value();

    moveObjectTo(targetObjId,
                 anchorObjPos.x() + dx,
                 targetObjPos.y());
    return true;
}

bool GKissScene::moveObjectByY(size_t targetObjId, size_t anchorObjId, int dy)
{
    auto _targetObjPos = getObjectCurrentPos(targetObjId);
    if (!_targetObjPos.has_value()) {
        qkWarning << QStringLiteral("[Scene] WTF???");
        return false;
    }
    auto targetObjPos = _targetObjPos.value();

    auto _anchorObjPos = getObjectCurrentPos(anchorObjId);
    if (!_anchorObjPos.has_value()) {
        qkWarning << QStringLiteral("[Scene] WTF???");
        return false;
    }
    auto anchorObjPos = _anchorObjPos.value();

    moveObjectTo(targetObjId,
                 targetObjPos.x(),
                 anchorObjPos.y() + dy);
    return true;
}

void GKissScene::executeTransparent(const FkissVariant &target, int delta)
{
    if (target.isObjectId()) {
        executeTransparentOnObj(target.getObjectId(), delta);
    } else if (target.isCelName()) {
        executeTransparentOnCel(target.getCelName(), delta);
    } else if (target.isString()) {
        executeTransparentOnCel(target.getOriginalString(), delta);  // FIXME: Unreasonable.
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

void GKissScene::setPaletteNoInKcfAndRender(uint8_t newPaletteNoInKcf)
{
    m_paletteNoInKcf = newPaletteNoInKcf;
    foreach (QGraphicsItem *_item, items()) {
        GKissItem *item = qgraphicsitem_cast<GKissItem *>(_item);
        if (item == nullptr) { continue; }
        item->setPaletteNoInKcfAndRender(m_paletteNoInKcf);
    }
}

void GKissScene::unfixObjectByItem(GKissItem *item)
{
    item->setFix(0);
    foreach (GKissItem *sib, *(item->siblings())) {
        sib->setFix(0);
    }
}

void GKissScene::resetObjectByItem(GKissItem *item)
{
    item->setPos(item->initialPos());
    item->setFix(item->cnfCel()->fix());
    foreach (auto *sib , *(item->siblings())) {
        sib->setPos(sib->initialPos());
        sib->setFix(sib->cnfCel()->fix());
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
    qkDebug << "mousePressEvent()" << item->cnfCel();
    if (event->button() == Qt::RightButton) {
        return;
    }
    m_draggingWithCtrl = event->modifiers() & Qt::ControlModifier;
    m_draggingWithShift = event->modifiers() & Qt::ShiftModifier;
    m_draggingItem = item;
    storeCurrentOverlappedItems(item);
    qkDebug << "[DEBUG] mousePressEvent " << item << event->scenePos();
    m_dragStartMouseScenePos = event->scenePos();
    m_fixDecreasedWhileDraggingItem = false;
    m_draggingStoppedByFkissMoveto = false;
    receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Press, { FV::fromCelName(m_draggingItem->celFileName()) });
    receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Press, { FV::fromObjectId(m_draggingItem->objId()) }); // press(target) - Triggered when the user clicks on the target.
    if (event->button() == Qt::MouseButton::MiddleButton) {
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
    if (m_draggingStoppedByFkissMoveto) { return; }
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
    // Compute all overlaps before emitting any event: the FKiSS actions of these events may move objects, even this one.
    const QSet<GKissItem *> originalPixelOverlappedItems = m_originalPixelOverlappedItems.value(m_draggingItem);
    const QSet<int> originalBoundingRectOverlappedObjIds = m_originalBoundingRectOverlappedObjIds;
    // =======================================================================
    // Pixel overlapping checking (collide / apart)
    // =======================================================================
    QSet<GKissItem *> currentPixelOverlappedItems;
    foreach (QGraphicsItem *x, m_draggingItem->collidingItems(Qt::IntersectsItemShape)) {
        GKissItem *item = qgraphicsitem_cast<GKissItem *>(x);
        if (item == nullptr) { continue; }
        currentPixelOverlappedItems.insert(item);
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
    emitCollideEvents(m_draggingItem, originalPixelOverlappedItems, currentPixelOverlappedItems);
    emitApartEvents(m_draggingItem, originalPixelOverlappedItems, currentPixelOverlappedItems);
    emitBoundingRectOverlapEvents(m_draggingItem->objId(), originalBoundingRectOverlappedObjIds, currentBoundingRectOverlappedObjectIds, currentBoundingRectNonOverlappedObjectIds);
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
    m_draggingWithShift = false;  // Otherwise collide() / apart() triggered by FKiSS movement stay paused after a Shift-drag.
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

void GKissScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QGraphicsItem *_item = itemAt(event->scenePos(), QTransform());
    if (_item == nullptr) { return; }
    GKissItem *item = qgraphicsitem_cast<GKissItem *>(_item);
    if (item == nullptr) { return; }

    QMenu menu;
    QAction *actUnfix = menu.addAction(tr("[%1] Unfix (&U)").arg(QString::number(item->fix())));
    actUnfix->setIcon(QIcon::fromTheme("unfix"));
    QAction *actResetPos= menu.addAction(tr("Reset Object (&R)"));
    actResetPos->setIcon(QIcon::fromTheme("reset-position"));
    menu.addSection(tr("Information"));
    // Show CEL filename
    QAction *dummyFileName = menu.addAction(item->cnfCel()->celFileName().original(), [=](){
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(item->cnfCel()->celFileName().original());
    });
    dummyFileName->setIcon(QIcon::fromTheme("image-x-generic"));
    dummyFileName->setStatusTip(tr("Click to copy filename to clipboard."));  // FIXME: doesn't work... Why?
    // dummyFileName->setEnabled(false);
    // Show comment if exists
    if (!item->cnfCel()->comment().isEmpty()) {
        QAction *dummyComment = menu.addAction(item->cnfCel()->comment());
        dummyComment->setIcon(QIcon::fromTheme("comment"));
        // dummyComment->setEnabled(false);
    }
    // Show image as preview
    QLabel *imgLabel = new QLabel{};
    QPixmap pixmap = item->pixmap();
    int targetSize = qMin(pixmap.width(), pixmap.height());
    // TODO: Scale according current zooming
    targetSize = qMin(targetSize, 400);
    imgLabel->setPixmap(pixmap.scaled(targetSize, targetSize, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    QWidgetAction *actImg = new QWidgetAction{&menu};
    actImg->setDefaultWidget(imgLabel);
    menu.addAction(actImg);
    // Show context menu synchronously
    QAction *selected = menu.exec(event->screenPos());
    if (selected == actUnfix) {
        unfixObjectByItem(item);
    } else if (selected == actResetPos) {
        resetObjectByItem(item);
    }
}



void GKissScene::storeCurrentOverlappedItems(GKissItem *targetItem)
{
    m_originalPixelOverlappedItems.clear();
    if (targetItem == nullptr) { return; }
    // All cels of the object, for emitCollideEventsBeforeFkissMovement(). mouseReleaseEvent() uses only targetItem's.
    const QVector<GKissItem *> &items = *m_objIdToItemsMap.value(targetItem->objId());
    const QList<QSet<GKissItem *>> pixelOverlappedItems = getPixelOverlappedItems(items);
    for (qsizetype i = 0; i < items.length(); i++) {
        m_originalPixelOverlappedItems.insert(items[i], pixelOverlappedItems[i]);
    }
    m_originalBoundingRectOverlappedObjIds.clear();
    foreach (QGraphicsItem *x, targetItem->collidingItems(Qt::IntersectsItemBoundingRect)) {
        GKissItem *item = qgraphicsitem_cast<GKissItem *>(x);
        if (item == nullptr) { continue; }
        m_originalBoundingRectOverlappedObjIds.insert(item->objId());
    }
}

QList<QSet<GKissItem *>> GKissScene::getPixelOverlappedItems(const QVector<GKissItem *> &targetItems) const
{
    QList<QSet<GKissItem *>> result;
    foreach (GKissItem *targetItem, targetItems) {
        QSet<GKissItem *> overlappedItems;
        foreach (QGraphicsItem *x, targetItem->collidingItems(Qt::IntersectsItemShape)) {
            GKissItem *item = qgraphicsitem_cast<GKissItem *>(x);
            if (item == nullptr) { continue; }
            overlappedItems.insert(item);
        }
        result.append(overlappedItems);
    }
    return result;
}

QSet<int> GKissScene::getBoundingRectOverlappedObjIds(int32_t objId, const QVector<GKissItem *> &targetItems)
{
    QSet<int> result;
    foreach (GKissItem *item, allKissItems()) {
        if (item->objId() == objId) { continue; }
        foreach (GKissItem *targetItem, targetItems) {
            if (item->collidesWithItem(targetItem, Qt::IntersectsItemBoundingRect)) {
                result.insert(item->objId());
                break;
            }
        }
    }
    return result;
}

void GKissScene::restrictMovementToPlayField(const QVector<GKissItem *> &items, int &dx, int &dy) const
{
    QRect rect;
    foreach (GKissItem *item, items) {
        rect |= item->sceneBoundingRect().toAlignedRect();
    }
    if (rect.isNull()) { return; }
    const QRect playField = sceneRect().toRect();
    // If the object is larger than the play field, its left / top edge stays in it.
    if (rect.right() + dx > playField.right()) { dx = playField.right() - rect.right(); }
    if (rect.left() + dx < playField.left()) { dx = playField.left() - rect.left(); }
    if (rect.bottom() + dy > playField.bottom()) { dy = playField.bottom() - rect.bottom(); }
    if (rect.top() + dy < playField.top()) { dy = playField.top() - rect.top(); }
}

void GKissScene::emitCollideEvents(GKissItem *movedItem, const QSet<GKissItem *> &originalPixelOverlappedItems, const QSet<GKissItem *> &currentPixelOverlappedItems)
{
    QSet<GKissItem *> newlyPixelOverlappedItems = currentPixelOverlappedItems;
    newlyPixelOverlappedItems.subtract(originalPixelOverlappedItems);
    foreach (auto item, newlyPixelOverlappedItems) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be a cel.
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Collide, { FV::fromCelName(movedItem->celFileName()), FV::fromCelName(item->celFileName()) });
    }
}

void GKissScene::emitApartEvents(GKissItem *movedItem, const QSet<GKissItem *> &originalPixelOverlappedItems, const QSet<GKissItem *> &currentPixelOverlappedItems)
{
    QSet<GKissItem *> newlyPixelApartedItems = originalPixelOverlappedItems;
    newlyPixelApartedItems.subtract(currentPixelOverlappedItems);
    foreach (auto item, newlyPixelApartedItems) {
         // In FKiSS2 to FKiSS3, both target1 and target2 must be a cel.
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Apart, { FV::fromCelName(movedItem->celFileName()), FV::fromCelName(item->celFileName()) });
    }
}

void GKissScene::emitBoundingRectOverlapEvents(int32_t objId, const QSet<int> &originalOverlappedObjIds, const QSet<int> &currentOverlappedObjIds, const QSet<int> &currentNonOverlappedObjIds)
{
    QSet<int> newlyBoundingRectInObjectIds = currentOverlappedObjIds;
    newlyBoundingRectInObjectIds.subtract(originalOverlappedObjIds);
    foreach (auto otherObjId, newlyBoundingRectInObjectIds) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be an object number.
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::In, { FV::fromObjectId(objId), FV::fromObjectId(otherObjId) });
    }
    foreach (auto otherObjId, currentOverlappedObjIds) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be an object number.
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Stillin, { FV::fromObjectId(objId), FV::fromObjectId(otherObjId) });
    }
    QSet<int> newlyBoundingRectOutObjectIds = originalOverlappedObjIds;
    newlyBoundingRectOutObjectIds.subtract(currentOverlappedObjIds);
    foreach (auto otherObjId, newlyBoundingRectOutObjectIds) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be an object number.
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Out, { FV::fromObjectId(objId), FV::fromObjectId(otherObjId) });
    }
    foreach (auto otherObjId, currentNonOverlappedObjIds) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be an object number.
        receiveFkissUserEventFromSceneItem(FkissUserMovementEvent::Stillout, { FV::fromObjectId(objId), FV::fromObjectId(otherObjId) });
    }
}

/// When FKiSS moves something while the user is dragging an item, mouseReleaseEvent() and
/// emitCollideEventsBeforeFkissMovement() should not emit the same events again, so update what they compare with: the
/// overlaps at mouse press.
template <typename T>
static void syncOverlapsAtMousePress(QSet<T> &overlapsAtMousePress, const T &moved, const T &dragging, const QSet<T> &original, const QSet<T> &current)
{
    if (moved == dragging) {
        overlapsAtMousePress.unite(current - original).subtract(original - current);
    } else if (current.contains(dragging) && !original.contains(dragging)) {
        overlapsAtMousePress.insert(moved);
    } else if (original.contains(dragging) && !current.contains(dragging)) {
        overlapsAtMousePress.remove(moved);
    }
}

void GKissScene::emitFkissMovementEvents(int32_t objId, const QVector<GKissItem *> &movedItems, const QList<QSet<GKissItem *>> &originalPixelOverlappedItems, const QSet<int> &originalBoundingRectOverlappedObjIds)
{
    if (m_fkissMovementDepth >= MAX_FKISS_MOVEMENT_DEPTH) {
        qkWarning.noquote() << tr("FKiSS movement events are nested more than %1 levels, so skip the events of moving object #%2.").arg(MAX_FKISS_MOVEMENT_DEPTH).arg(objId);
        return;
    }
    // The items of the same object are all in movedItems: count each pair once.
    const QList<QSet<GKissItem *>> currentPixelOverlappedItems = getPixelOverlappedItems(movedItems);
    QList<QSet<GKissItem *>> originalPixel, currentPixel;
    for (qsizetype i = 0; i < movedItems.length(); i++) {
        const QSet<GKissItem *> countedItems { movedItems.begin(), movedItems.begin() + i };
        originalPixel.append(originalPixelOverlappedItems[i] - countedItems);
        currentPixel.append(currentPixelOverlappedItems[i] - countedItems);
    }
    const QSet<int> currentBoundingRectOverlappedObjIds = getBoundingRectOverlappedObjIds(objId, movedItems);
    QSet<int> currentBoundingRectNonOverlappedObjIds;
    foreach (GKissItem *item, allKissItems()) {
        if (item->objId() != objId && !currentBoundingRectOverlappedObjIds.contains(item->objId())) {
            currentBoundingRectNonOverlappedObjIds.insert(item->objId());
        }
    }
    if (m_draggingItem != nullptr) {
        for (qsizetype i = 0; i < movedItems.length(); i++) {
            for (auto it = m_originalPixelOverlappedItems.begin(); it != m_originalPixelOverlappedItems.end(); ++it) {
                syncOverlapsAtMousePress(it.value(), movedItems[i], it.key(), originalPixel[i], currentPixel[i]);
            }
        }
        syncOverlapsAtMousePress<int>(m_originalBoundingRectOverlappedObjIds, objId, m_draggingItem->objId(), originalBoundingRectOverlappedObjIds, currentBoundingRectOverlappedObjIds);
    }
    m_fkissMovementDepth++;
    for (qsizetype i = 0; i < movedItems.length(); i++) {
        emitApartEvents(movedItems[i], originalPixel[i], currentPixel[i]);
    }
    emitBoundingRectOverlapEvents(objId, originalBoundingRectOverlappedObjIds, currentBoundingRectOverlappedObjIds, currentBoundingRectNonOverlappedObjIds);
    for (qsizetype i = 0; i < movedItems.length(); i++) {
        emitCollideEvents(movedItems[i], originalPixel[i], currentPixel[i]);
    }
    m_fkissMovementDepth--;
}

void GKissScene::emitCollideEventsBeforeFkissMovement(int32_t objId, const QVector<GKissItem *> &items)
{
    if (m_draggingItem == nullptr || m_draggingItem->objId() != objId) { return; }
    if (m_fkissMovementDepth >= MAX_FKISS_MOVEMENT_DEPTH) { return; }  // emitFkissMovementEvents() warns.
    // The items of the same object are all in items: count each pair once.
    const QList<QSet<GKissItem *>> currentPixelOverlappedItems = getPixelOverlappedItems(items);
    QList<QSet<GKissItem *>> lastCheckedPixel, currentPixel;
    for (qsizetype i = 0; i < items.length(); i++) {
        const QSet<GKissItem *> countedItems { items.begin(), items.begin() + i };
        QSet<GKissItem *> &lastChecked = m_originalPixelOverlappedItems[items[i]];
        lastCheckedPixel.append(lastChecked - countedItems);
        currentPixel.append(currentPixelOverlappedItems[i] - countedItems);
        // Only add: apart() is not checked here, so mouseReleaseEvent() can still emit it.
        lastChecked.unite(currentPixelOverlappedItems[i]);
    }
    m_fkissMovementDepth++;
    for (qsizetype i = 0; i < items.length(); i++) {
        emitCollideEvents(items[i], lastCheckedPixel[i], currentPixel[i]);
    }
    m_fkissMovementDepth--;
}


