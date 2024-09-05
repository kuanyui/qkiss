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
    return renderCurCnfAllSets();
}

bool GKissView::showSceneByIndex(uint8_t setIndex)
{
    if (m_scenes.length() == 0) {
        return false;
    }
    if (setIndex >= m_scenes.length()) {
        m_curSceneIdx = 0;
        setScene(m_scenes[0]);
        return false;
    }
    m_curSceneIdx = setIndex;
    setScene(m_scenes[setIndex]);
    return true;
}

GKissScene *GKissView::curScene()
{
    return m_scenes[m_curSceneIdx];  // TODO: Check range.
}

bool GKissView::renderCurCnfAllSets()
{
    m_scenes.clear();
    ParsedCnf *cnf = m_archivedFile->curCnf();
    QStringList kcfFileNamesInCnf = cnf->kcfFileNames();
    for (ParsedCnfSet *cnfSet : cnf->sets()) {
        GKissScene *scene = new GKissScene(0 ,0, cnf->playFieldSize().width(), cnf->playFieldSize().height(), this);
        scene->drawBackaground(m_archivedFile, cnfSet);
        m_scenes.append(scene);
        // Create cels
        for (int celIdx = cnf->cels().length() - 1; celIdx >= 0; celIdx--) {
            ParsedCnfCel *cnfCel = cnf->cels()[celIdx];
            auto objPos = cnfSet->getObjPosByObjId(cnfCel->objId());
            if (!objPos.has_value()) {
                // This cel is a part of an object, but this object is not used in this cnfSet, so skip.
                continue;
            }
            // auto _celPos = cnfSet->getObjPosList()[celIdx];
            // QPoint celPos = _celPos.has_value() ? _celPos.value() : QPoint();
            auto _cel = m_archivedFile->getParsedCel(cnfCel->celFileName());
            if (!_cel.has_value()) {
                qCritical() << "[ERROR] CNF requires a CEL, but no such CEL file found in this archived file: "_L1 << cnfCel->celFileName();
                continue;
            }
            ParsedCel *cel = _cel.value();
            GKissItem *item;
            if (cel->celType() == ParsedCel::Type::PaletteBased) {
                auto _kcf = m_archivedFile->getParsedKcfByOrderedIndexInCurrentCnf(cnfCel->kcfFileIndex());
                if (!_kcf.has_value()) {
                    qCritical() << "[ERROR] In CNF, the CEL '%1' requires KCF index $2, but no such KCF file found in this archived file."_L1
                                   .arg(cnfCel->celFileName())
                                   .arg(cnfCel->kcfFileIndex());
                    continue;
                }
                ParsedKcf *kcf = _kcf.value();
                item = new GKissItem(cnfCel, cel, kcf, m_archivedFile->curPaletteNo());
            } else if (cel->celType() == ParsedCel::Type::ColorBased) {
                item = new GKissItem(cnfCel, cel);
            }
            item->setPos(cel->x(), cel->y());
            scene->addCelItem(item);
            centerOn(item);
            qDebug() << "[DEBUG][View] addItem" << item->celFileName() << item->pos() << item->boundingRect() << item->isVisible() << scene->items().contains(item);
        }
        qDebug() << "[DEBUG][View] scene.items().length() ==" << scene->items().length();
        // All CEL are rendered as items. Now bound items into obj (according to objId), and handle events.
        bindItemsToObjects(scene, cnfSet);

    }
    return showSceneByIndex(0);
}

void GKissView::bindItemsToObjects(GKissScene *scene, ParsedCnfSet *cnfSet)
{
    auto objPosList = cnfSet->getObjPosList();
    for (size_t objId = 0; objId < objPosList.length(); objId++) {
        auto _objPos = objPosList[objId];
        if (!_objPos.has_value()) {
            qWarning() << "[WARNING][View] This should not happened. Should be excluded before this step.";
            continue;
        }
        auto objPos = _objPos.value();
        auto items = scene->getItemsByObjId(objId);
        for (GKissItem *item : *items) {
            item->setPos(objPos.x() + item->cel()->x(),
                         objPos.y() + item->cel()->y());
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
                                                 .arg(-1)
                                                 .arg(-1)
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
