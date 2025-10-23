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

GKissView::GKissView(ArchiveFile *archiveFile, QWidget *parent):
    QGraphicsView(parent)
{
    setArchiveFileAndRenderFirstCnf(archiveFile);
}


bool GKissView::setArchiveFileAndRenderFirstCnf(ArchiveFile *archiveFile)
{
    // deleteAllScenes();  /// FIXME: Unnecessary? Should not be done here?
    m_archiveFile = archiveFile;
    while (m_connectionsOfArchiveFile.length()) {
        disconnect(m_connectionsOfArchiveFile.takeLast());
    }
    // NOTE: 1. I also have considered to move this connect() into ArchiveFile itself but the semantic of context looks too weird....
    // NOTE: 2. When loading a new LZH file, should trigger both event after fkiss runtime is ready.
    m_connectionsOfArchiveFile.append(connect(m_archiveFile, &ArchiveFile::fkissChangedCnfSetNo, [this](uint8_t newCurCnfSetNo) {
        m_archiveFile->curCnf()->fkiss()->runtime()->triggerSet(newCurCnfSetNo);
    }));
    m_connectionsOfArchiveFile.append(connect(m_archiveFile, &ArchiveFile::fkissChangedPaletteNo, [this](uint8_t newCurPaletteNo) {
        m_archiveFile->curCnf()->fkiss()->runtime()->triggerCol(newCurPaletteNo);
    }));
    CPStringList cnfFileNameList = m_archiveFile->getCnfNameList();
    if (cnfFileNameList.isEmpty()) {
        qCritical().noquote() << tr("No CNF file found in this archive file.");
        return false;
    }
    setCurCnfAndRenderScenes(cnfFileNameList.first());
    return true;
}

void GKissView::deleteAllScenes()
{
    if (m_archiveFile != nullptr) {
        auto cnfs = m_archiveFile->parsedCnfFiles();
        for (auto it = cnfs->begin(); it != cnfs->end(); it++) {
            ParsedCnf *cnf = it.value();
            cnf->fkiss()->stopRuntime();
        }
    }
    setScene(nullptr);    // This is native `QGraphicsScene::setScene()`
    foreach (GKissScene *s, m_scenes) {
        delete s;
    }
    m_scenes.clear();
}

// FIXME: separate setCurCnfAndRenderScenes(), create startFkiss() / stopFkiss()
bool GKissView::setCurCnfAndRenderScenes(CPString cnfFileName)
{
    if (m_archiveFile->curCnf()) {
        m_archiveFile->curCnf()->fkiss()->stopRuntime();
    }
    bool ok = m_archiveFile->setCurCnfByName(cnfFileName);   // REFACTOR: So counterintuitive... Why do this in View???
    if (!ok) {
        qCritical().noquote() << tr(R"(No CNF file named "%1" found in this archive file.)").arg(cnfFileName.original());
        return false;
    }
    qDebug() << "Set current cnfFileName as " << cnfFileName;
    ok = renderCurCnfAllSets();
    if (!ok) { return false; }
    if (m_scenes.length() == 0) { return false; }
    ok = showKissSceneByIndex(0);
    if (!ok) { return false; }
    setScaleModeForAllItems();
    m_archiveFile->curCnf()->fkiss()->startRuntime(curKissScene());
    m_archiveFile->curCnf()->fkiss()->runtime()->triggerSet(0);   // REFACTOR: triggering set(), col() in here is ... so fucking ugly.
    m_archiveFile->curCnf()->fkiss()->runtime()->triggerCol(curKissScene()->paletteNoInKcf());   // NOTE: The default paletteNo is defined in CNF, the lines starts with `$`.
    return true;
}

uint8_t GKissView::scenesAmount() const
{
    return m_scenes.length();
}

bool GKissView::showKissSceneByIndex(uint8_t setIndex)
{
    if (scenesAmount() == 0) {
        qWarning().noquote() << tr("Only %1 scenes in this view but you ask for sceneIndex %2")
                      .arg(scenesAmount())
                      .arg(setIndex);
        return false;
    }
    if (setIndex >= scenesAmount()) {
        qWarning().noquote() << tr("Only %1 scenes in this view but you ask for sceneIndex %2")
                      .arg(scenesAmount())
                      .arg(setIndex);
        m_curSceneIdx = 0;
        setScene(m_scenes[0]);   // This is native `QGraphicsScene::setScene()`
        return false;
    }
    m_curSceneIdx = setIndex;
    GKissScene *kissScene = m_scenes[setIndex];
    setScene(kissScene);   // This is native `QGraphicsScene::setScene()`
    m_archiveFile->curCnf()->fkiss()->runtime()->setCurKissScene(kissScene);
    return true;
}

