#include "gkissitem.h"
#include "gkissscene.h"
#include <QSet>
using namespace Qt::Literals::StringLiterals;
using FV = FkissVariant;


GKissItem::GKissItem(ParsedCnfCel *cnfCel, ParsedCel *cel, ParsedKcf *kcf, uint16_t paletteNoInKcf, QGraphicsItem *parent):
    QGraphicsPixmapItem(parent),
    m_cnfCel(cnfCel),
    m_cel(cel),
    m_kcf(kcf),
    m_paletteNoInKcf(paletteNoInKcf)
{
    init();
}

GKissItem::GKissItem(ParsedCnfCel *cnfCel, ParsedCel *cel, QGraphicsItem *parent):
    QGraphicsPixmapItem(parent),
    m_cnfCel(cnfCel),
    m_cel(cel)
{
    init();
}

void GKissItem::init()
{
    // setFlag(QGraphicsItem::ItemIsMovable, true);
    // setFlag(QGraphicsItem::ItemIsSelectable);
    // setFlag(QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);
    m_image = new QImage(m_cel->width(), m_cel->height(), QImage::Format_ARGB32);
    rerenderImage();
    m_fix = m_cnfCel->fix();
}


void GKissItem::rerenderImage()
{
    // qDebug() << QStringLiteral("[Item] generate image for CEL '%1' size(%2, %3)  pos(%4, %5)")
    //             .arg(m_cnfCel->celFileName())
    //             .arg(m_cel->width())
    //             .arg(m_cel->height())
    //             .arg(m_cel->x())
    //             .arg(m_cel->y())
    //             ;
    switch (m_cel->celType()) {
    case ParsedCel::Type::PaletteBased: return renderPaletteBasedImage();
    case ParsedCel::Type::FullColor: return renderColorBasedImage();
    }
}


void GKissItem::renderPaletteBasedImage()
{
    std::unique_ptr<QVector<QRgb>> celColorizedPixelsData = m_cel->getColorizedPaletteBasedCelPixels(*m_kcf, m_paletteNoInKcf);
    uint32_t i = 0;
    if (m_cel->celType() == ParsedCel::Type::PaletteBased) {
        for (int y = 0; y < m_cel->height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb *>(m_image->scanLine(y));
            for (int x = 0; x < m_cel->width(); ++x) {
                if (i == celColorizedPixelsData->length()) {
                    qCritical() << "[WARNING][Item] %1: Pixels length and WxH are not matched. This should not happened."_L1.arg(m_cnfCel->celFileName());
                    goto end;
                }
                line[x] = celColorizedPixelsData->at(i++);
                // line[x] = qRgb(255,255,0);
            }
        }
    }
    end:
    setPixmap(QPixmap::fromImage(*m_image));
}


void GKissItem::renderColorBasedImage()
{
    QVector<QRgb> *celPixelsData = m_cel->getColorBasedPixels();
    uint32_t i = 0;
    if (m_cel->celType() == ParsedCel::Type::FullColor) {
        for (int y = 0; y < m_cel->height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb *>(m_image->scanLine(y));
            for (int x = 0; x < m_cel->width(); ++x) {
                if (i == celPixelsData->length()) {
                    qCritical() << "[WARNING][Item] %1: This should not happened."_L1.arg(m_cnfCel->celFileName());
                    goto end;
                }
                line[x] = (*celPixelsData)[i++];
            }
        }
    }
    end:
    setPixmap(QPixmap::fromImage(*m_image));
}

void GKissItem::setFix(int32_t newFix)
{
    m_fix = newFix;
}

void GKissItem::decreaseFix()
{
    if (m_fix <= 0) { return; }
    m_fix--;
}

QVariant GKissItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == GraphicsItemChange::ItemSceneChange) {
        m_cachedKissScene = value.value<GKissScene *>();
    }
    return value;
}

GKissScene *GKissItem::kissScene()
{
    return m_cachedKissScene;
}

void GKissItem::dispatchFkissUserEventToScene(FkissUserMovementEvent evtType, QVector<FkissVariant> vars)
{
    if (!m_cachedKissScene) { return; }
    m_cachedKissScene->receiveFkissUserEventFromSceneItem(evtType, vars);
}


ParsedCnfCel *GKissItem::cnfCel() const
{
    return m_cnfCel;
}

void GKissItem::addSibling(GKissItem *item)
{
    m_siblings.append(item);
}

QVector<GKissItem *> *GKissItem::siblings()
{
    return &m_siblings;
}

void GKissItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();  // NOTE: Important!!! Otherwise mouseMoveEvent will not be triggered!
    qDebug() << "[DEBUG] mousePressEvent " << this;
    m_dragStartPosition = event->pos();
    dispatchFkissUserEventToScene(FkissUserMovementEvent::Press, { FV::fromCelName(this->celFileName()) });
    dispatchFkissUserEventToScene(FkissUserMovementEvent::Press, { FV::fromObjectId(this->objId()) }); // press(target) - Triggered when the user clicks on the target.
    if (event->button() == Qt::RightButton) {
        m_fix = 0;
        foreach (GKissItem *sib, m_siblings) {
            sib->setFix(0);
        }
    }
    // QGraphicsItem::mousePressEvent(event);
}

void GKissItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    bool hasCtrl = event->modifiers() & Qt::ControlModifier;
    // bool hasShift = event->modifiers() & Qt::ControlModifier;

    // catch(target) - The catch event is triggered when the user first begins to move the target.
    dispatchFkissUserEventToScene(FkissUserMovementEvent::Catch, { FV::fromCelName(this->celFileName()) });
    dispatchFkissUserEventToScene(FkissUserMovementEvent::Catch, { FV::fromObjectId(this->objId()) });
    if (m_fix > 1) {
        // fixcatch(target) - Triggered when the user first begins to move a target which has a non-zero fix value.
        dispatchFkissUserEventToScene(FkissUserMovementEvent::Fixcatch, { FV::fromCelName(this->celFileName()) });
        dispatchFkissUserEventToScene(FkissUserMovementEvent::Fixcatch, { FV::fromObjectId(this->objId()) });
    } else if (m_fix == 1) {
        // unfix(target) - Triggered when the target's fix value reaches 0.
        dispatchFkissUserEventToScene(FkissUserMovementEvent::Unfix, { FV::fromCelName(this->celFileName()) });
        dispatchFkissUserEventToScene(FkissUserMovementEvent::Unfix, { FV::fromObjectId(this->objId()) });
    }
    decreaseFix();
    m_fixDecreasedWhileMouseMoveEvent = true;
    if (m_fix > 0) { return; }

    QPointF delta = event->pos() - m_dragStartPosition;
    setPos(pos() + delta);
    if (!hasCtrl) {
        foreach (GKissItem *sib, m_siblings) {
            sib->setPos(sib->pos() + delta);
        }
    }
    // QGraphicsItem::mouseMoveEvent(event);
}
void GKissItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    // =======================================================================
    // Pixel overlapping checking (collide / apart)
    // =======================================================================
    QSet<GKissItem *> currentPixelOverlappedItems;
    foreach (QGraphicsItem *x, collidingItems(Qt::IntersectsItemShape)) {
        GKissItem *item = qgraphicsitem_cast<GKissItem *>(x);
        if (item == nullptr) { continue; }
        currentPixelOverlappedItems.insert(item);
    }
    QSet<GKissItem *> newlyPixelOverlappedItems = currentPixelOverlappedItems;
    newlyPixelOverlappedItems.subtract(m_originalPixelOverlappedItems);
    foreach (auto item, newlyPixelOverlappedItems) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be a cel.
        dispatchFkissUserEventToScene(FkissUserMovementEvent::Collide, { FV::fromCelName(this->celFileName()), FV::fromCelName(item->celFileName()) });
    }
    QSet<GKissItem *> newlyPixelApartedItems = m_originalPixelOverlappedItems;
    newlyPixelApartedItems.subtract(currentPixelOverlappedItems);
    foreach (auto item, newlyPixelApartedItems) {
         // In FKiSS2 to FKiSS3, both target1 and target2 must be a cel.
        dispatchFkissUserEventToScene(FkissUserMovementEvent::Apart, { FV::fromCelName(this->celFileName()), FV::fromCelName(item->celFileName()) });
    }

    // =======================================================================
    // Bounding rectangle overlapping checking (in / stillin / out / stillout)
    // =======================================================================
    QSet<int> currentBoundingRectOverlappedObjectIds;
    QSet<int> currentBoundingRectNonOverlappedObjectIds;
    foreach (GKissItem *item, m_cachedKissScene->allKissItems()) {
        if (item->objId() == this->objId()) { continue; }
        if (item->collidesWithItem(this, Qt::IntersectsItemBoundingRect)) {
            currentBoundingRectOverlappedObjectIds.insert(item->objId());
        } else {
            currentBoundingRectNonOverlappedObjectIds.insert(item->objId());
        }
    }
    QSet<int> newlyBoundingRectInObjectIds = currentBoundingRectOverlappedObjectIds;
    newlyBoundingRectInObjectIds.subtract(m_originalBoundingRectOverlappedObjIds);
    foreach (auto otherObjId, newlyBoundingRectInObjectIds) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be an object number.
        dispatchFkissUserEventToScene(FkissUserMovementEvent::In, { FV::fromObjectId(this->objId()), FV::fromObjectId(otherObjId) });
    }
    foreach (auto otherObjId, currentBoundingRectOverlappedObjectIds) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be an object number.
        dispatchFkissUserEventToScene(FkissUserMovementEvent::Stillin, { FV::fromObjectId(this->objId()), FV::fromObjectId(otherObjId) });
    }
    QSet<int> newlyBoundingRectOutObjectIds = m_originalBoundingRectOverlappedObjIds;
    newlyBoundingRectOutObjectIds.subtract(currentBoundingRectOverlappedObjectIds);
    foreach (auto otherObjId, newlyBoundingRectOutObjectIds) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be an object number.
        dispatchFkissUserEventToScene(FkissUserMovementEvent::Out, { FV::fromObjectId(this->objId()), FV::fromObjectId(otherObjId) });
    }
    foreach (auto otherObjId, currentBoundingRectNonOverlappedObjectIds) {
        // In FKiSS2 to FKiSS3, both target1 and target2 must be an object number.
        dispatchFkissUserEventToScene(FkissUserMovementEvent::Stillin, { FV::fromObjectId(this->objId()), FV::fromObjectId(otherObjId) });
    }
    // =======================================================================
    // Other events
    // =======================================================================
    dispatchFkissUserEventToScene(FkissUserMovementEvent::Release, { FV::fromCelName(this->celFileName()) });
    dispatchFkissUserEventToScene(FkissUserMovementEvent::Release, { FV::fromObjectId(this->objId()) });
    if (!m_fixDecreasedWhileMouseMoveEvent) {
        decreaseFix();
        m_fixDecreasedWhileMouseMoveEvent = false;
    }
    if (m_fix == 0) {
        // drop(target) event is triggered, if fix value of object is equal to 0
        dispatchFkissUserEventToScene(FkissUserMovementEvent::Drop, { FV::fromCelName(this->celFileName()) });
        dispatchFkissUserEventToScene(FkissUserMovementEvent::Drop, { FV::fromObjectId(this->objId()) });
    } else {
        // fixdrop(target) event is triggered, if fix value of object is greater than 0
        dispatchFkissUserEventToScene(FkissUserMovementEvent::Fixdrop, { FV::fromCelName(this->celFileName()) });
        dispatchFkissUserEventToScene(FkissUserMovementEvent::Fixdrop, { FV::fromObjectId(this->objId()) });
    }

    foreach (GKissItem* sib, m_siblings) {
        sib->setFix(m_fix);
    }
    // =======================================================================
    // update all overlapped items
    // =======================================================================
    storeCurrentOverlappedItems();
    QSet<int> allEffectedObjIds;
    allEffectedObjIds.unite(newlyBoundingRectInObjectIds);
    allEffectedObjIds.unite(newlyBoundingRectOutObjectIds);
    QSet<GKissItem *> allEffectedItems;
    foreach (auto item, m_cachedKissScene->allKissItems()) {
        if (item == this) { continue; }
        if (allEffectedObjIds.contains(item->objId())) {
            allEffectedItems.insert(item);
        }
    }
    allEffectedItems.unite(m_originalPixelOverlappedItems);
    allEffectedItems.unite(currentPixelOverlappedItems);
    for (auto item: allEffectedItems) {
        item->storeCurrentOverlappedItems();
    }
    // QGraphicsItem::mouseReleaseEvent(event);
}

