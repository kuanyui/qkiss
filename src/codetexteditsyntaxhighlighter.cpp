/**************************************************************************
* QKiSS - A modern implementation of KISS (KISekae Set system) viewer in Qt 6.
*
* Copyright (C) 2024 ono ono (kuanyui) <azazabc123 АТ GМАIL.СОМ>
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
*
* ===============================================================
*
* And this file is forked from Qt official documentation:
*
* - https://doc.qt.io/qt-6.2/qtwidgets-widgets-codeeditor-example.html
* - https://doc.qt.io/qt-6.2/qtwidgets-richtext-syntaxhighlighter-example.html
*
* According to https://doc.qt.io/qt-6/examples-license.html , the following
* is a copy of the license:
*
* ===============================================================
*
* Copyright (C) 2018 The Qt Company Ltd.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*  * Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*  * Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
*  * Neither the name of The Qt Company Ltd nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
**************************************************************************/
#include "codetexteditsyntaxhighlighter.h"

using namespace Qt::Literals::StringLiterals;

CodeTextEditSyntaxHighlighter::CodeTextEditSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter{parent}
{

}


void CodeTextEditSyntaxHighlighter::highlightBlock(const QString &text)
{
    auto stxPlt = m_themeManager.syntaxHighlightFormatPalette();
    // Kcf line
    static QRegularExpression reKcf(R"#(^%(\S+))#");
    QRegularExpressionMatchIterator it = reKcf.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        setFormat(m.capturedStart(1), m.capturedLength(1), stxPlt.purple);
    }

    // Cel line
    static QRegularExpression reCel(R"#(^#(\d+)(?:[.](\d*))?[\s　]+([^\s\t　*:]+)(?:[\s　]*[*][\s　]*?(\d+))?(?:[\s　]*[:][\s　]*?([\d\s　]+))?(?:[;；](.*))?)#");
    it = reCel.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        setFormat(m.capturedStart(1), m.capturedLength(1), stxPlt.blue);
        setFormat(m.capturedStart(2), m.capturedLength(2), stxPlt.red);
        setFormat(m.capturedStart(3), m.capturedLength(3), stxPlt.green);
        setFormat(m.capturedStart(4), m.capturedLength(4), stxPlt.purple);
        setFormat(m.capturedStart(5), m.capturedLength(5), stxPlt.orange);
    }

    // Obj Id
    static QRegularExpression reObjId(R"#((#)([0-9]+))#");
    it = reObjId.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        setFormat(m.capturedStart(1), m.capturedLength(1), stxPlt.yellow);
        setFormat(m.capturedStart(2), m.capturedLength(2), stxPlt.blue);
    }

    // set
    static QRegularExpression reSet0(QStringLiteral(R"#(^(\$)([0-9]))#"));
    it = reSet0.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        setFormat(m.capturedStart(1), m.capturedLength(1), stxPlt.orange);
        setFormat(m.capturedStart(2), m.capturedLength(2), stxPlt.magenta);
    }

    // quoteation
    static QRegularExpression reQuotes(QStringLiteral("\".*?\""));
    it = reQuotes.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        setFormat(m.capturedStart(0), m.capturedLength(0), stxPlt.cyan);
    }
    // command
    static QRegularExpression reCmd(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    it = reCmd.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        setFormat(m.capturedStart(0), m.capturedLength(0), stxPlt.green);
    }

    // comment
    QList<QRegularExpression> reComments {
        QRegularExpression{";[^@].*"},
        QRegularExpression{"^;@?"}
    };
    for (auto re : std::as_const(reComments)) {
        it = re.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            setFormat(m.capturedStart(0), m.capturedLength(0), stxPlt.gray);
        }
    }


    // setCurrentBlockState(0);

    // int startIndex = 0;
    // if (previousBlockState() != 1)
    //     startIndex = text.indexOf(m_commentStartExpression);

    // while (startIndex >= 0) {
    //     QRegularExpressionMatch match = m_commentEndExpression.match(text, startIndex);
    //     int endIndex = match.capturedStart();
    //     int commentLength = 0;
    //     if (endIndex == -1) {
    //         setCurrentBlockState(1);
    //         commentLength = text.length() - startIndex;
    //     } else {
    //         commentLength = endIndex - startIndex
    //                         + match.capturedLength();
    //     }
    //     setFormat(startIndex, commentLength, m_multiLineCommentFormat);
    //     startIndex = text.indexOf(m_commentStartExpression, startIndex + commentLength);
    // }
}


