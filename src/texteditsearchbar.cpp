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