ParsedCel *GKissItem::cel() const
{
    return m_cel;
}

void GKissItem::setPaletteNoInKcf(uint8_t newPaletteNoInKcf)
{
    m_paletteNoInKcf = newPaletteNoInKcf;
    rerenderImage();
}

int32_t GKissItem::objId() const
{
    return m_cnfCel->objId();
}

QString GKissItem::celFileName() const
{
    return m_cnfCel->celFileName();
}

int32_t GKissItem::fix() const
{
    return m_fix;
}

void GKissItem::storeCurrentOverlappedItems()
{
    /// TODO: chaining react...
    m_originalPixelOverlappedItems.clear();
    foreach (QGraphicsItem *x, collidingItems(Qt::IntersectsItemShape)) {
        GKissItem *item = qgraphicsitem_cast<GKissItem *>(x);
        if (item == nullptr) { continue; }
        m_originalPixelOverlappedItems.insert(item);
    }
    m_originalBoundingRectOverlappedObjIds.clear();
    foreach (QGraphicsItem *x, collidingItems(Qt::IntersectsItemBoundingRect)) {
        GKissItem *item = qgraphicsitem_cast<GKissItem *>(x);
        if (item == nullptr) { continue; }
        m_originalBoundingRectOverlappedObjIds.insert(item->objId());
    }
}

QDebug operator<<(QDebug dbg, const GKissItem &item) {
    dbg.nospace().noquote() << QString("GKissItem { objId: #%1, \tcelFileName: '%2', \tpos: (%3, %4), \tvisible: %5 }")
                           .arg(item.objId())
                           .arg(item.celFileName())
                           .arg(item.pos().x())
                           .arg(item.pos().y())
                           .arg(item.isVisible());
    return dbg;
};


