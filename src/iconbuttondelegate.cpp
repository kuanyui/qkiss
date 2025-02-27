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
    QSize hint {
        iconSize.width() + textSize.width(),
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
     int padVert = opt.widget->style()->pixelMetric(QStyle::PM_FocusFrameVMargin);
    qDebug() << padHori << padVert;
    int itemHeight = opt.rect.height();
    int itemIconSize = itemHeight - padVert * 2;
    QRect iconRect;
    QRect textRect;

    // Draw button background
    QStyleOptionButton buttonOption;
    buttonOption.rect = opt.rect;
    buttonOption.state = QStyle::State_Enabled;

    if (opt.state & QStyle::State_MouseOver) {
        buttonOption.state |= QStyle::State_MouseOver;
    }
    if (index == m_pressedIndex && index == m_hoveredIndex) {
        buttonOption.state |= QStyle::State_Sunken;
    }

    // Draw according to standard style of QStyle::ControlElement::PushButton
    QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter);

    if (!icon.isNull()) {
        iconRect = opt.rect.adjusted(padHori, padVert, 0, 0);
        iconRect.setHeight(itemIconSize);
        iconRect.setWidth(itemIconSize);
        QPixmap pixmap = icon.pixmap(itemIconSize, itemIconSize);
        painter->drawPixmap(iconRect.topLeft(), pixmap);
    }
    if (!text.isEmpty()) {
        // Calculate text width
        QFontMetrics fm(opt.font);
        int textWidth = fm.horizontalAdvance(text);
        int textHeight = fm.height();

        textRect.setX(opt.rect.x() + iconRect.width());
        textRect.setY(opt.rect.y() + padVert);
        textRect.setWidth(textWidth);
        textRect.setHeight(textHeight);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignHCenter, text);
    }
    qDebug() << opt.rect << iconRect << textRect;
}

bool IconButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    if (event->type() == QEvent::MouseButtonPress) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            m_pressedIndex = index;
            m_hoveredIndex = index;
        }
        mouseEvent->accept();
        return true;
    } else if (event->type() == QEvent::MouseMove) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (option.rect.contains(mouseEvent->pos()) && m_pressedIndex.isValid()) {
            m_hoveredIndex = index;
        } else {
            m_hoveredIndex = QModelIndex();
        }
        mouseEvent->accept();
    } else if (event->type() == QEvent::MouseButtonRelease) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton && m_pressedIndex == index) {
            emit pressed(index);
            mouseEvent->accept();
        }
        m_hoveredIndex = QModelIndex();
        m_pressedIndex = QModelIndex();
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

