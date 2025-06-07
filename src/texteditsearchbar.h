#ifndef TEXTEDITSEARCHBAR_H
#define TEXTEDITSEARCHBAR_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QKeyEvent>

class TextEditSearchBar : public QWidget
{
    Q_OBJECT

public:
    explicit TextEditSearchBar(QWidget *parent = nullptr);
    void setFocus();

signals:
    void keywordChanged(const QString &text);
    void searchNext();
    void searchPrevious();
    void searchClosed();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onKeywordChanged();
    void onFindNext();
    void onFindPrevious();
    void onClose();

private:
    QLineEdit *m_editKeyword;
    QToolButton *m_btnNext;
    QToolButton *m_btnPrev;
    QToolButton *m_btnClose;
};

#endif // TEXTEDITSEARCHBAR_H
