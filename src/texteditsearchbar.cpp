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

#include "texteditsearchbar.h"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QStyle>

TextEditSearchBar::TextEditSearchBar(QWidget *parent)
    : QWidget(parent)
{

    m_editKeyword = new QLineEdit(this);
    m_editKeyword->setPlaceholderText("Search...");
    m_editKeyword->setMaximumWidth(200);

    m_btnPrev = new QToolButton(this);
    m_btnPrev->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    m_btnPrev->setToolTip("Previous");

    m_btnNext = new QToolButton(this);
    m_btnNext->setIcon(style()->standardIcon(QStyle::SP_ArrowDown));
    m_btnNext->setToolTip("Next");

    m_btnClose = new QToolButton(this);
    m_btnClose->setIcon(QIcon::fromTheme("window-close"));
    m_btnClose->setToolTip("Close search bar");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_editKeyword);
    layout->addWidget(m_btnPrev);
    layout->addWidget(m_btnNext);
    layout->addStretch();
    layout->addWidget(m_btnClose);

    connect(m_editKeyword, &QLineEdit::textChanged, this, &TextEditSearchBar::onKeywordChanged);
    connect(m_btnNext, &QToolButton::clicked, this, &TextEditSearchBar::onFindNext);
    connect(m_btnPrev, &QToolButton::clicked, this, &TextEditSearchBar::onFindPrevious);
    connect(m_btnClose, &QToolButton::clicked, this, &TextEditSearchBar::onClose);

    hide();
}

void TextEditSearchBar::setFocus()
{
    m_editKeyword->setFocus();
    m_editKeyword->selectAll();
}

void TextEditSearchBar::setKeyword(const QString &keyword)
{
    m_editKeyword->setText(keyword);
    emit keywordChanged(keyword);
}

void TextEditSearchBar::clearKeyword()
{
    m_editKeyword->clear();
    emit keywordChanged(m_editKeyword->text());
}

void TextEditSearchBar::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        onClose();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (event->modifiers() & Qt::ShiftModifier) {
            onFindPrevious();
        } else {
            onFindNext();
        }
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void TextEditSearchBar::onKeywordChanged()
{
    QString text = m_editKeyword->text();
    emit keywordChanged(text);
}

void TextEditSearchBar::onFindNext()
{
    emit searchNext();
}

void TextEditSearchBar::onFindPrevious()
{
    emit searchPrevious();
}

void TextEditSearchBar::onClose()
{
    hide();
    emit searchClosed();
}
