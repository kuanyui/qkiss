#include "gkissview.h"
#include "gkissitem.h"
#include "parsedcnf.h"
#include "parsedkcf.h"
#include "parsedcel.h"
#include <QString>
#include <QMouseEvent>

using namespace Qt::Literals::StringLiterals;

GKissView::GKissView(QWidget *parent):
    QGraphicsView(parent)
{

}

GKissView::GKissView(ArchivedFile *archivedFile, QWidget *parent):
    QGraphicsView(parent)
{
    loadArchivedFile(archivedFile);
}


bool GKissView::loadArchivedFile(ArchivedFile *archivedFile)
{
    m_archivedFile = archivedFile;
    QStringList cnfFileNameList = m_archivedFile->getCnfNameList();
    if (cnfFileNameList.isEmpty()) {
        qCritical() << "[ERROR][View] No CNF file found in this archivedFile.";
        return false;
    }
    setCurCnf(cnfFileNameList.first());
    return true;
}

void GKissView::unloadArchivedFile()
{
    deleteAllScenes();
    // m_archivedFile = nullptr;   // DON'T DO THIS!
}

void GKissView::deleteAllScenes()
{
    setScene(nullptr);
    for (GKissScene *s : m_scenes) {
        delete s;
    }
    m_scenes.clear();
}

bool GKissView::setCurCnf(QString cnfFileName)
{
    bool ok = m_archivedFile->setCurCnf(cnfFileName);
    if (!ok) {
        qCritical() << "[ERROR][View] No CNF file named " << cnfFileName << " found in this archivedFile.";
        return false;
    }
    qDebug() << "[LOG][View] Set currtent cnfFileName as " << cnfFileName;
    return renderCurCnfAllSets();
}

uint8_t GKissView::scenesAmount() const
{
    return m_scenes.length();
}

bool GKissView::showSceneByIndex(uint8_t setIndex)
{
    if (scenesAmount() == 0) {
        qWarning() << QString("[WARNING][View] only $1 scenes in this view but you ask for sceneIndex $2")
                      .arg(scenesAmount())
                      .arg(setIndex);
        return false;
    }
    if (setIndex >= scenesAmount()) {
        qWarning() << QString("[WARNING][View] only $1 scenes in this view but you ask for sceneIndex $2")
                      .arg(scenesAmount())
                      .arg(setIndex);
        m_curSceneIdx = 0;
        setScene(m_scenes[0]);
        return false;
    }
    m_curSceneIdx = setIndex;
    setScene(m_scenes[setIndex]);
    return true;
}

void GKissView::scaleAbs(qreal targetScale)
{
    qreal currentScale = transform().m11();
    qreal scaleFactor = targetScale / currentScale;
    scale(scaleFactor, scaleFactor);
}

GKissScene *GKissView::curScene()
{
    return m_scenes[m_curSceneIdx];  // TODO: Check range.
}

