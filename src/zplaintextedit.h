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

#ifndef ZPLAINTEXTEDIT_H
#define ZPLAINTEXTEDIT_H

#include <QPlainTextEdit>
#include <QAction>

/// A `QPlainTextEdit` with a "Word Wrap" checkbox at the end of its
/// right-click menu. Word wrap is off by default.
class ZPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit ZPlainTextEdit(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    /// Set the text of `m_wordWrapAction` in the current UI language.
    void updateWordWrapActionText();
    QAction *m_wordWrapAction;
};

#endif // ZPLAINTEXTEDIT_H
