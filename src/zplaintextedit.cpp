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

#include "zplaintextedit.h"
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QEvent>
#include <QMenu>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QTextBlock>

ZPlainTextEdit::ZPlainTextEdit(QWidget *parent)
    : QPlainTextEdit{parent},
      m_wordWrapAction{new QAction{this}}
{
    updateWordWrapActionText();
    // Word wrap starts off. This widget does not save the choice, so word wrap
    // is off again the next time the app starts.
    m_wordWrapAction->setIcon(QIcon::fromTheme("wrap-text"));
    m_wordWrapAction->setCheckable(true);
    m_wordWrapAction->setChecked(false);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    connect(m_wordWrapAction, &QAction::toggled, this, [this](bool checked) {
        // `setLineWrapMode()` calls `ensureCursorVisible()`. This can scroll to the
        // text cursor. In a read-only viewer, the cursor may be far from the view.
        // Save the first visible block and restore it after changing the wrap mode.
        const QTextBlock topBlock = firstVisibleBlock();
        setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
        verticalScrollBar()->setValue(topBlock.firstLineNumber());
    });
}

void ZPlainTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    // Other code may call `setLineWrapMode()` directly. So update the checkbox
    // from the real wrap mode first. `QSignalBlocker` stops `toggled` here, so
    // a mode like `FixedColumnWidth` is not replaced by `WidgetWidth`.
    {
        const QSignalBlocker blocker{m_wordWrapAction};
        m_wordWrapAction->setChecked(lineWrapMode() != QPlainTextEdit::NoWrap);
    }
    // Keep the standard items (Copy, Select All, ...), and add "Word Wrap"
    // below them.
    QMenu *menu = createStandardContextMenu();
    menu->addSeparator();
    menu->addAction(m_wordWrapAction);
    // The menu deletes itself after it closes. It does not delete
    // `m_wordWrapAction`, because this widget owns the action.
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup(event->globalPos());
}

void ZPlainTextEdit::changeEvent(QEvent *event)
{
    // Qt sends `QEvent::LanguageChange` to every widget when the app changes
    // its translators, for example after the user picks another UI language.
    if (event->type() == QEvent::LanguageChange) {
        updateWordWrapActionText();
    }
    QPlainTextEdit::changeEvent(event);
}

void ZPlainTextEdit::updateWordWrapActionText()
{
    // `ZTranslator` translates this text, so this class does not need the
    // `*.ts` files of the app. Same as "&Copy" in `ZTreeWidget`: the text is
    // in a variable, so `lupdate` does not add it to the `*.ts` files.
    static const char *strWordWrap { "&Word Wrap" };
    m_wordWrapAction->setText(QCoreApplication::tr(strWordWrap));
}
