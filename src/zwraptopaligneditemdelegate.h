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

#ifndef ZWRAPTOPALIGNEDITEMDELEGATE_H
#define ZWRAPTOPALIGNEDITEMDELEGATE_H

#include <QStyledItemDelegate>

/// A delegate for item views with word wrap turned on (for example
/// `QTreeView::setWordWrap(true)`).
///
/// Default (centered)      With this delegate
/// +----------+-------+    +----------+-------+
/// |          | blah  |    | (i) Info | blah  |
/// | (i) Info | blah  |    |          | blah  |
/// |          | blaah |    |          | blaah |
/// +----------+-------+    +----------+-------+
///
/// Qt has two problems with word wrap in MVC (QTreeView, QTableView):
///
/// 1. When a long~long text wraps to many lines in a table cell, the
///    height of this row -- along with all the other cells in this
///    row, will increase. The icon and short text will be rendered
///    in the middle of the row, but I expect the icon + short text
///    are still in the first line.
///
/// 2. When `QTreeView` asks for the height of a row, it does not tell
///    the delegate how wide the column is. So the height only counts
///    the lines broken by `\n`, not the lines broken by wrapping.
///
/// This delegate puts icon and text at the top of the row, and gives
/// the column width to the size calculation. But when word wrap is off, it
/// behaves exactly like `QStyledItemDelegate`.
///
/// What you should do:
///
/// - You should turn on word wrap in the view: `QTreeView::setWordWrap(true)`,
///   `QTableView::setWordWrap(true)`, or `QListView::setWordWrap(true)`.
///   Without word wrap, this delegate does nothing.
/// - `QTreeView`: you should call `QTreeView::doItemsLayout()` after a
///   column is resized. For example, connect `QHeaderView::sectionResized`
///   to `QTreeView::doItemsLayout()`. Reason: `QTreeView` saves the height of
///   each row, and does not call `QAbstractItemDelegate::sizeHint()` again
///   when a column gets wider or narrower. But with word wrap, the number of
///   lines in a row depends on the column width.
/// - Subclass of `QTreeView`: you should call
///   `QAbstractItemView::scheduleDelayedItemsLayout()` instead (a protected
///   function). Then `QTreeView::doItemsLayout()` runs only once after Qt has
///   handled the current events, even when several columns are resized at
///   the same time. (`ZTreeWidget` does this.)
/// - `QTableView`: you should make every row as high as its text. Call
///   `QHeaderView::setSectionResizeMode(QHeaderView::ResizeToContents)` on
///   `QTableView::verticalHeader()`. Otherwise every row keeps the same
///   fixed height.
///
/// Limitations:
///
/// - `QTreeView`: the indentation of child items is not subtracted from the
///   column width, so the last line of a child row may be cut off. Flat
///   lists (only top-level items, with `QTreeView::setRootIsDecorated(false)`)
///   are fine.
/// - With word wrap on, `QTreeView::resizeColumnToContents()` (also called by
///   double-clicking the divider between two header sections) and
///   `QHeaderView::ResizeToContents` cannot make the column wider than the
///   current width. The reason: this delegate wraps the text at the current
///   column width, so the widest line is never wider than the current width.
class ZWrapTopAlignedItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};

#endif // ZWRAPTOPALIGNEDITEMDELEGATE_H
