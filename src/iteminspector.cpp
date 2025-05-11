#include "iteminspector.h"
#include "gkissitem.h"
#include <QHeaderView>
#include <QScrollBar>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

ItemInspector::ItemInspector(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout *vboxLayout = new QVBoxLayout{this};
    QScrollArea *scrollArea = new QScrollArea{};
    QWidget *scrolledWidget = new QWidget{};
    m_gridLayout = new QGridLayout();
    vboxLayout->addWidget(scrollArea);
    scrollArea->setWidget(scrolledWidget);
    scrolledWidget->setLayout(m_gridLayout);

    scrollArea->setWidgetResizable(true);  // Important!!! QLabel is invisible without this.
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_gridLayout->setColumnStretch(2, 1);    // Important!!!!!!!! Do NOT use QLabel::setSizePolicy() to expand, it breaks QScrollArea.
    // scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    // int scrollbarWidth = scrollArea->verticalScrollBar()->sizeHint().width();

    appendKvRow(&m_twCelName, "cel-file-name", tr("CEL File"));
    appendKvRow(&m_twObjId, "obj-id", tr("Object ID"));
    appendKvRow(&m_twCelPos, "obj-pos", tr("CEL Position"));
    appendKvRow(&m_twFix, "obj-fix-count", tr("Fix"));
    appendKvRow(&m_twCelSize, "cel-size", tr("CEL Size"));
    appendKvRow(&m_twCelComment, "comment", tr("CEL Comment"));
    appendKvRow(&m_twCelPreview, "cel-file-preview", tr("CEL Preview"));
    appendKvRow(&m_twCelSiblings, "obj-cels", tr("CEL Siblings"));
    m_gridLayout->setRowStretch(m_gridLayout->rowCount(), 1);
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
    m_twCelPos->setText("(%1, %2)"_L1.argn(qRound(item->x())).argn(qRound(item->y())));
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

    int r = m_gridLayout->rowCount();
    auto iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon::fromTheme(icon).pixmap(h, h));
    iconLabel->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
    m_gridLayout->addWidget(iconLabel, r, 0, Qt::AlignTop);

    auto keyLabel = new QLabel(this);
    keyLabel->setText(title);
    keyLabel->setStyleSheet("font-weight: bold; color: gray;"_L1);
    keyLabel->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
    m_gridLayout->addWidget(keyLabel, r, 1, Qt::AlignTop);


    (*ppLabel) = new QLabel(this);
    QLabel *valueLabel = (*ppLabel);
    valueLabel->setWordWrap(true);
    valueLabel->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
    m_gridLayout->addWidget(valueLabel, r, 2, Qt::AlignTop);

}

