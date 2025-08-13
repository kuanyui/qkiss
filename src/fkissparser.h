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

#ifndef FKISSPARSER_H
#define FKISSPARSER_H

#include <QObject>
#include "fkiss.h"

// ================================================
// Token
// ================================================

struct Token {
    enum class Type {
        Identifier, // CommandName | VariableName
        Space,
        LParen,
        RParen,
        Comma,
        String,
        Integer,
        ObjectID,
        EndOfLine
    };
    Token(Type tokenType, const QString& value, int lineIdx, int columnIdx) : type(tokenType), value(value), lineIdx(lineIdx), columnIdx(columnIdx) {}
    static QString stringifyType(Type type);
    int lineNo() const { return lineIdx + 1; }
    int columnNo() const { return columnIdx + 1; }
    Type type;
    QString value;
    int lineIdx;
    int columnIdx;
};
QDebug operator<<(QDebug debug, const Token& token);
QDebug operator<<(QDebug debug, const Token::Type& tokenType);


class FkissParser
{
public:
    FkissParser();
    QVector<std::shared_ptr<ASTPairedCommands> > parseScriptLines(const QList<FkissLineInfo> &lineInfoList);
private:
    static bool commandIsEvent(const QString &command);
    static bool commandIsAction(const QString &command);
    /** source code -> tokens */
    QVector<Token> tokenize(const QList<FkissLineInfo> &lineInfoList);
    /** tokens -> AST (flat, no semantic, so no nested structure (e.g. `alert() ...`, `ifeq() ...` )  */
    QVector<std::shared_ptr<ASTCommand> > parse0(const QVector<Token> &tokens);
    /** Returns the index of nearest Token. If not found, returns -1. */
    int findNearestToken(const QVector<Token> &tokens, Token::Type type, int fromIndex) const;
    /** Remove `;@` prefix and `; comment` suffix. */
    QString removePrefixAndCommentOfLine(QString line);
    /** AST -> Commands (Event command + Action command(s)). Traverse across all commands, bind each event + its action(s) (procedure) pair, and store to pairs list. */
    QVector<std::shared_ptr<ASTPairedCommands>> pairAstCommands(FkissProcedure &commands);;
    void logSyntaxWarning(const Token &token, QString msg) const;
    void logSyntaxError(const Token &token, QString msg) const;
};

#endif // FKISSPARSER_H
