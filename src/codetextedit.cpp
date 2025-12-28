/**************************************************************************
* QKiSS - A modern implementation of KISS (KISekae Set system) viewer in Qt 6.
*
* Copyright (C) 2024 ono ono (kuanyui) <azazabc123 АТ GМАIL.СОМ>
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
*
* ===============================================================
*
* And this file is forked from Qt official documentation:
*
* - https://doc.qt.io/qt-6.2/qtwidgets-widgets-codeeditor-example.html
* - https://doc.qt.io/qt-6.2/qtwidgets-richtext-syntaxhighlighter-example.html
*
* According to https://doc.qt.io/qt-6/examples-license.html , the following
* is a copy of the license:
*
* ===============================================================
*
* Copyright (C) 2018 The Qt Company Ltd.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*  * Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*  * Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
*  * Neither the name of The Qt Company Ltd nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************/

#include <QPainter>
#include <QTextBlock>

#include "codetextedit.h"

CodeTextEdit::CodeTextEdit(QWidget *parent):
    QPlainTextEdit{parent}
{
    m_lineNumberArea = new CodeTextEditLineNumberArea(this);
    m_syntaxHighlighter = new CodeTextEditSyntaxHighlighter(this->document());
    m_font = QFont {"Monospace"};
    m_font.setStyleHint(QFont::TypeWriter);
    m_textCharFmtSearchCurrentOccurrence.setBackground(QColor(255, 255, 0, 200));
    m_textCharFmtSearchCurrentOccurrence.setForeground(QColor(0, 0, 0, 255));
    m_textCharFmtSearchOtherOccurrence.setBackground(QColor(255, 0, 255, 100));
    setFont(m_font);
    setReadOnly(true);

    connect(this, &CodeTextEdit::blockCountChanged, this, &CodeTextEdit::updateLineNumberAreaWidth);
    connect(this, &CodeTextEdit::updateRequest, this, &CodeTextEdit::updateLineNumberArea);
    connect(this, &CodeTextEdit::cursorPositionChanged, this, &CodeTextEdit::highlightCurrentLine);
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int CodeTextEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = (LINENUM_HORI_PAD * 2) + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void CodeTextEdit::searchText(const QString &keyword)
{
    QTextCharFormat clearFormat;
    clearFormat.setBackground(Qt::NoBrush);
    clearFormat.setForeground(Qt::NoBrush);
    for (QTextCursor &cursor : m_searchOccurrences) {
        cursor.mergeCharFormat(clearFormat);
    }

    if (keyword.isEmpty()) {
        clearSearch();
        return;
    }

    m_searchKeyword = keyword;
    m_searchOccurrences.clear();
    m_searchOccurrenceIndex = -1;

    QTextDocument *doc = document();   // doc holds formatted text
    QTextCursor cursor(doc);

    QTextDocument::FindFlags flags;
    if (m_searchCaseSensitive) { flags |= QTextDocument::FindFlag::FindCaseSensitively; }
    if (m_searchWholeWord) { flags |= QTextDocument::FindFlag::FindWholeWords; }

    while (!cursor.isNull() && !cursor.atEnd()) {
        cursor = doc->find(keyword, cursor, flags);
        if (cursor.isNull()) { break; }
        m_searchOccurrences.append(cursor);
    }

    if (!m_searchOccurrences.isEmpty()) {
        m_searchOccurrenceIndex = 0;
        moveToSearchOccurrence(0);
    }
    renderAllSearchOccurrencesHighlights();
}

void CodeTextEdit::searchNext()
{
    if (m_searchOccurrences.isEmpty()) { return; }
    m_searchOccurrenceIndex = (m_searchOccurrenceIndex + 1) % m_searchOccurrences.length();
    moveToSearchOccurrence(m_searchOccurrenceIndex);
    renderAllSearchOccurrencesHighlights();
}

void CodeTextEdit::searchPrevious()
{
    if (m_searchOccurrences.isEmpty()) { return; }
    m_searchOccurrenceIndex = (m_searchOccurrenceIndex - 1 + m_searchOccurrences.length()) % m_searchOccurrences.length();   // ...too show-off to understand?
    moveToSearchOccurrence(m_searchOccurrenceIndex);
    renderAllSearchOccurrencesHighlights();
}

void CodeTextEdit::clearSearch()
{
    m_searchKeyword.clear();
    m_searchOccurrences.clear();
    m_searchOccurrenceIndex = -1;
    renderAllSearchOccurrencesHighlights();
}

void CodeTextEdit::setPlainTextAndClearSearch(const QString &text)
{
    m_searchKeyword.clear();
    m_searchOccurrences.clear();
    m_searchOccurrenceIndex = -1;
    // Clear all QTextCharFormat
    QTextCursor cursor(document());
    cursor.select(QTextCursor::Document);
    cursor.setCharFormat(QTextCharFormat());
    setPlainText(text);
}

void CodeTextEdit::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeTextEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }
    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void CodeTextEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.x(), cr.y(), lineNumberAreaWidth(), cr.height()));
}

void CodeTextEdit::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    auto clrplt = m_themeManager.syntaxHighlightFormatPalette();
    QColor lineColor = QColor(clrplt.curLineHlBg);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    setExtraSelections(extraSelections);
}

void CodeTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    auto clrplt = m_themeManager.syntaxHighlightFormatPalette();
    QPainter painter{qobject_cast<QWidget*>(m_lineNumberArea)};
    painter.fillRect(event->rect(), clrplt.lineNumBg);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(clrplt.lineNumFg);
            painter.drawText(0, top, m_lineNumberArea->width() - LINENUM_HORI_PAD, fontMetrics().height(),
                             Qt::AlignRight, number);  // AlignRight, so the text drawing should be x=0, and width should -= PAD
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeTextEdit::renderAllSearchOccurrencesHighlights()
{
    // Syntax rehighlight (this will not remove merged format)
    QSyntaxHighlighter *highlighter = findChild<QSyntaxHighlighter *>();
    if (highlighter) {
        highlighter->rehighlight();
    }

    for (int i = 0; i < m_searchOccurrences.length(); i++) {
        auto occurredCursor = m_searchOccurrences[i];
        if (i == m_searchOccurrenceIndex) {
            occurredCursor.mergeCharFormat(m_textCharFmtSearchCurrentOccurrence);
        } else {
            occurredCursor.mergeCharFormat(m_textCharFmtSearchOtherOccurrence);
        }
    }
}

void CodeTextEdit::moveToSearchOccurrence(int index)
{
    if (index < 0) { return; }
    if (index >= m_searchOccurrences.length()) { return; }
    m_searchOccurrenceIndex = index;
    setTextCursor(m_searchOccurrences[index]);
}


