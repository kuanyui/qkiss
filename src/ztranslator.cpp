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

#include "ztranslator.h"
#include <QHash>
#include <QDebug>

using namespace Qt::Literals::StringLiterals;

const QHash<QString, QHash<QString, QString>> Z_TRANSLATIONS = {
    { "zh_TW", {
        { "&Copy", "複製 (&C)" },
        { "Select All", "全選" },
        { "&Undo", "復原 (&U)" },
        { "&Redo", "重做 (&R)" },
        { "Cu&t", "剪下 (&T)" },
        { "&Paste", "貼上 (&P)" },
        { "Delete", "刪除" },
        { "Copy &Link Location", "複製連結位置 (&L)" },
        { "QScrollBar::Scroll here", "捲動到這裡" },
        { "QScrollBar::Top", "移到頂端" },
        { "QScrollBar::Bottom", "移到底端" },
        { "QScrollBar::Page up", "向上一頁" },
        { "QScrollBar::Page down", "向下一頁" },
        { "QScrollBar::Scroll up", "向上捲動" },
        { "QScrollBar::Scroll down", "向下捲動" },
    }},
    { "ja_JP", {
        { "&Copy", "コピー (&C)" },
        { "Select All", "すべて選択" },
        { "&Undo", "元に戻す (&U)" },
        { "&Redo", "やり直し (&R)" },
        { "Cu&t", "切り取り (&T)" },
        { "&Paste", "貼り付け (&P)" },
        { "Delete", "削除" },
        { "Copy &Link Location", "リンクの場所をコピー (&L)" },
        { "QScrollBar::Scroll here", "ここにスクロール" },
        { "QScrollBar::Top", "先頭へ" },
        { "QScrollBar::Bottom", "最後へ" },
        { "QScrollBar::Page up", "前のページ" },
        { "QScrollBar::Page down", "次のページ" },
        { "QScrollBar::Scroll up", "上にスクロール" },
        { "QScrollBar::Scroll down", "下にスクロール" },
    }}
};

ZTranslator::ZTranslator(const QString &lang, QObject *parent)
    : QTranslator(parent), m_curLang(lang)
{

}

bool ZTranslator::setLang(const QString &lang)
{
    if (!Z_TRANSLATIONS.contains(lang)) {
        return false;
    }
    m_curLang = lang;
    return true;
}


QString ZTranslator::translate(const char *context, const char *sourceText, const char *disambiguation, int n) const
{
    Q_UNUSED(sourceText);
    Q_UNUSED(disambiguation);
    Q_UNUSED(n);
    QString globalKey = sourceText;     // QString("::%1").arg(src);
    if (Z_TRANSLATIONS[m_curLang].contains(globalKey)) {
        return Z_TRANSLATIONS[m_curLang][globalKey];
    }
    QString ctxKey = QString("%1::%2").arg(context, sourceText);
    if (Z_TRANSLATIONS[m_curLang].contains(ctxKey)) {
        return Z_TRANSLATIONS[m_curLang][ctxKey];
    }
    // qDebug().noquote() << R"#([ZTranslator] Not translated ( %1 )  %2::%3")#"_L1.arg(m_curLang, context, sourceText);
    return QString();   // fallback to Qt itself
}
