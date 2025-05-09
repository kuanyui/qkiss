#ifndef ITEMINSPECTOR_H
#define ITEMINSPECTOR_H

#include <QObject>
#include <QTreeWidget>

class GKissItem;

class ItemInspector : public QTreeWidget
{
    Q_OBJECT
public:
    explicit ItemInspector(QWidget *parent = nullptr);
    void setInspectedItem(GKissItem *item);
signals:
private:
    void appendKvRow(QTreeWidgetItem **item, const QString &icon, const QString &title);
    GKissItem *m_curItem;
    QTreeWidgetItem *m_twFix;
    QTreeWidgetItem *m_twObjId;
    QTreeWidgetItem *m_twCelPos;
    QTreeWidgetItem *m_twCelName;
    QTreeWidgetItem *m_twCelSize;
    QTreeWidgetItem *m_twCelComment;
    QTreeWidgetItem *m_twCelSiblings;
};

#endif // ITEMINSPECTOR_H
