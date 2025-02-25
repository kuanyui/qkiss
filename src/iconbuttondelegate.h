#ifndef ICONBUTTONDELEGATE_H
#define ICONBUTTONDELEGATE_H

#include <QApplication>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QMouseEvent>

class IconButtonDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit IconButtonDelegate(QObject* parent = nullptr);

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    void pressed(const QModelIndex& index) const;

};



#endif // ICONBUTTONDELEGATE_H
