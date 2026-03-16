/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2025 - 2026 ono ono (kuanyui) <azazabc123@gmail.com>
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
#include "aboutdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTextBrowser>
#include <QLabel>
#include <QIcon>
#include <QDateTime>
#include <QTimeZone>
#include <QDialogButtonBox>
#include <QApplication>
#include <QCoreApplication>
#include <QRegularExpression>
#include "configfile.h"
#include "version.h"
#include "logging.h"

using namespace Qt::Literals::StringLiterals;

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , m_tabWidget(new QTabWidget(this))
{
    setupUi();
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::setupUi()
{
    setWindowTitle(tr("About QKiSS"));
    resize(800, 700);

    QVBoxLayout *layoutMain = new QVBoxLayout{this};
    QVBoxLayout *layoutHeader = new QVBoxLayout{this};
    layoutHeader->setAlignment(Qt::AlignmentFlag::AlignCenter);

    QLabel *labelIcon = new QLabel{};
    QIcon appIcon = QIcon::fromTheme("qkiss");
    labelIcon->setPixmap(appIcon.pixmap(48, 48));
    labelIcon->setAlignment(Qt::AlignmentFlag::AlignCenter);
    labelIcon->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    layoutHeader->addWidget(labelIcon);


    QLabel *labelAppName = new QLabel{this};
    labelAppName->setText("QKiSS");
    labelAppName->setAlignment(Qt::AlignmentFlag::AlignCenter);
    labelAppName->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    layoutHeader->addWidget(labelAppName);

    QLabel *labelAppVersion = new QLabel{this};
    labelAppVersion->setText(PROJECT_VERSION);
    labelAppVersion->setAlignment(Qt::AlignmentFlag::AlignCenter);
    labelAppVersion->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);

    QDateTime commitDateTime = QDateTime::fromSecsSinceEpoch(GIT_COMMIT_TIME_UNIX_SEC, QTimeZone::utc());  // `QTimeZone::UTC` is since Qt6.5
    QString commitDateTimeStr = commitDateTime.toString("yyyy-MM-dd (ddd) HH:mm 'UTC'");
    labelAppVersion->setToolTip("Commit Hash: %1\nCommit Time: %2"_L1.arg(GIT_COMMIT_HASH, commitDateTimeStr));
    labelAppVersion->setCursor(Qt::CursorShape::WhatsThisCursor);
    layoutHeader->addWidget(labelAppVersion);

    QLabel *labelSummary = new QLabel{this};
    labelSummary->setText(md2Html(tr("QKiSS is a modern implementation of [KiSS](https://ja.wikipedia.org/wiki/KISekae_Set_system) viewer in Qt 6.")));
    labelSummary->setAlignment(Qt::AlignmentFlag::AlignCenter);
    labelSummary->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    layoutHeader->addWidget(labelSummary);

    QLabel *labelCopyright= new QLabel{this};
    labelCopyright->setText("Copyright (C) 2024 - 2026 kuanyui. All rights reserved.");
    labelCopyright->setAlignment(Qt::AlignmentFlag::AlignCenter);
    labelCopyright->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    layoutHeader->addWidget(labelCopyright);


    layoutMain->addLayout(layoutHeader);
    layoutMain->addWidget(m_tabWidget);

    // Information
    QTextBrowser *browserInfo = new QTextBrowser{this};
    browserInfo->setOpenExternalLinks(true);
    browserInfo->setReadOnly(true);
    browserInfo->setHtml(md2Html(tr(R"(
QKiSS is a modern implementation of [KiSS (Kisekae Set System)](https://ja.wikipedia.org/wiki/KISekae_Set_system) viewer in Qt 6, compatible with KISS/GS and FKiSS 1 & 2 specifications, as a complete rebirth inspired by [fkiss](https://www2s.biglobe.ne.jp/~yav/kiss/index.html) and [GnomeKiSS](https://devel.tlrmx.org/kiss/) on Linux.

"Kisekae" comes from the Japanese word "着せ替え人形" (pronunciation: "kisekae ningyou"), meaning "dress-up dolls".

KiSS originated from Japanese [otaku](https://ja.wikipedia.org/wiki/おたく) subculture and personal computer enthusiasts community (particularly those using the [NEC PC-9800 series](https://ja.wikipedia.org/wiki/PC-9800シリーズ)) in the 1990s, and gained popularity throughout the 1990s and early 2000s.

This project comes from nostalgia for the Windows 95/98 and GTK+ 2 era —— a lost age before Internet was polluted by AI-generated junk (this project does use some AI though). After all, life is frightening enough as it is; the real world is a hot mess, while [nijigen](https://ja.wikipedia.org/wiki/2次元) and [isekai](https://w.wiki/4So4) remain a pure promised land.

[Website of QKiSS](https://github.com/kuanyui/qkiss).
)")));
    m_tabWidget->addTab(browserInfo, tr("Information"));

    // Authors tab
    QTextBrowser *browserAuthors = new QTextBrowser{this};
    browserAuthors->setOpenExternalLinks(true);
    browserAuthors->setReadOnly(true);
    QString authorsMd = QStringLiteral(R"(
# %1
- [kuanyui](https://github.com/kuanyui)

# %2
- [kuanyui](https://github.com/kuanyui)
- [GNOME Projects](https://www.gnome.org)
- [Ubuntu Humanity](https://launchpad.net/humanity)
- [GIMP](https://www.gimp.org/)
- [Inkscape](https://inkscape.org/)
- [flag-icons](https://github.com/lipis/flag-icons)

@@@@@
# %3
- English: [kuanyui](https://github.com/kuanyui)
- 日本語: Claude Code + [kuanyui](https://github.com/kuanyui)
- 日本語（関西弁）: Copilot + [kuanyui](https://github.com/kuanyui)
- 日本語（お嬢様）: Copilot + [kuanyui](https://github.com/kuanyui)
- 日本語（悪役令嬢）: Copilot + [kuanyui](https://github.com/kuanyui)
- 日本語（メ〇〇キ）: Copilot + [kuanyui](https://github.com/kuanyui)
- 日本語（〈沈黙の魔女〉モニカ・エヴァレット）: Copilot + モニカ・ノートン
- 台灣繁體: Claude Code + [kuanyui](https://github.com/kuanyui)
)")
                          .arg(tr("Programming"),
                               tr("Icons & Arts"),
                               tr("Translators"));
    if (ConfigFile::engOnly()) {
        int idx = authorsMd.indexOf("\n@@@@@\n");
        if (idx != -1) {
            authorsMd.truncate(idx);
        }
    } else {
        authorsMd.remove("\n@@@@@\n");
    }
    browserAuthors->setHtml(md2Html(authorsMd));
    m_tabWidget->addTab(browserAuthors, tr("Authors"));

    // Licenses tab
    QTextBrowser *browserLicense = new QTextBrowser{this};
    browserLicense->setOpenExternalLinks(true);
    browserLicense->setReadOnly(true);
    browserLicense->setHtml(
        md2Html(
            tr(R"(QKiSS is licensed under GPLv3.

QKiSS used some third-parties SVG icons from other FLOSS projects via `[tango-remixer](https://github.com/kuanyui/tango-remixer)`, and they are released under various licenses:)") +
            QStringLiteral(R"(
- [Tango Icon Theme](https://github.com/kuanyui/tango-icon-theme): `GPL v2` or `GPL v3`
- [Nome Icon Theme](https://github.com/kuanyui/nome-icon-theme): `LGPL v3` or `CC by-sa 3.0`
- [GIMP](https://www.gimp.org/): `CC by-sa 3.0`
- [flag-icons](https://github.com/lipis/flag-icons): `MIT`
)")));
    m_tabWidget->addTab(browserLicense, tr("Licenses"));

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
    layoutMain->addWidget(buttonBox);
}

QString AboutDialog::md2Html(const QString &markdown) {
    static const QRegularExpression reH1(R"(^# (.+)$)", QRegularExpression::PatternOption::MultilineOption);
    static const QRegularExpression reH2(R"(^## (.+)$)", QRegularExpression::PatternOption::MultilineOption);
    static const QRegularExpression reH3(R"(^### (.+)$)", QRegularExpression::PatternOption::MultilineOption);
    static const QRegularExpression reCode(R"(`(.+?)`)");
    static const QRegularExpression reBoldItalic(R"(\*\*\*(.+?)\*\*\*)");
    static const QRegularExpression reBold(R"(\*\*(.+?)\*\*)");
    static const QRegularExpression reItalic(R"(\*([^*]+?)\*)");
    static const QRegularExpression reLink(R"(\[([^\]]+)\]\(([^)]+)\))");
    static const QRegularExpression reParagraph(R"(\n\n+)");
    static const QRegularExpression reEmptyPara(R"(<p>\s*</p>)");

    static const QRegularExpression reUlLi(R"(^[*-] (.+)$)", QRegularExpression::PatternOption::MultilineOption);
    static const QRegularExpression reUlLiWorkaround(R"(<\/ul>\n*<ul>)");

    QString html = markdown;

    html.replace('&', "&amp;");
    html.replace('<', "&lt;");
    html.replace('>', "&gt;");

    html.replace(reUlLi, R"(<ul><li>\1</li></ul>)");
    html.replace(reUlLiWorkaround, "");

    html.replace(reLink, R"(<a href="\2" title="\2">\1</a>)");   // `title=""` is for tooltip

    html.replace(reH3, R"(<h5>\1</h5>)");
    html.replace(reH2, R"(<h4>\1</h4>)");
    html.replace(reH1, R"(<h3>\1</h3>)");

    html.replace(reCode, R"(<code>\1</code>)");
    html.replace(reBoldItalic, R"(<b><i>\1</i></b>)");
    html.replace(reBold, R"(<b>\1</b>)");
    html.replace(reItalic, R"(<i>\1</i>)");


    html.replace(reParagraph, "</p><p>");
    html = "<p>" + html + "</p>";
    html.replace(reEmptyPara, "");
    // nsDebug() << "\n";
    // nsDebug() << html;
    return html;
}
