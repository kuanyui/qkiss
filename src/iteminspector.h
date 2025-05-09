#ifndef ITEMINSPECTOR_H
#define ITEMINSPECTOR_H

#include <QObject>
#include <QTableWidget>

class GKissItem;

class ItemInspector : public QTableWidget
{
    Q_OBJECT
public:
    explicit ItemInspector(QWidget *parent = nullptr);
    void setInspectedItem(GKissItem *item);
signals:
private:
    void appendKvRow(QTableWidgetItem **item, const QString &icon, const QString &title);
    GKissItem *m_curItem;
    QTableWidgetItem *m_twFix;
    QTableWidgetItem *m_twObjId;
    QTableWidgetItem *m_twCelPos;
    QTableWidgetItem *m_twCelName;
    QTableWidgetItem *m_twCelSize;
    QTableWidgetItem *m_twCelComment;
    QTableWidgetItem *m_twCelSiblings;
};

#endif // ITEMINSPECTOR_H
