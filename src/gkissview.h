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
    GKissView(ArchivedFile *archivedFile, QWidget *parent=nullptr);
    bool loadArchivedFile(ArchivedFile *archivedFile);
    void unloadArchivedFile();
    bool setCurCnf(QString cnfFileName);
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
    void deleteAllScenes();
    ArchivedFile *m_archivedFile;
    /// Each set is stored in its scene.
    QVector<GKissScene *> m_scenes;
    uint8_t m_curSceneIdx = 0;
    GKissItem *m_curHoveredCelItem;

    // QWidget interface

};

#endif // GKISSVIEW_H
