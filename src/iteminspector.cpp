#include "iteminspector.h"
#include "gkissitem.h"
#include <QHeaderView>

using namespace Qt::StringLiterals;

ItemInspector::ItemInspector(QWidget *parent)
    : QTableWidget{parent}
{
    setColumnCount(2);
    setWordWrap(true);
    verticalHeader()->setVisible(false);
    setHorizontalHeaderLabels({tr("Attribute"), tr("Value")});
    appendKvRow(&m_twCelName, "cel-file-name", tr("CEL File"));
    appendKvRow(&m_twObjId, "obj-id", tr("Object ID"));
    appendKvRow(&m_twCelPos, "obj-pos", tr("CEL Position"));
    appendKvRow(&m_twFix, "obj-fix-count", tr("Fix"));
    appendKvRow(&m_twCelSize, "cel-size", tr("CEL Size"));
    appendKvRow(&m_twCelComment, "comment", tr("CEL Comment"));
    appendKvRow(&m_twCelSiblings, "obj-cels", tr("CEL Siblings"));
    resizeRowsToContents();
}

#define narg(num) arg(QString::number(num))

void ItemInspector::setInspectedItem(GKissItem *item)
{
    m_curItem = item;
    if (item == nullptr) {
        m_twCelName->setText("");
        m_twObjId->setText("");
        m_twCelPos->setText("");
        m_twCelSize->setText("");
        m_twCelComment->setText("");
        m_twFix->setText("");
        m_twCelSiblings->setText("");
        resizeRowsToContents();
        return;
    }
    QStringList itemsOfObj;
    itemsOfObj.append(item->celFileName());
    foreach (auto sibling, *item->siblings()) {
        itemsOfObj.append(sibling->celFileName());
    }
    auto cel = item->cel();
    m_twCelName->setText(item->celFileName());
    m_twObjId->setText(QString::number(item->objId()));
    m_twCelPos->setText("(%%2)"_L1.narg(cel->x()).narg(cel->y()));
    m_twCelSize->setText("%1 x %2"_L1.narg(cel->width()).narg(cel->height()));
    m_twCelComment->setText(item->cnfCel()->comment());
    m_twFix->setText(QString::number(item->fix()));
    m_twCelSiblings->setText("(%1) %2"_L1
                             .narg(itemsOfObj.length())
                             .arg(itemsOfObj.join(", "))
                             );
    resizeRowsToContents();
}

void ItemInspector::appendKvRow(QTableWidgetItem **ppItem, const QString &icon, const QString &title)
{
    int r = rowCount();
    insertRow(r);
    auto labelItem = new QTableWidgetItem(QIcon::fromTheme(icon), title);
    labelItem->setTextAlignment(Qt::AlignLeft | Qt::AlignTop);
    (*ppItem) = new QTableWidgetItem();
    QTableWidgetItem *valueItem = (*ppItem);
    setItem(r, 0, labelItem);
    setItem(r, 1, valueItem);
}
