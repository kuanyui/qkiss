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
    QVariant iconVariant = index.data(Qt::DecorationRole);
    QVariant textVariant = index.data(Qt::DisplayRole);
    QIcon icon = qvariant_cast<QIcon>(iconVariant);
    QString text = textVariant.toString();
    if (icon.isNull() && text.isEmpty()) {
        return;
    }
    static int padHori = option.widget->style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
    static int padVert = option.widget->style()->pixelMetric(QStyle::PM_FocusFrameVMargin);
    qDebug() << padHori << padVert;
    int itemHeight = option.rect.height();
    int itemIconSize = itemHeight - padVert * 2;
    QRect iconRect;
    QRect textRect;
    // Draw button background
    QStyleOptionButton buttonOption;
    buttonOption.rect = option.rect;
    buttonOption.state = QStyle::State_Enabled;

    if (option.state & QStyle::State_MouseOver) {
        buttonOption.state |= QStyle::State_MouseOver;
    }
    if (option.state & QStyle::State_Selected) {
        buttonOption.state |= QStyle::State_Sunken;
    }

    // Draw according to standard style of QStyle::ControlElement::PushButton
    QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter);

    if (!icon.isNull()) {
        iconRect = option.rect.adjusted(padHori, padVert, 0, 0);
        iconRect.setHeight(itemIconSize);
        iconRect.setWidth(itemIconSize);
        QPixmap pixmap = icon.pixmap(itemIconSize, itemIconSize);
        painter->drawPixmap(iconRect.topLeft(), pixmap);
    }
    if (!text.isEmpty()) {
        // Calculate text width
        QFontMetrics fm(option.font);
        int textWidth = fm.horizontalAdvance(text);
        int textHeight = fm.height();

        textRect.setX(option.rect.x() + iconRect.width());
        textRect.setY(option.rect.y() + padVert);
        textRect.setWidth(textWidth);
        textRect.setHeight(textHeight);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignHCenter, text);
    }
    qDebug() << option.rect << iconRect << textRect;
}

bool IconButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (option.rect.contains(mouseEvent->pos())) {
            emit pressed(index);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