bool GKissView::renderCurCnfAllSets()
{
    deleteAllScenes();
    ParsedCnf *cnf = m_archivedFile->curCnf();
    QStringList kcfFileNamesInCnf = cnf->kcfFileNames();
    for (size_t cnfSetIdx = 0; cnfSetIdx < cnf->sets().length(); cnfSetIdx++) {
        ParsedCnfSet *cnfSet = cnf->sets().at(cnfSetIdx);
        GKissScene *scene = new GKissScene(this);
        scene->setEssentialContext(m_archivedFile, cnf, cnfSet);
        scene->drawCurSetBackground();
        m_scenes.append(scene);
        // Create cels
        for (int celIdx = cnf->cels().length() - 1; celIdx >= 0; celIdx--) {
            ParsedCnfCel *cnfCel = cnf->cels()[celIdx];
            if (!cnfCel->setsIndexes().contains(cnfSetIdx)) {
                continue;
            }
            auto _cel = m_archivedFile->getParsedCel(cnfCel->celFileName());
            if (!_cel.has_value()) {
                qCritical() << QStringLiteral("[ERROR] CNF requires a CEL '%1', but not found in this archived file: ")
                               .arg(cnfCel->celFileName());
                continue;
            }
            ParsedCel *cel = _cel.value();
            GKissItem *item;
            if (cel->celType() == ParsedCel::Type::PaletteBased) {
                auto _kcf = m_archivedFile->getParsedKcfByOrderedIndexInCurrentCnf(cnfCel->kcfFileIndex());
                if (!_kcf.has_value()) {
                    qCritical() << "[ERROR] In CNF '%1', the CEL '%2' requires KCF index %3, but no KCF file available in this CNF."_L1
                                   .arg(cnf->fileName())
                                   .arg(cnfCel->celFileName())
                                   .arg(cnfCel->kcfFileIndex());
                    continue;
                }
                ParsedKcf *kcf = _kcf.value();
                item = new GKissItem(cnfCel, cel, kcf, m_archivedFile->curPaletteNo());
            } else if (cel->celType() == ParsedCel::Type::ColorBased) {
                item = new GKissItem(cnfCel, cel);
            } else {
                qCritical() << QString("[ERROR][View] Unknown CEL type (not PaletteBased nor ColorBased), skip: %1")
                               .arg(cnfCel->celFileName());
                continue;
            }
            item->setPos(cel->x(), cel->y());
            // qDebug() << QString("[DEBUG][View] setIdx = %1, cel = %2") .arg(cnfSetIdx) .arg(cnfCel->celFileName());
            scene->addCelItem(item);
            centerOn(item);
            qDebug() << QString("[DEBUG][View] addItem ID #%1    '%2'    (%3, %4)  Visible = %5")
                        .arg(item->objId())
                        .arg(item->celFileName())
                        .arg(item->pos().x())
                        .arg(item->pos().y())
                        .arg(item->isVisible());
        }
        //qDebug() << "[DEBUG][View] scene.items().length() ==" << scene->items().length();
        // All CEL are rendered as items. Now bound items into obj (according to objId), and handle events.
        bindItemsToObjects(scene, cnfSet);

    }
    return showSceneByIndex(0);
}

void GKissView::bindItemsToObjects(GKissScene *scene, ParsedCnfSet *cnfSet)
{
    QList<ParsedCnfSetObjPos> objPosList = cnfSet->getObjPosList();
    for (size_t objId = 0; objId < objPosList.length(); objId++) {
        qDebug() << QString("objPosList.length = %1 , bind object id %2")
                    .arg(objPosList.length())
                    .arg(objId);
        auto _objPos = objPosList[objId];
        QPoint objPos = _objPos.has_value() ? _objPos.value() : QPoint(0, 0);
        auto _items = scene->getItemsByObjId(objId);
        if (!_items.has_value()) {
            qDebug() << QStringLiteral("[WARNING][View] This CNF's set definitions (starts with $ in CNF) has objects amounts greater than it's actually objects count. This is because in CNF's CEL lines, object IDs is not continuous/subsequent and has some gaps. This is a not a matter, but not a good practice.");
            continue;
        }
        auto items = _items.value();
        int32_t maxFix = items->isEmpty() ? 0 : items->at(0)->fix();
        for (GKissItem *item : *items) {
            maxFix = qMax(item->fix(), maxFix);
            item->setPos(objPos.x() + item->cel()->x(),
                         objPos.y() + item->cel()->y());
        }
        // The fix value for an object is the maximum value for all member cels.
        for (GKissItem *item : *items) {
            item->setFix(maxFix);
        }
    }
}

void GKissView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    QGraphicsItem *curItemRaw = scene()->itemAt(scenePos, QTransform());
    if (curItemRaw != m_curHoveredCelItem) {
        if (curItemRaw != nullptr && curItemRaw->type() == GKissItem::Type) {
            GKissItem *curItem = static_cast<GKissItem *>(curItemRaw);
            m_curHoveredCelItem = curItem;
            emit hoveredItemChanged(curItem, curScene());
            qDebug() << QString("File: %1, Pos: (%2, %3), Size: (%4, %5), Fix: %6")
                                                 .arg(curItem->celFileName())
                                                 .arg(curItem->x())
                                                 .arg(curItem->y())
                                                 .arg(curItem->cel()->width())
                                                 .arg(curItem->cel()->height())
                                                 .arg(curItem->fix());
        } else {
            emit hoveredItemChanged(nullptr, curScene());
            m_curHoveredCelItem = nullptr;
        }
    }
    // Handle other events
    QGraphicsView::mouseMoveEvent(event);
}
