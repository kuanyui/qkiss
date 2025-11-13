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
        { "KStandardGuiItem::&OK", "確定 (&O)" },
        { "KStandardGuiItem::&Cancel", "取消 (&C)" },
        { "KStandardGuiItem::&Apply", "套用 (&A)" },
        { "QFileDialog::All Files (*)", "所有檔案 (*)" },
        { "QFileDialog::Look in:", "查看位置：" },
        { "QFileDialog::Back", "返回" },
        { "QFileDialog::Go back", "返回上一層" },
        { "QFileDialog::Alt+Left", "Alt+左" },
        { "QFileDialog::Forward", "前進" },
        { "QFileDialog::Go forward", "前往下一層" },
        { "QFileDialog::Alt+Right", "Alt+右" },
        { "QFileDialog::Parent Directory", "上層目錄" },
        { "QFileDialog::Go to the parent directory", "前往上層目錄" },
        { "QFileDialog::Alt+Up", "Alt+上" },
        { "QFileDialog::Create New Folder", "建立新資料夾" },
        { "QFileDialog::Create a New Folder", "建立新資料夾" },
        { "QFileDialog::List View", "清單檢視" },
        { "QFileDialog::Change to list view mode", "切換為清單檢視模式" },
        { "QFileDialog::Detail View", "詳細資料檢視" },
        { "QFileDialog::Change to detail view mode", "切換為詳細資料檢視模式" },
        { "QFileDialog::Sidebar", "側邊欄" },
        { "QFileDialog::List of places and bookmarks", "位置與書籤清單" },
        { "QFileDialog::Files", "檔案" },
        { "QFileDialog::Files of type:", "檔案類型：" },
        { "QFileDialog::File &name:", "檔案名稱 (&N)：" },
        { "QFileDialog::&Open", "開啟 (&O)" },
        { "QFileDialog::Show &hidden files", "顯示隱藏檔 (&H)" },
        { "QFileDialog::&New Folder", "新資料夾 (&N)" },
        { "QFileDialog::&Rename", "重新命名 (&R)" },
        { "QFileDialog::&Delete", "刪除 (&D)" },
        { "QFileDialog::&Choose", "選擇 (&C)" },
        { "QFileDialog::Remove", "移除" },
        { "QFileDialog::Directory:", "目錄：" },
        { "QFileSystemModel::Name", "名稱" },
        { "QFileSystemModel::Size", "大小" },
        { "QFileSystemModel::Type", "類型" },
        { "QFileSystemModel::Date Modified", "修改日期" },
        { "QFileSystemModel::Computer", "電腦" },
        { "KMessageWidget::&Close", "關閉 (&C)" },
        { "KMessageWidget::Close message", "關閉訊息" },
        { "KMessageWidget::Note", "備註" },
        { "KMessageWidget::Error", "錯誤" },
        { "KStandardActions::&Up", "上一層 (&U)" },
        { "KStandardActions::Go up", "移至上一層" },
        { "KStandardActions::&Refresh", "重新整理 (&R)" },
        { "KStandardActions::Refresh document", "重新整理文件" },
        { "KStandardActions::&Rename…", "重新命名 (&R)…" },
        { "KStandardActions::&Add Bookmark", "新增書籤 (&B)" },
        { "KBookmarkMenu::New Bookmark Folder…", "新書籤資料夾…" },
        { "KBookmarkMenu::Create a new bookmark folder in this menu", "在此選單中建立新的書籤資料夾" },
        { "QIODevice::No such file or directory", "沒有此檔案或目錄" },
        { "QAbstractFileIconProvider::Drive", "磁碟機" },
        { "QAbstractFileIconProvider::Folder", "資料夾" },
        { "QAbstractFileIconProvider::Shortcut", "捷徑" },
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
    // メ〇〇キ
    { "ja_MK", {
        { "&Copy", "コピーくらい自分でしなよ〜〜〜 (&C)" },
        { "Select All", "どうせ全部選ぶんでしょ？ざこ (&A)" },
        { "&Undo", "元に戻すとか、ダッサ〜〜〜 (&U)" },
        { "&Redo", "やり直し〜〜〜？よっわ (&R)" },
        { "Cu&t", "切り取りでもしてろよ (&T)" },
        { "&Paste", "貼り付け？ぷっ、センスな (&P)" },
        { "Delete", "削除〜〜はいはい消えた〜〜〜" },
        { "Copy &Link Location", "リンクコピー？そんなのしかできないの？ (&L)" },
        { "QScrollBar::Scroll here", "ここにスクロールすりゃいいじゃん" },
        { "QScrollBar::Top", "先頭まで行けよ、ざーーこ" },
        { "QScrollBar::Bottom", "最後まで落ちとけ〜〜〜" },
        { "QScrollBar::Page up", "前のページ戻んの？だっさ〜" },
        { "QScrollBar::Page down", "次のページ行けよ、はやく〜〜〜" },
        { "QScrollBar::Scroll up", "上にスクロールしなよ、よっわ" },
        { "QScrollBar::Scroll down", "下にスクロールでもしてな〜〜〜" },
    }},
    // モニカ・エヴァレット
    { "ja_ME", {
        { "KStandardGuiItem::&OK", "OK (&O)…だ、だいじょうぶ…だと…思いまふ…" },
        { "KStandardGuiItem::&Cancel", "キャンセル (&C)…む、むりでふ…うぇぇん…" },
        { "KStandardGuiItem::&Apply", "適用 (&A)…こ、こわいけど…が、がんばりまふ…" },
        { "QFileDialog::All Files (*)", "すべてのファイル (*)、た、多分…" },
        { "QFileDialog::Look in:", "表示場所：…あ、あの…こ、ここ…ですか…" },
        { "QFileDialog::Back", "か…帰りたい、ひぃん…" },
        { "QFileDialog::Go back", "一つ戻る…こ、怖い…けど…" },
        { "QFileDialog::Alt+Left", "Alt+左" },
        { "QFileDialog::Forward", "すっ、進む…む、むり…" },
        { "QFileDialog::Go forward", "一つ進む…だ、だいじょうぶ…かな…" },
        { "QFileDialog::Alt+Right", "Alt+右" },
        { "QFileDialog::Parent Directory", "お…親ディレクトリ…こ、ここ…でふ…" },
        { "QFileDialog::Go to the parent directory", "お…親ディレクトリへ移動…あ、あの…ゆ、勇気だして…" },
        { "QFileDialog::Alt+Up", "Alt+上" },
        { "QFileDialog::Create New Folder", "新しいフォルダーを作成…で、できる…はず…" },
        { "QFileDialog::Create a New Folder", "新しいフォルダーを作成…が、がんばりまふ…" },
        { "QFileDialog::List View", "リスト表示…み、見やすい…はず…" },
        { "QFileDialog::Change to list view mode", "リスト表示に切り替え…ご、ごめんなさい…見づらかったら…" },
        { "QFileDialog::Detail View", "詳細表示…こ、細かいの…こわい…" },
        { "QFileDialog::Change to detail view mode", "詳細表示に切り替え…ど、どうでしょう…" },
        { "QFileDialog::Sidebar", "サイドバー…ふ、ふみゅ…" },
        { "QFileDialog::List of places and bookmarks", "場所とブックマークの一覧…お、覚えきれない…かも…" },
        { "QFileDialog::Files", "ファイル…あ、あの…これ…です…" },
        { "QFileDialog::Files of type:", "ファイルの種類：…え、えっと…" },
        { "QFileDialog::File &name:", "ファイル名 (&N)：…ど、どれ…に…しまふ…" },
        { "QFileDialog::&Open", "ひ、ひぃん…あ、あのっ…ひ、開く (&O)…でふぅ…" },
        { "QFileDialog::Show &hidden files", "み、みみっ…み、見えちゃう…こ、こわいぃぃ…隠しファイルを表示 (&H)…うぇぇん…" },
        { "QFileDialog::&New Folder", "し、新規フォルダー (&N)…た、多分…で、できます…ひぃん…" },
        { "QFileDialog::&Rename", "な、ななな、名前を…へ、変更 (&R)…ま、間違えたら…ご、ごめんなさいぃぃ…" },
        { "QFileDialog::&Delete", "い、いやぁぁぁ…ひ、ひぃん…さ、削除 (&D)…こ、怖いぃぃ…" },
        { "QFileDialog::&Choose", "え、えっと…え、選択 (&C)…え、えらべ…ま、ます…た、多分…でふっ…" },
        { "QFileDialog::Remove", "削除…ご、ごめんなさい…" },
        { "QFileDialog::Directory:", "ディレクトリ：" },
        { "QFileSystemModel::Name", "な、名前…でふ…" },
        { "QFileSystemModel::Size", "サ、サイズ…お、大きいと…こ、怖…" },
        { "QFileSystemModel::Type", "し、種類…え、えっと…" },
        { "QFileSystemModel::Date Modified", "こ、更新日時…お、遅れたら…ご、ごめ…" },
        { "QFileSystemModel::Computer", "こ、コンピュータ…ひ、ひぃん…" },
        { "KMessageWidget::&Close", "と、とじる (&C)…し、失礼しまふ…" },
        { "KMessageWidget::Close message", "メッセージ…と、とじる…ご、ごめ…" },
        { "KMessageWidget::Note", "ちゅ、注意…あ、あの…" },
        { "KMessageWidget::Error", "ひっ！え、エラー…こ、怖…うぇ…" },
        { "KStandardActions::&Up", "う、上へ (&U)…" },
        { "KStandardActions::Go up", "う、上へ…い、移動…" },
        { "KStandardActions::&Refresh", "こ、更新 (&R)…が、がんば…" },
        { "KStandardActions::Refresh document", "ぶ、文書…こ、更新…ま、まだ…" },
        { "KStandardActions::&Rename…", "な、名前…へ、変更 (&R)…こ、怖…" },
        { "KStandardActions::&Add Bookmark", "ブ、ブックマーク…つ、追加 (&B)…い、一応…" },
        { "KBookmarkMenu::New Bookmark Folder…", "新しいブックマーク…つ、作…" },
        { "KBookmarkMenu::Create a new bookmark folder in this menu", "このメニューに新しいブックマーク…え、えっと…" },
        { "QIODevice::No such file or directory", "ファイルやディレクトリなし…ご、ごめ…むり…" },
        { "QAbstractFileIconProvider::Drive", "ドライブ…ふ…" },
        { "QAbstractFileIconProvider::Folder", "フォルダー…あ、あ…" },
        { "QAbstractFileIconProvider::Shortcut", "ショートカット…ち、近…" },
        { "&Copy", "こ、コピー (&C)…ま、間違えたら…ご、ごめ…" },
        { "Select All", "ぜ、全部…せ、選択…" },
        { "&Undo", "も、元に…も、戻す (&U)…た、多分…" },
        { "&Redo", "や、やり直す (&R)…む、無理…じゃ…ない…" },
        { "Cu&t", "き、切り取り (&T)…ひ、ひぃん…" },
        { "&Paste", "は、貼り付け (&P)…ど、どこ…に…" },
        { "Delete", "さ、削除…い、いやぁ…" },
        { "Copy &Link Location", "リンク場所をこ…こ…コピ…ころさないで (&L)…" },
        { "QScrollBar::Scroll here", "っ…こ、ここまで…す、スクロール…す、す、すこし…だけ…" },
        { "QScrollBar::Top", "ひ、ひぃん…う、上端…" },
        { "QScrollBar::Bottom", "う、うぇぇ…し、下端…" },
        { "QScrollBar::Page up", "1ぺ、ページ…う、上…が、がんば…" },
        { "QScrollBar::Page down", "1ぺ、ページ…し、下…こ、怖…" },
        { "QScrollBar::Scroll up", "あ、あぅ…う、上に…す、スクロール…" },
        { "QScrollBar::Scroll down", "む、むり…し、下に…す、スクロール…"
    } }
    /*
    // 悪役令嬢 v2
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
