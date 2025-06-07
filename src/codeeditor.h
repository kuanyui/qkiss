#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QWidget>
#include <QShortcut>

#include "codetextedit.h"
#include "texteditsearchbar.h"

class CodeEditor : public QWidget
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);
    void setPlainText(const QString &text);

signals:

public slots:
    void showSearchBar();
    void hideSearchBar();
    void onSearchBarKeywordChanged(const QString &keyword);

private:
    void setupUi();
    CodeTextEdit *m_codeEdit;
    TextEditSearchBar *m_searchBar;
    QShortcut *m_searchShortcut;
};

#endif // CODEEDITOR_H
