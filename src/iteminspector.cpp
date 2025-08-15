/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2024 - 2025 ono ono (kuanyui) <azazabc123@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "iteminspector.h"
#include "gkissitem.h"
#include "gkissscene.h"
#include "zlabel.h"
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

    appendKvRow(&m_twCelName, "cel-file-name", [](){ return tr("CEL File"); });
    appendKvRow(&m_twObjId, "obj-id", [](){ return tr("Object ID"); });
    appendKvRow(&m_twCelPos, "obj-pos", [](){ return tr("CEL Position"); });
    appendKvRow(&m_twFix, "obj-fix-count", [](){ return tr("Fix"); });
    appendKvRow(&m_twCelSize, "cel-size", [](){ return tr("CEL Size"); });
    appendKvRow(&m_twCelComment, "comment", [](){ return tr("CEL Comment"); });
    appendKvRow(&m_twCelPreview, "cel-file-preview", [](){ return tr("CEL Preview"); });
    appendKvRow(&m_twCelSiblings, "obj-cels", [](){ return tr("CEL Siblings"); });
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
    auto celItemCurPos = item->pos().toPoint();
    auto objPos = item->kissScene()->getObjectCurrentPos(item->objId()).value_or(QPoint(-1, -1));
    m_twCelName->setText(item->celFileName());
    m_twObjId->setText(QString::number(item->objId()));
    m_twCelPos->setText("Obj (%1, %2) + CEL (%3, %4) = (%5, %6)"_L1
                        .argn(objPos.x()).argn(objPos.y())
                        .argn(cel->x()).argn(cel->y())
                        .argn(celItemCurPos.x()).argn(celItemCurPos.y()));
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

void ItemInspector::appendKvRow(QLabel **ppLabel, const QString &icon, std::function<QString()> &&titleGetter)
{
    QFont defaultFont = QApplication::font();
    QFontMetrics metrics(defaultFont);
    int h = metrics.height();

    int r = m_gridLayout->rowCount();
    auto iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon::fromTheme(icon).pixmap(h, h));
    iconLabel->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
    m_gridLayout->addWidget(iconLabel, r, 0, Qt::AlignTop);

    auto keyLabel = new ZLabel(this);
    keyLabel->setText(std::move(titleGetter));
    keyLabel->setStyleSheet("font-weight: bold; color: gray;"_L1);
    keyLabel->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
    m_gridLayout->addWidget(keyLabel, r, 1, Qt::AlignTop);


    (*ppLabel) = new QLabel(this);
    QLabel *valueLabel = (*ppLabel);
    valueLabel->setWordWrap(true);
    valueLabel->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
    m_gridLayout->addWidget(valueLabel, r, 2, Qt::AlignTop);

}

