#ifndef GKISSVIEW_H
#define GKISSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QObject>
#include <QWidget>
#include "gkissscene.h"
#include "gkissitem.h"

class GKissView : public QGraphicsView
{
    Q_OBJECT;
public:
    GKissView(QWidget *parent=nullptr);
    GKissView(ArchiveFile *archiveFile, QWidget *parent=nullptr);
    bool setArchiveFile(ArchiveFile *archiveFile);
    void deleteAllScenes();
    bool setCurCnfAndRenderScenes(QString cnfFileName);
    uint8_t scenesAmount() const;
    bool showKissSceneByIndex(uint8_t setIndex);
    void scaleAbs(qreal targetScale);
    GKissScene *curKissScene();
    /** create scenes & items according to current CNF. */
    bool renderCurCnfAllSets();
    // bool renderCurCnfSet(uint8_t setNo);

signals:
    /// e.g. hovered item changed, no longer hovering any item (pass nullptr)... etc.
    void highlightedItemChanged(GKissItem *item);
protected:
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
private:
    ArchiveFile *m_archiveFile;
    /// Each set is stored in its scene.
    QVector<GKissScene *> m_scenes;
    /// REFACTOR: Wait... Why not just use `ArchiveFile::curCnfSetNo` ???
    uint8_t m_curSceneIdx = 0;
    GKissItem *m_curHoveredCelItem;
    QList<QMetaObject::Connection> m_connectionsOfArchiveFile;

    // QWidget interface

};

#endif // GKISSVIEW_H
