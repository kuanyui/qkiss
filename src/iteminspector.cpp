#include "iteminspector.h"
#include "gkissitem.h"
#include <QHeaderView>

using namespace Qt::StringLiterals;

ItemInspector::ItemInspector(QWidget *parent)
    : QWidget{parent},
    m_layout {new QGridLayout(this)}
{
    static auto titleStyle = "font-weight: bold; color: gray;"_L1;
    auto attr = new QLabel(tr("Attribute"));
    auto val = new QLabel(tr("Value"));
    attr->setStyleSheet(titleStyle);
    val->setStyleSheet(titleStyle);
    m_layout->addWidget(attr, 0, 1);
    m_layout->addWidget(val, 0, 2);
    appendKvRow(&m_twCelName, "cel-file-name", tr("CEL File"));
    appendKvRow(&m_twObjId, "obj-id", tr("Object ID"));
    appendKvRow(&m_twCelPos, "obj-pos", tr("CEL Position"));
    appendKvRow(&m_twFix, "obj-fix-count", tr("Fix"));
    appendKvRow(&m_twCelSize, "cel-size", tr("CEL Size"));
    appendKvRow(&m_twCelComment, "comment", tr("CEL Comment"));
    appendKvRow(&m_twCelPreview, "cel-file-preview", tr("CEL Preview"));
    appendKvRow(&m_twCelSiblings, "obj-cels", tr("CEL Siblings"));
    m_layout->setRowStretch(m_layout->rowCount(), 1);
}

#define argn(num) arg(QString::number(num))

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
        m_twCelPreview->clear();
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
    m_twCelPos->setText("(%1, %2)"_L1.argn(cel->x()).argn(cel->y()));
    m_twCelSize->setText("%1 x %2"_L1.argn(cel->width()).argn(cel->height()));
    m_twCelComment->setText(item->cnfCel()->comment());
    m_twFix->setText(QString::number(item->fix()));
    m_twCelSiblings->setText("(%1) %2"_L1
                             .argn(itemsOfObj.length())
                             .arg(itemsOfObj.join(", "))
                             );
    QPixmap pixmap = item->pixmap();
    int targetSize = qMin(pixmap.width(), pixmap.height());
    // TODO: Scale according current zooming
    targetSize = qMin(targetSize, m_twCelPreview->width());
    m_twCelPreview->setPixmap(pixmap.scaled(targetSize, targetSize, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
}

void ItemInspector::appendKvRow(QLabel **ppLabel, const QString &icon, const QString &title)
{
    QFont defaultFont = QApplication::font();
    QFontMetrics metrics(defaultFont);
    int h = metrics.height();

    int r = m_layout->rowCount();
    auto iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon::fromTheme(icon).pixmap(h, h));
    iconLabel->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
    m_layout->addWidget(iconLabel, r, 0, Qt::AlignTop);

    auto keyLabel = new QLabel(this);
    keyLabel->setText(title);
    keyLabel->setStyleSheet("font-weight: bold; color: gray;"_L1);
    keyLabel->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
    m_layout->addWidget(keyLabel, r, 1, Qt::AlignTop);


    (*ppLabel) = new QLabel(this);
    QLabel *valueLabel = (*ppLabel);
    valueLabel->setWordWrap(true);
    valueLabel->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Minimum);
    m_layout->addWidget(valueLabel, r, 2, Qt::AlignTop);

}
