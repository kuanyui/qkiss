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
    }},
    // 関西弁
    { "ja_KS", {
        { "&Copy", "コピるんや (&C)" },
        { "Select All", "ぜんぶ選ぶんや" },
        { "&Undo", "元に戻すんや (&U)" },
        { "&Redo", "やり直すで (&R)" },
        { "Cu&t", "切り取るで (&T)" },
        { "&Paste", "貼りつけるで (&P)" },
        { "Delete", "消すで" },
        { "Copy &Link Location", "リンクの場所コピるんや (&L)" },
        { "QScrollBar::Scroll here", "ここにスクロールするんや" },
        { "QScrollBar::Top", "いっちゃん上へ" },
        { "QScrollBar::Bottom", "いっちゃん下へ" },
        { "QScrollBar::Page up", "前のページやで" },
        { "QScrollBar::Page down", "次のページやで" },
        { "QScrollBar::Scroll up", "上にスクロールするで" },
        { "QScrollBar::Scroll down", "下にスクロールするで" },
    }},
    // お嬢さん
    { "ja_OJ", {
        { "&Copy", "コピーですわ (&C)" },
        { "Select All", "すべてをお選びあそばせ" },
        { "&Undo", "元に戻しますのよ (&U)" },
        { "&Redo", "やり直しましてよ (&R)" },
        { "Cu&t", "切り取りなさいませ (&T)" },
        { "&Paste", "貼り付けますわよ (&P)" },
        { "Delete", "削除なさいますわ" },
        { "Copy &Link Location", "リンクの所在をコピーいたしますの (&L)" },
        { "QScrollBar::Scroll here", "ここにて御スクロールあそばせ" },
        { "QScrollBar::Top", "いと高き所へ参りますわ" },
        { "QScrollBar::Bottom", "いと低き所へ落ちていただきますの" },
        { "QScrollBar::Page up", "前の頁へお進みあそばせ" },
        { "QScrollBar::Page down", "次の頁へお進みあそばせ" },
        { "QScrollBar::Scroll up", "上へ御スクロールなさいますわ" },
        { "QScrollBar::Scroll down", "下へ御スクロールなさいますわ" },
    }},
    // 悪役令嬢
    { "ja_AR", {
          { "&Copy", "コピーして差し上げますわ、平民風情には勿体なくてよ (&C)" },
          { "Select All", "すべてをお選びあそばせ──どうせ貴様には扱いきれませんけれど" },
          { "&Undo", "元にお戻したがるの？あらあら、後悔なさるのは庶民の常ですわね (&U)" },
          { "&Redo", "やり直してご覧なさいませ、どうせ結果は同じでしてよ！オホホホ！ (&R)" },
          { "Cu&t", "切り取って差し上げますわ、不要なものは捨て置くのが貴族の嗜みざます (&T)" },
          { "&Paste", "貼り付けて差し上げますのよ、感謝なさいませ！ (&P)" },
          { "Delete", "消え去りなさいませ！奈落の底まで堕ちるがよろしい！オホホホ！" },
          { "Copy &Link Location", "リンクの所在をお写しあそばせ──もっとも貴様に理解できるかしら？ (&L)" },
          { "QScrollBar::Scroll here", "ここにて御スクロールなさいますの？まあ、滑稽ですこと" },
          { "QScrollBar::Top", "いっとう上まで参りますわ──高みはわたくしの居場所ですもの" },
          { "QScrollBar::Bottom", "奈落の底まで落ちていただきますの！オホホホ！" },
          { "QScrollBar::Page up", "前の頁へお進みあそばせ、這い上がれるものならば" },
          { "QScrollBar::Page down", "次の頁へお進みあそばせ──転落の始まりですわよ" },
          { "QScrollBar::Scroll up", "上へ御スクロールなさいますわ、身の程知らずにも" },
          { "QScrollBar::Scroll down", "下へ御スクロールなさいますわ──堕落の道連れにして差し上げますわ" },
    }},
    /*
    // 悪役令嬢
    { "&Copy", "コピーして差し上げますわ、オホホ！ (&C)" },
    { "Select All", "すべて選んでやりましてよ (&A)" },
    { "&Undo", "後悔なさいますの？ (&U)" },
    { "&Redo", "やり直してご覧なさいませ！ (&R)" },
    { "Cu&t", "切り取りざます！ (&T)" },
    { "&Paste", "貼り付けますのよ！ (&P)" },
    { "Delete", "消えなさいませ！オホホホ！" },
    { "Copy &Link Location", "リンクを写して差し上げますわ (&L)" },
    { "QScrollBar::Scroll here", "ここにスクロールなさいますの？" },
    { "QScrollBar::Top", "いっとう上へ参りますわ" },
    { "QScrollBar::Bottom", "奈落の底まで落ちていただきますの！" },
    { "QScrollBar::Page up", "前の頁へどうぞ" },
    { "QScrollBar::Page down", "次の頁へどうぞ" },
    { "QScrollBar::Scroll up", "上へ参りますわ" },
    { "QScrollBar::Scroll down", "下へ堕ちていただきますわ" },
    */
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
