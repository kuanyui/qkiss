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
        setScene(m_scenes[0]);
        return false;
    }
    setScene(m_scenes[setIndex]);
    return true;
}

bool GKissView::renderCurCnfAllSets()
{
    m_scenes.clear();
    ParsedCnf *cnf = m_archivedFile->curCnf();
    QStringList kcfFileNamesInCnf = cnf->kcfFileNames();
    for (ParsedCnfSet *cnfSet : cnf->sets()) {
        GKissScene *scene = new GKissScene(0 ,0, cnf->playFieldSize().width(), cnf->playFieldSize().height(), this);
        m_scenes.append(scene);
        // Create play field rect bg
        ParsedKcf *firstKcf = m_archivedFile->getParsedKcfByOrderedIndexInCurrentCnf(0).value();  // TODO: nullity check
        ParsedKcfPalette *firstPalette = firstKcf->getPaletteInKcf(m_archivedFile->curPaletteNo()); // TODO: nullity checking
        QRgb borderColor = (*firstPalette)[cnf->outsideBorderColor()];  // TODO: nullity checking
        scene->setBackgroundBrush(QBrush(borderColor));
        // auto r = cnf->playFieldSize();
        // QGraphicsRectItem *playRect = new QGraphicsRectItem(0, 0, r.width(), r.height());
        // playRect->setBrush(QBrush());
        // scene->addItem(playRect);
        // Create cels
        for (int celIdx = cnf->cels().length() - 1; celIdx >= 0; celIdx--) {
            ParsedCnfCel *cnfCel = cnf->cels()[celIdx];
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
            // item->setPos(250, 250);
            scene->addItem(item);
        }
    }
    return showSceneByIndex(0);
}

void GKissView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    QGraphicsItem *curItemRaw = scene()->itemAt(scenePos, QTransform());
    if (curItemRaw == m_curHoveredItem) {
        return;
    }
    if (curItemRaw == nullptr) {
        emit hoveredItemChanged(nullptr);
        return;
    }
    if (curItemRaw->type() == GKissItem::Type) {
        GKissItem *curItem = static_cast<GKissItem *>(curItemRaw);
        m_curHoveredItem = curItem;
        emit hoveredItemChanged(curItem);
        qDebug() << QString("File: %1, Pos: (%2, %3), Size: (%4, %5), Fix: %6")
                                             .arg(curItem->celFileName())
                                             .arg(-1)
                                             .arg(-1)
                                             .arg(curItem->cel()->width())
                                             .arg(curItem->cel()->height())
                                             .arg(curItem->fix());
    }
    // 繼續處理其他滑鼠事件
    QGraphicsView::mouseMoveEvent(event);
}
