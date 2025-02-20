#include "iconbuttondelegate.h"


IconButtonDelegate::IconButtonDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}


void IconButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
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

    QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter);

    // REFACTOR: Magic number
    QRect iconRect = option.rect.adjusted(5, 5, -5, -5);
    QRect textRect = option.rect.adjusted(30, 0, -5, 0);

    if (!icon.isNull()) {
        QPixmap pixmap = icon.pixmap(20, 20);
        painter->drawPixmap(iconRect.topLeft(), pixmap);
    }
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);
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

