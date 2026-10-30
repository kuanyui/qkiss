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

#include "iconbuttondelegate.h"


IconButtonDelegate::IconButtonDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

QSize IconButtonDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // When implement sizeHint, should not use `option.rect` (which is effected by user dragging header columns)
    if (!index.isValid()) {
        return QSize{0, 0};
    }
    QVariant iconVariant = index.data(Qt::DecorationRole);
    QVariant textVariant = index.data(Qt::DisplayRole);
    QIcon icon = qvariant_cast<QIcon>(iconVariant);
    QString text = textVariant.toString();

    static int padHori = option.widget->style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
    static int padVert = option.widget->style()->pixelMetric(QStyle::PM_FocusFrameVMargin);
    // Get font metrics
    QFontMetrics fm(option.font);
    int textHeight = fm.height();
    int textWidth = fm.horizontalAdvance(text);
    QSize iconSize {};
    if (!icon.isNull()) {
        iconSize = QSize {
                fm.height() + padHori * 2,
                fm.height() + padVert * 2
        };
    }
    QSize textSize {};
    if (!text.isEmpty()) {
        textSize = QSize {
            textWidth + padHori * 2,
            textHeight + padVert * 2
        };
    }
    // Same as the spacing between icon and text in paint()
    int iconSpacing = (!icon.isNull() && !text.isEmpty()) ? padHori : 0;
    QSize hint {
        iconSize.width() + iconSpacing + textSize.width(),
        qMax(iconSize.height(), textSize.height())
    };
    return hint;
}


void IconButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // https://doc.qt.io/qt-6/qstyleditemdelegate.html#paint
    if (!index.isValid()) {
        return;
    }
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QIcon icon = opt.icon;
    QString text = opt.text;
    if (icon.isNull() && text.isEmpty()) {
        return;
    }
    int padHori = opt.widget->style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
    // qkDebug << padHori;
    QFontMetrics fm(opt.font);
    int itemIconSize = fm.height();   // Same as the icon size in sizeHint(). (The row can be taller than sizeHint(), so don't use `opt.rect.height()` here, or the icon becomes larger than the width that sizeHint() gave)
    // Not QStyle's spacing of QPushButton (hardcoded 4px in `QCommonStyle`, no QStyle::PixelMetric for it), which looks too large with these small paddings.
    int iconSpacing = (!icon.isNull() && !text.isEmpty()) ? padHori : 0;
    QRect iconRect;
    QRect textRect;

    // Draw button background
    QStyleOptionButton buttonOption;
    buttonOption.rect = opt.rect;
    buttonOption.state = QStyle::State_Enabled;
    if (opt.state & QStyle::State_MouseOver) {
        if (index == m_hoveredOnIndex) {
            buttonOption.state |= QStyle::State_MouseOver;
        } else if (index == m_origPressedIndex) {
            buttonOption.state |= QStyle::State_MouseOver;
        }
    }
    if (index == m_origPressedIndex && index == m_hoveredOnIndex) {
        buttonOption.state |= QStyle::State_Sunken;
    } else {
        buttonOption.state &= ~QStyle::State_Sunken;
    }
    // Draw according to standard style of QStyle::ControlElement::PushButton
    QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter);

    // Center icon + spacing + text in the button as a whole
    // Calculate text width
    int textWidth = fm.horizontalAdvance(text);
    int iconAndTextWidth = (icon.isNull() ? 0 : itemIconSize) + iconSpacing + textWidth;
    int x = opt.rect.x() + (opt.rect.width() - iconAndTextWidth) / 2;
    if (!icon.isNull()) {
        iconRect = QRect(x, opt.rect.center().y() - itemIconSize / 2, itemIconSize, itemIconSize);
        QPixmap pixmap = icon.pixmap(itemIconSize, itemIconSize);
        painter->drawPixmap(iconRect.topLeft(), pixmap);
        x = iconRect.right() + 1 + iconSpacing;   // Text starts after the icon (not at `iconRect.width()`, which ignores where the icon starts)
    }
    if (!text.isEmpty()) {
        textRect = QRect(x, opt.rect.y(), textWidth, opt.rect.height());
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignHCenter, text);
    }
    // qkDebug << opt.rect << iconRect << textRect;
}

bool IconButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    /// NOTE: 1. QStyledItemDelegate::editorEvent doesn't trigger `QEvent::MouseLeave`.
    /// NOTE: 2. Assume delegate is applied only on column 2, then the editorEvent() is only triggered when current pixel and previous pixel are both on column 2.
    /// NOTE: 3. View must call `QWidget ::setMouseTracking(true)` if you wants QEvent::MouseMove event without pressing.
    /// NOTE: 4. With mouse, only 3 events are triggered: MouseButtonPress, MouseMove, MouseButtonRelease
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    if (event->type() == QEvent::MouseButtonPress) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            m_origPressedIndex = index;
            m_hoveredOnIndex = index;
        }
        mouseEvent->accept();
        return true;
    } else if (event->type() == QEvent::MouseMove) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        // qkDebug << "QEvent.pos" << mouseEvent->pos();
        if (opt.rect.contains(mouseEvent->pos())) {
            m_hoveredOnIndex = index;
        } else {
            m_hoveredOnIndex = QModelIndex();
        }
        m_hoveredOnIndex = index;
        mouseEvent->accept();
        return true;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton && m_origPressedIndex == index) {
            emit pressed(index);
            mouseEvent->accept();
        }
        m_hoveredOnIndex = QModelIndex();
        m_origPressedIndex = QModelIndex();

        return true;
    }
    // if (event->type() == QEvent::TouchBegin) {
    //     auto *touchEvent = static_cast<QTouchEvent *>(event);
    //     if (!touchEvent->points().isEmpty()) {
    //         const QTouchEvent::TouchPoint &point = touchEvent->points().first();
    //         if (option.rect.contains(point.position().toPoint())) {
    //             m_pressedIndex = index;
    //             touchEvent->accept();
    //             return true;
    //         }
    //     }
    // } else if (event->type() == QEvent::TouchUpdate) {
    //     auto *touchEvent = static_cast<QTouchEvent *>(event);
    //     if (!touchEvent->points().isEmpty()) {
    //         const QTouchEvent::TouchPoint &point = touchEvent->points().first();
    //         if (!option.rect.contains(point.position().toPoint()) && m_pressedIndex.isValid()) {
    //             m_pressedIndex = QModelIndex();
    //         }
    //     }
    // } else if (event->type() == QEvent::TouchEnd) {
    //     m_pressedIndex = QModelIndex();
    //     event->accept();
    //     return true;
    // }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

