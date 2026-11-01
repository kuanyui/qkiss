/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2026 ono ono (kuanyui) <azazabc123@gmail.com>
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

#include "zwraptopaligneditemdelegate.h"

#include <QHeaderView>
#include <QTreeView>


QSize ZWrapTopAlignedItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!(option.features & QStyleOptionViewItem::WrapText) || option.rect.isValid()) {
        return QStyledItemDelegate::sizeHint(option, index);
    }
    // - Problem: `QTreeView` calls `sizeHint()` with an empty `option.rect`
    //   (width -1). So the style does not know the column width, and measures
    //   the whole text as one line. The row becomes only one line hight.
    //
    // - Fix: copy `option`, set `opt.rect` to column width of header, and pass `opt`
    //   to `QStyledItemDelegate::sizeHint()`.
    const QTreeView *view = qobject_cast<const QTreeView*>(option.widget);
    if (view == nullptr) { return QStyledItemDelegate::sizeHint(option, index); }
    const int columnWidth = view->header()->sectionSize(index.column());
    if (columnWidth <= 0) { return QStyledItemDelegate::sizeHint(option, index); }
    QStyleOptionViewItem opt = option;
    opt.rect = QRect(0, 0, columnWidth, 1);  // `QStyle` only uses the width of `opt.rect`. If the height is 0, `QRect::isValid()` returns false, and `QStyle` ignores `opt.rect`.
    return QStyledItemDelegate::sizeHint(opt, index);
}

void ZWrapTopAlignedItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    if (!(option->features & QStyleOptionViewItem::WrapText)) { return; }
    // Keep the horizontal alignment. Only move the icon and the text to the top.
    option->decorationAlignment = (option->decorationAlignment & Qt::AlignHorizontal_Mask) | Qt::AlignTop;
    option->displayAlignment = (option->displayAlignment & Qt::AlignHorizontal_Mask) | Qt::AlignTop;
}
