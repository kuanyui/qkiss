/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2025 ono ono (kuanyui) <azazabc123@gmail.com>
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
#include <QDialogButtonBox>
#include <QApplication>
#include <QCoreApplication>
#include <QRegularExpression>

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
    resize(500, 500);

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
    labelAppVersion->setText("1.0.0");
    labelAppVersion->setAlignment(Qt::AlignmentFlag::AlignCenter);
    labelAppVersion->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    layoutHeader->addWidget(labelAppVersion);

    QLabel *labelSummary = new QLabel{this};
    labelSummary->setText(md2Html(tr("QKiSS is a modern implementation of [KiSS](https://ja.wikipedia.org/wiki/KISekae_Set_system) viewer in Qt 6.")));
    labelSummary->setAlignment(Qt::AlignmentFlag::AlignCenter);
    labelSummary->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    layoutHeader->addWidget(labelSummary);

    QLabel *labelCopyright= new QLabel{this};
    labelCopyright->setText("Copyright (C) 2024 - 2025 kuanyui. All rights reserved.");
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
QKiSS is a modern implementation of [KiSS (Kisekae Set System)](https://ja.wikipedia.org/wiki/KISekae_Set_system) viewer in Qt 6.


Kisekae is short for *"kisekae ningyou"* (Japanese: *着せ替え人形*, meaning *"dress-up dolls"*).
KiSS originated from and was popular in Japanese [otaku](https://en.wikipedia.org/wiki/Otaku) and personal computer enthusiast (NEC PC-9800 series) subculture in the 1990s.

[Website of QKiSS](https://github.com/kuanyui/qkiss).
)")));
    m_tabWidget->addTab(browserInfo, tr("Information"));

    // Authors tab
    QTextBrowser *browserAuthors = new QTextBrowser{this};
    browserAuthors->setOpenExternalLinks(true);
    browserAuthors->setReadOnly(true);
    browserAuthors->setHtml(md2Html(QStringLiteral(R"(
# %1
- [kuanyui](https://github.com/kuanyui)

# %2
- [kuanyui](https://github.com/kuanyui)
- [GNOME Projects](https://www.gnome.org)
- [Ubuntu Humanity](https://launchpad.net/humanity)
- [GIMP](https://www.gimp.org/)
- [Inkscape](https://inkscape.org/)

# %3
- English: [kuanyui](https://github.com/kuanyui)
- 日本語: Claude Code + [kuanyui](https://github.com/kuanyui)
- 日本語（関西弁）: Copilot + [kuanyui](https://github.com/kuanyui)
- 日本語（お嬢様）: Copilot + [kuanyui](https://github.com/kuanyui)
- 日本語（悪役令嬢）: Copilot + [kuanyui](https://github.com/kuanyui)
- 日本語（メ〇〇キ）: Copilot + [kuanyui](https://github.com/kuanyui)
- 日本語（〈沈黙の魔女〉モニカ・エヴァレット）: Copilot + モニカ・ノートン
- 台灣繁體: Claude Code + [kuanyui](https://github.com/kuanyui)
)")).arg(tr("Programming"),
         tr("Icons & Arts"),
         tr("Translators")
         ));
    m_tabWidget->addTab(browserAuthors, tr("Authors"));

    // Licenses tab
    QTextBrowser *browserLicense = new QTextBrowser{this};
    browserLicense->setOpenExternalLinks(true);
    browserLicense->setReadOnly(true);
    browserLicense->setHtml(
        md2Html(
            tr(R"(QKiSS is licensed under GPLv3.

QKiSS used some third-parties SVG icons from other FLOSS projects via `[tango-icon-remixer](https://github.com/kuanyui/tango-icon-remixer)`, and they are released under various licenses:)") +
            QStringLiteral(R"(

- [gnome-icon-theme-fork](https://github.com/kuanyui/gnome-icon-theme-fork): `LGPL v3` or `CC by-sa 3.0`
- [GNOME Projects](https://www.gnome.org): `LGPL v3` or `CC by-sa 3.0`
- [Ubuntu Humanity](https://launchpad.net/humanity): `GPL v2`
- [GIMP](https://www.gimp.org/): `CC by-sa 3.0`
- [Inkscape](https://inkscape.org/): `GPL v2`
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
    // qDebug() << "\n";
    // qDebug() << html;
    return html;
}


