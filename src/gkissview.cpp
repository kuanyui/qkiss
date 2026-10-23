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
#include "logging.h"
#include "crtfilter.h"
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
        qkCritical.noquote() << tr("No CNF file found in this archive file.");
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
        qkCritical.noquote() << tr(R"(No CNF file named "%1" found in this archive file.)").arg(cnfFileName.original());
        return false;
    }
    qkDebug << "Set current cnfFileName as " << cnfFileName;
    ok = renderCurCnfAllSets();
    if (!ok) { return false; }
    if (m_scenes.length() == 0) { return false; }
    ok = showKissSceneByIndex(0);
    if (!ok) { return false; }
    setScaleModeForAllItems();
    m_archiveFile->curCnf()->fkiss()->startRuntime(m_scenes, curKissScene());
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
        qkWarning.noquote() << tr("Only %1 scenes in this view but you ask for sceneIndex %2")
                      .arg(scenesAmount())
                      .arg(setIndex);
        return false;
    }
    if (setIndex >= scenesAmount()) {
        qkWarning.noquote() << tr("Only %1 scenes in this view but you ask for sceneIndex %2")
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

void GKissView::setCrtFilterEnabled(bool enabled)
{
    m_crtFilterEnabled = enabled;
    // With the filter, a changed pixel also changes the glow around it. So repaint the whole viewport, not only the
    // changed items. This also makes scrolling repaint everything, instead of moving the old pixels.
    // Without the filter, go back to MinimalViewportUpdate, the default of QGraphicsView.
    setViewportUpdateMode(enabled ? FullViewportUpdate : MinimalViewportUpdate);
    viewport()->update();
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
                qkCritical.noquote() << tr(R"(CNF requires a CEL "%1", but not found in this archived file: )")
                               .arg(cnfCel->celFileName().original());
                continue;
            }
            ParsedCel *cel = _cel.value();
            GKissItem *item;
            if (cel->celType() == ParsedCel::Type::PaletteBased) {
                auto _kcf = m_archiveFile->getParsedKcfByOrderedIndexInCurrentCnf(cnfCel->kcfFileIndex());
                if (!_kcf.has_value()) {
                    qkCritical.noquote() << tr(R"(In CNF "%1", the CEL "%2" requires KCF index %3, but no KCF file available in this CNF.)")
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
                qkCritical.noquote() << QString(R"([View] Unknown CEL type (not PaletteBased nor ColorBased), skip: "%1")")
                                .arg(cnfCel->celFileName().original());
                continue;
            }
            item->setPos(cel->x(), cel->y());
            // qkDebug << QString("[DEBUG][View] setIdx = %1, cel = %2") .arg(cnfSetIdx) .arg(cnfCel->celFileName());
            scene->addCelItem(item);
            centerOn(item);
            // qkDebug << "[DEBUG][View] addItem" << *item;
        }
        //qkDebug << "[DEBUG][View] scene.items().length() ==" << scene->items().length();
        // All CEL are rendered as items. Now bound items into obj (according to objId), and handle events.
        scene->bindItemsToObjects();
    }
    qkDebug.noquote() << tr("[Benchmark] Creating all graphics items in %1 scene(s) cost %2 ms").arg(cnf->sets().length()).arg(elapsedTimer.elapsed());
    if (m_scenes.length() == 0) {
        qkCritical.noquote() << tr("No scene found. Aborted");
        return false;
    }
    return true;
}

void GKissView::mousePressEvent(QMouseEvent *event)
{
    qkDebug << "[DEBUG] View::mousePressEvent() event->ignore()";
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

void GKissView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->angleDelta().y() > 0) {
            emit requestZoomIn();
        } else {
            emit requestZoomOut();
        }
        event->accept();
        return;
    }
    QGraphicsView::wheelEvent(event);
}

void GKissView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);
    if (!m_crtFilterEnabled || scene() == nullptr) { return; }
    // NOTE: QGraphicsView::paintEvent() has already painted the background and the items, and the filtered image covers
    // them. Don't override paintEvent() to skip that work. QGraphicsView::paintEvent() resets the "update all" flag of
    // the scene, which QGraphicsScene::update() sets (for example in setBackgroundBrush()). While that flag is set, the
    // scene ignores changed items, so without QGraphicsView::paintEvent() the view would stop updating.
    drawCrtFilteredItems(painter, rect);
    CrtFilter::drawPhosphorStripes(painter, rect, transform().m11() * viewport()->devicePixelRatioF());
}

void GKissView::drawCrtFilteredItems(QPainter *painter, const QRectF &rect)
{
    // A pixel changes only when a pixel of another color is within CrtFilter::REACH. One more pixel keeps the edge of
    // the filtered image unchanged, so it matches the background around it.
    const int margin = CrtFilter::REACH + 1;
    const QRectF area = rect.adjusted(-margin, -margin, margin, margin);   // Pixels just outside `rect` can glow into it.
    // The bounding rectangle of the visible items in `area`. (QGraphicsScene::items() skips invisible items.)
    QRectF itemsRect;
    for (const QGraphicsItem *item : scene()->items(area, Qt::IntersectsItemBoundingRect)) {
        itemsRect |= item->sceneBoundingRect();
    }
    if (itemsRect.isEmpty()) { return; }
    // Away from the items, there is only the flat background, which the filter doesn't change.
    const QRect sourceRect = (itemsRect.adjusted(-margin, -margin, margin, margin) & area).toAlignedRect();
    // A very large area (e.g. an item dragged far away at a very small zoom) would need too much memory.
    constexpr qsizetype MAX_PIXELS = 2048 * 2048;
    if (qsizetype(sourceRect.width()) * sourceRect.height() > MAX_PIXELS) { return; }
    // Filter at the resolution of the KiSS set (1 image pixel = 1 KiSS pixel), so the effect looks the same at any zoom.
    QImage frame(sourceRect.size(), QImage::Format_RGB32);
    if (frame.isNull()) { return; }
    frame.fill(viewport()->palette().color(viewport()->backgroundRole()));   // Shown where the scene has no background brush
    QPainter framePainter(&frame);
    scene()->render(&framePainter, frame.rect(), sourceRect);
    framePainter.end();
    CrtFilter::apply(frame);
    // Scaled like the items: smoothly only at a zoom that is not a whole number. See setScaleModeForAllItems().
    painter->drawImage(QRectF(sourceRect), frame);
}
