#ifndef GKISSSCENE_H
#define GKISSSCENE_H

#include <QGraphicsScene>
#include <QHash>
#include <QVector>
#include "archivedfile.h"
#include "gkissitem.h"

class GKissScene : public QGraphicsScene
{
    Q_OBJECT   // For qobject_cast
public:
    explicit GKissScene (QObject *parent = nullptr);
    // explicit GKissScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);
    ~GKissScene();
    /** Call this */
    void setEssentialContext(ArchivedFile *archivedFile, ParsedCnf* cnf, ParsedCnfSet *cnfSet);
    void drawCurSetBackground();
    void addCelItem(GKissItem *item);
    std::optional<QVector<GKissItem *> *> getItemsByObjId(int32_t objId);
    std::optional<GKissItem *> getItemByCelName(const QString &celFileName);
    void bindItemsToObjects();
    bool moveObjectTo(size_t objId, int x, int y);
    void receiveFkissUserEventFromSceneItem(FkissUserMovementEvent evtType, QVector<FkissVariant> vars);
    QList<GKissItem *> allKissItems();
    uint8_t paletteNoInKcf() const;
    void setPaletteNoInKcf(uint8_t newPaletteNoInKcf);

private:
    QHash<int32_t, QVector<GKissItem*>*> m_objIdToItemsMap;
    QHash<QString, GKissItem *> m_celNameToItemMap;
    ArchivedFile *m_archivedFile;
    ParsedCnf* m_cnf;
    ParsedCnfSet *m_cnfSet;
    /// Read from CNF file ($), and can be overritten from UI.
    uint8_t m_paletteNoInKcf = 0;
};

#endif // GKISSSCENE_H
