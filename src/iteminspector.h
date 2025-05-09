#ifndef ITEMINSPECTOR_H
#define ITEMINSPECTOR_H

#include <QObject>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>


class GKissItem;

class ItemInspector : public QWidget
{
    Q_OBJECT
public:
    explicit ItemInspector(QWidget *parent = nullptr);
    void setInspectedItem(GKissItem *item);
signals:

private:
    void appendKvRow(QLabel **item, const QString &icon, const QString &title);
    GKissItem *m_curItem;
    QGridLayout *m_gridLayout;
    QLabel *m_twFix;
    QLabel *m_twObjId;
    QLabel *m_twCelPos;
    QLabel *m_twCelName;
    QLabel *m_twCelSize;
    QLabel *m_twCelComment;
    QLabel *m_twCelSiblings;
    QLabel *m_twCelPreview;

    // QWidget interface
protected:
};

#endif // ITEMINSPECTOR_H
