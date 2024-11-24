#include "gkissview.h"
#include "gkissitem.h"
#include "parsedcnf.h"
#include "parsedkcf.h"
#include "parsedcel.h"
#include "fkissruntime.h"
#include <QString>
#include <QMouseEvent>

using namespace Qt::Literals::StringLiterals;

GKissView::GKissView(QWidget *parent):
    QGraphicsView(parent)
{

}

GKissView::GKissView(ArchiveFile *archivedFile, QWidget *parent):
    QGraphicsView(parent)
{
    setArchiveFile(archivedFile);
}


bool GKissView::setArchiveFile(ArchiveFile *archivedFile)
{
    // deleteAllScenes();  /// FIXME: Unnecessary? Should not be done here?
    m_archivedFile = archivedFile;
    QStringList cnfFileNameList = m_archivedFile->getCnfNameList();
    if (cnfFileNameList.isEmpty()) {
        qCritical() << "[ERROR][View] No CNF file found in this archivedFile.";
        return false;
    }
    setCurCnf(cnfFileNameList.first());
    return true;
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
    if (m_archivedFile->curCnf()) {
        m_archivedFile->curCnf()->fkiss()->stopRuntime();
    }
    bool ok = m_archivedFile->setCurCnfByName(cnfFileName);
    if (!ok) {
        qCritical() << "[ERROR][View] No CNF file named " << cnfFileName << " found in this archivedFile.";
        return false;
    }
    qDebug() << "[LOG][View] Set current cnfFileName as " << cnfFileName;
    ok = renderCurCnfAllSets();
    if (!ok) { return false; }
    if (m_scenes.length() == 0) { return false; }
    ok = showKissSceneByIndex(0);
    if (!ok) { return false; }
    m_archivedFile->curCnf()->fkiss()->startRuntime(curKissScene());
    return true;
}

uint8_t GKissView::scenesAmount() const
{
    return m_scenes.length();
}

bool GKissView::showKissSceneByIndex(uint8_t setIndex)
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
    GKissScene *kissScene = m_scenes[setIndex];
    setScene(kissScene);
    m_archivedFile->curCnf()->fkiss()->runtime()->setCurKissScene(kissScene);
    return true;
}

void GKissView::scaleAbs(qreal targetScale)
{
    qreal currentScale = transform().m11();
    qreal scaleFactor = targetScale / currentScale;
    scale(scaleFactor, scaleFactor);
}

GKissScene *GKissView::curKissScene()
{
    return m_scenes[m_curSceneIdx];  // TODO: Check range.
}

bool GKissView::renderCurCnfAllSets()
{
    deleteAllScenes();
    ParsedCnf *cnf = m_archivedFile->curCnf();
    QStringList kcfFileNamesInCnf = cnf->kcfFileNames();
    QElapsedTimer elapsedTimer;
    elapsedTimer.start();
    for (int cnfSetIdx = 0; cnfSetIdx < cnf->sets().length(); cnfSetIdx++) {
        ParsedCnfSet *cnfSet = cnf->sets().at(cnfSetIdx);
        GKissScene *scene = new GKissScene(cnfSetIdx, this);
        connect(scene, &GKissScene::highlightedItemChanged, this, &GKissView::highlightedItemChanged);   // REFACTOR: naming is too ugly...
        scene->setEssentialContext(m_archivedFile, cnf, cnfSet);   // FIXME: Is this really necessary? Why not do this in Scene's ctor directly?
        scene->drawCurSetBackground();
        m_scenes.append(scene);
        // Create cels
        for (int celIdx = cnf->cels().length() - 1; celIdx >= 0; celIdx--) {
            ParsedCnfCel *cnfCel = cnf->cels()[celIdx];
            // Empty setsIndexes means this CEL is used by all sets.
            if (!cnfCel->setsIndexes().isEmpty() && !cnfCel->setsIndexes().contains(cnfSetIdx)) {
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
            } else if (cel->celType() == ParsedCel::Type::FullColor) {
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
            // qDebug() << "[DEBUG][View] addItem" << *item;
        }
        //qDebug() << "[DEBUG][View] scene.items().length() ==" << scene->items().length();
        // All CEL are rendered as items. Now bound items into obj (according to objId), and handle events.
        scene->bindItemsToObjects();
    }
    qDebug().noquote() << QStringLiteral("[LOG][Benchmark][KissView] Creating all graphics items in %1 scene(s) cost %2 ms").arg(cnf->sets().length()).arg(elapsedTimer.elapsed());
    if (m_scenes.length() == 0) {
        qCritical() << QString("[ERROR][View] no scene found. Aborted");
        return false;
    }
    return true;
}



void GKissView::mouseMoveEvent(QMouseEvent *event)
{
    if (scene() == nullptr) { return; }
    QPointF scenePos = mapToScene(event->pos());
    QGraphicsItem *curItemRaw = scene()->itemAt(scenePos, QTransform());
    if (curItemRaw != m_curHoveredCelItem) {
        if (curItemRaw != nullptr && curItemRaw->type() == GKissItem::Type) {
            GKissItem *curItem = static_cast<GKissItem *>(curItemRaw);
            m_curHoveredCelItem = curItem;
            emit highlightedItemChanged(curItem);
        } else {
            m_curHoveredCelItem = nullptr;
            emit highlightedItemChanged(nullptr);
        }
    }
    // Handle other events
    QGraphicsView::mouseMoveEvent(event);
}

void GKissView::dragEnterEvent(QDragEnterEvent *event)
{
    // dragging LZH file into QMainWindow can load this LZH file, but QGraphicsView stops it so ignore this event here.
    event->ignore();
}

void GKissView::dropEvent(QDropEvent *event)
{
    // dragging LZH file into QMainWindow can load this LZH file, but QGraphicsView stops it so ignore this event here.
    event->ignore();
}
