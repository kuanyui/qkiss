#include "codeeditor.h"

CodeEditor::CodeEditor(QWidget *parent)
    : QWidget{parent}
{
    setupUi();
}

void CodeEditor::setPlainText(const QString &text)
{
    m_codeEdit->setPlainText(text);
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