void GKissView::scaleAbs(qreal targetScale)
{
    qreal currentScale = transform().m11();
    qreal scaleFactor = targetScale / currentScale;
    scale(scaleFactor, scaleFactor);
    setScaleModeForAllItems();
}

void GKissView::setScaleModeForAllItems()
{
    qreal currentScale = transform().m11();
    using RH = QPainter::RenderHint;
    if (currentScale == std::floor(currentScale)) {   // is integer
        setRenderHints(RH::TextAntialiasing);
        foreach (auto scene , m_scenes) {
            foreach (auto item , scene->allKissItems()) {
                item->setTransformationMode(Qt::TransformationMode::FastTransformation);
            }
        }
    } else {
        setRenderHints(RH::TextAntialiasing | RH::Antialiasing | QPainter::SmoothPixmapTransform);
        foreach (auto scene , m_scenes) {
            foreach (auto item , scene->allKissItems()) {
                item->setTransformationMode(Qt::TransformationMode::SmoothTransformation);
            }
        }
    }
}

GKissScene *GKissView::curKissScene()
{
    return m_scenes[m_curSceneIdx];  // TODO: Check range.
}

bool GKissView::renderCurCnfAllSets()
{
    deleteAllScenes();
    ParsedCnf *cnf = m_archiveFile->curCnf();
    QStringList kcfFileNamesInCnf = cnf->kcfFileNames();
    QElapsedTimer elapsedTimer;
    elapsedTimer.start();
    for (int cnfSetIdx = 0; cnfSetIdx < cnf->sets().length(); cnfSetIdx++) {
        GKissScene *scene = new GKissScene(m_archiveFile, cnf, cnfSetIdx, this);
        connect(scene, &GKissScene::highlightedItemChanged, this, &GKissView::highlightedItemChanged);   // REFACTOR: naming is too ugly...
        scene->drawCurSetBackground();
        m_scenes.append(scene);
        int sceneDefaultPaletteNo = scene->paletteNoInKcf();   // This default paletteNo is defined in CNF; that is the lines starts with `$`. `SPALETTE_NO`
        // Create cels
        for (int celIdx = cnf->cels().length() - 1; celIdx >= 0; celIdx--) {   // The first CEL in CNF is on the top of all CELs. So render from the last CEL to the first one.
            ParsedCnfCel *cnfCel = cnf->cels()[celIdx];
            if (!cnfCel->setsIndexes().contains(cnfSetIdx)) {
                continue;
            }
            auto _cel = m_archiveFile->getParsedCel(cnfCel->celFileName());
            if (!_cel.has_value()) {
                qCritical().noquote() << tr(R"(CNF requires a CEL "%1", but not found in this archived file: )")
                               .arg(cnfCel->celFileName().original());
                continue;
            }
            ParsedCel *cel = _cel.value();
            GKissItem *item;
            if (cel->celType() == ParsedCel::Type::PaletteBased) {
                auto _kcf = m_archiveFile->getParsedKcfByOrderedIndexInCurrentCnf(cnfCel->kcfFileIndex());
                if (!_kcf.has_value()) {
                    qCritical().noquote() << tr(R"(In CNF "%1", the CEL "%2" requires KCF index %3, but no KCF file available in this CNF.)")
                                   .arg(cnf->fileName().original())
                                   .arg(cnfCel->celFileName().original())
                                   .arg(cnfCel->kcfFileIndex());
                    continue;
                }
                ParsedKcf *kcf = _kcf.value();
                item = new GKissItem(cnfCel, cel, kcf, sceneDefaultPaletteNo);
            } else if (cel->celType() == ParsedCel::Type::FullColor) {
                item = new GKissItem(cnfCel, cel);
            } else {
                qCritical().noquote() << QString(R"([View] Unknown CEL type (not PaletteBased nor ColorBased), skip: "%1")")
                                .arg(cnfCel->celFileName().original());
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
    qDebug().noquote() << tr("[Benchmark] Creating all graphics items in %1 scene(s) cost %2 ms").arg(cnf->sets().length()).arg(elapsedTimer.elapsed());
    if (m_scenes.length() == 0) {
        qCritical().noquote() << tr("No scene found. Aborted");
        return false;
    }
    return true;
}

void GKissView::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "[DEBUG] View::mousePressEvent() event->ignore()";
    switch (event->button()) {
    case Qt::MouseButton::BackButton:
        emit requestSwitchingSet(-1);
        break;
    case Qt::MouseButton::ForwardButton:
        emit requestSwitchingSet(1);
        break;
    default:
        // (In Qt, the event handling ordering is View -> Scene -> Item)
        QGraphicsView::mousePressEvent(event);  // Do what base class does (pass event to scene).
        break;
    }
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
