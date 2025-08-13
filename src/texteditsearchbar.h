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
    void setKeyword(const QString &keyword);
    void clearKeyword();

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
