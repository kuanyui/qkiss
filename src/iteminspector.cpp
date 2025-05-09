#include "iteminspector.h"
#include "gkissitem.h"

using namespace Qt::StringLiterals;

ItemInspector::ItemInspector(QWidget *parent)
    : QTreeWidget{parent}
{
    setRootIsDecorated(false);
    setAlternatingRowColors(true);
    setColumnCount(2);
    setHeaderLabels({tr("Attribute"), tr("Value")});
    appendKvRow(&m_twCelName, "cel-file-name", tr("CEL File"));
    appendKvRow(&m_twObjId, "obj-id", tr("Object ID"));
    appendKvRow(&m_twCelPos, "obj-pos", tr("CEL Position"));
    appendKvRow(&m_twFix, "obj-fix-count", tr("Fix"));
    appendKvRow(&m_twCelSize, "cel-size", tr("CEL Size"));
    appendKvRow(&m_twCelComment, "comment", tr("CEL Comment"));
    appendKvRow(&m_twCelSiblings, "obj-cels", tr("CEL Siblings"));
}

#define narg(num) arg(QString::number(num))

void ItemInspector::setInspectedItem(GKissItem *item)
{
    m_curItem = item;
    if (item == nullptr) {
        m_twCelName->setText(1, "");
        m_twObjId->setText(1, "");
        m_twCelPos->setText(1, "");
        m_twCelSize->setText(1, "");
        m_twCelComment->setText(1, "");
        m_twFix->setText(1, "");
        m_twCelSiblings->setText(1, "");
        return;
    }
    QStringList itemsOfObj;
    itemsOfObj.append(item->celFileName());
    foreach (auto sibling, *item->siblings()) {
        itemsOfObj.append(sibling->celFileName());
    }
    auto cel = item->cel();
    m_twCelName->setText(1, item->celFileName());
    m_twObjId->setText(1, QString::number(item->objId()));
    m_twCelPos->setText(1, "(%1, %2)"_L1.narg(cel->x()).narg(cel->y()));
    m_twCelSize->setText(1, "%1 x %2"_L1.narg(cel->width()).narg(cel->height()));
    m_twCelComment->setText(1, item->cnfCel()->comment());
    m_twFix->setText(1, QString::number(item->fix()));
    m_twCelSiblings->setText(1, "(%1) %2"_L1
                             .narg(itemsOfObj.length())
                             .arg(itemsOfObj.join(", "))
                             );
}

void ItemInspector::appendKvRow(QTreeWidgetItem **ppItem, const QString &icon, const QString &title)
{
    (*ppItem) = new QTreeWidgetItem {this};
    QTreeWidgetItem *item = (*ppItem);
    item->setIcon(0, QIcon::fromTheme(icon));
    item->setText(0, title);
    // item->setFlags(item->flags() & (~Qt::ItemFlag::ItemIsEnabled));
    addTopLevelItem(item);
}
