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

#include "codeeditor.h"

CodeEditor::CodeEditor(QWidget *parent)
    : QWidget{parent}
{
    setupUi();
}

void CodeEditor::setPlainText(const QString &text)
{
    m_searchBar->hide();
    m_searchBar->clearKeyword();
    m_codeEdit->setPlainTextAndClearSearch(text);
}

void CodeEditor::showSearchBar()
{
    m_searchBar->show();
    m_searchBar->setFocus();
}

void CodeEditor::hideSearchBar()
{
    m_searchBar->hide();
    m_codeEdit->clearSearch();
    m_codeEdit->setFocus();
}

void CodeEditor::onSearchBarKeywordChanged(const QString &keyword)
{
    m_codeEdit->searchText(keyword);
}

void CodeEditor::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_codeEdit = new CodeTextEdit(this);
    m_searchBar = new TextEditSearchBar(this);

    layout->addWidget(m_codeEdit);
    layout->addWidget(m_searchBar);

    m_searchShortcut = new QShortcut(QKeySequence::Find, this);
    connect(m_searchShortcut, &QShortcut::activated, this, &CodeEditor::showSearchBar);

    connect(m_searchBar, &TextEditSearchBar::keywordChanged,  this, &CodeEditor::onSearchBarKeywordChanged);
    connect(m_searchBar, &TextEditSearchBar::searchNext, m_codeEdit, &CodeTextEdit::searchNext);
    connect(m_searchBar, &TextEditSearchBar::searchPrevious, m_codeEdit, &CodeTextEdit::searchPrevious);
    connect(m_searchBar, &TextEditSearchBar::searchClosed, this, &CodeEditor::hideSearchBar);
}
