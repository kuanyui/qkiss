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

#include "fkissparser.h"
#include "configfile.h"
#include "logging.h"
#include <QRegularExpression>

using namespace Qt::Literals::StringLiterals;

QDebug operator<<(QDebug debug, const Token &token) {
    return debug.nospace() << QStringLiteral("Token { type: %1, value: %2, line: %3, col: %4 }")
                                        .arg(Token::stringifyType(token.type))
                                        .arg(token.value)
                                        .arg(token.lineNo())
                                        .arg(token.columnNo());
}
QDebug operator<<(QDebug debug, const Token::Type &tokenType) {
    QString str;
    Token::stringifyType(tokenType);
    return debug.nospace() << str;
}


QString Token::stringifyType(Type type)
{
    switch (type) {
    case Type::Identifier: return QStringLiteral("Identifier");
    case Type::Space: return QStringLiteral("Space");
    case Type::LParen: return QStringLiteral("LParen");
    case Type::RParen: return QStringLiteral("RParen");
    case Type::Comma: return QStringLiteral("Comma");
    case Type::String: return QStringLiteral("String");
    case Type::Integer: return QStringLiteral("Integer");
    case Type::ObjectID: return QStringLiteral("ObjectID");
    case Type::EndOfLine: return QStringLiteral("EndOfLine");
    }
    return QStringLiteral("[UnknownType]");
}

FkissParser::FkissParser()
{

}

QVector<std::shared_ptr<ASTPairedCommands> > FkissParser::parseScriptLines(const QList<FkissLineInfo> &lineInfoList)
{
    QVector<Token> tokens = tokenize(lineInfoList);
    // qkDebug << "parsed tokens count: " << tokens.length();
    // foreach (Token token, tokens) {
    //     qkDebug << "[Fkiss]" << token;
    // }
    FkissProcedure ast0 = parse0(tokens);
    // foreach (ASTCommand astNode, ast0) {
    //     qkDebug << "[Fkiss]" << astNode;
    // }
    return pairAstCommands(ast0);
}

bool FkissParser::commandIsEvent(const QString &command)
{
    return Fkiss::KEYWORDS_EVENT.contains(command);
}

bool FkissParser::commandIsAction(const QString &command)
{
    return Fkiss::KEYWORDS_ACTION.contains(command);
}

QVector<Token> FkissParser::tokenize(const QList<FkissLineInfo> &lineInfoList) {
    QVector<Token> tokens;
    for (int r = 0; r < lineInfoList.length(); ++r) {  // row index (line index)
        auto lineInfo = lineInfoList[r];
        int oriLineNo = lineInfo.lineNoInCnf;
        QString line = removePrefixAndCommentOfLine(tryToAddMissingClosingQuote(lineInfo));
        int c = 0;  // column index
        while (c < line.length()) {
            if (line[c].isSpace()) {
                int begC = c;
                c++;
                while (c < line.length() && line[c].isSpace()) {
                    c++;
                }
                tokens.append(Token(Token::Type::Space, line.mid(begC, c - begC), oriLineNo, begC));
            } else if (line[c].isLetter()) {
                int begC = c;
                while (c < line.length() && (line[c].isLetterOrNumber() || line[c] == '_')) {
                    c++;
                }
                tokens.append(Token(Token::Type::Identifier, line.mid(begC, c - begC), oriLineNo, begC));
            } else if (line[c] == '(') {
                tokens.append(Token(Token::Type::LParen, "(", oriLineNo, c));
                c++;
            } else if (line[c] == ')') {
                tokens.append(Token(Token::Type::RParen, ")", oriLineNo, c));
                c++;
            } else if (line[c] == ',') {
                tokens.append(Token(Token::Type::Comma, ",", oriLineNo, c));
                c++;
            } else if (line[c] == '"') {
                int begC = c;
                c++;
                bool foundStr = false;
                while (c < line.length()) {
                    if (line[c] == '"' && line[c-1] != '\\') {
                        foundStr = true;
                        c++;
                        QString strWithQuotes = line.mid(begC, c - begC);
                        QString str = strWithQuotes.mid(1, strWithQuotes.length() - 2);
                        QString cleanStr = str.replace(R"#(\")#", R"#(")#");
                        tokens.append(Token(Token::Type::String, cleanStr, oriLineNo, begC));
                        break;  // end of a string
                    }
                    c++;
                }
                if (!foundStr) {
                    qkCritical.noquote() << tr("Found a double quote mark, but cannot find another paired quote mark to form a string. SOURCE: ").arg(lineInfo.line);
                    break;
                }
            } else if (line[c] == '#') {
                int begC = c;
                c++;
                while (c < line.length() && line[c].isDigit()) {
                    c++;
                }
                tokens.append(Token(Token::Type::ObjectID, line.mid(begC, c - begC), oriLineNo, begC));
            } else if (line[c].isDigit() || (c+1 < line.length() && line[c] == '-' && line[c+1].isDigit())) {
                int begC = c;
                c++;
                while (c < line.length() && line[c].isDigit()) {
                    c++;
                }
                tokens.append(Token(Token::Type::Integer, line.mid(begC, c - begC), oriLineNo, begC));
            } else {
                qkCritical.noquote() << tr("Ignore this character because it is unexpected [%1:%2]: `%3`").arg(lineInfo.lineNoInCnf).arg(c).arg(lineInfo.line);
                c++;
            }
        }
        tokens.append(Token(Token::Type::EndOfLine, "\n", oriLineNo, line.length()));
    }
    return tokens;
}

QVector<std::shared_ptr<ASTCommand> > FkissParser::parse0(const QVector<Token> &tokens)
{
    FkissProcedure astCmds;
    bool bodyStarted = false;
    const QString TOKEN_EVENT_HANDLER = QStringLiteral("EventHandler");
    const QKiSS::Case caseSensitivity = ConfigFile::instance().caseSensitivity();
    bool hasCaseSensitiveEventHandler = false;   /// More tolerance for non-standard FKiSS script which use a case-insensitive `EventHandler`
    foreach (const Token &token , tokens) {
        // qkDebug << "[DEBUG] Current Token = " << token;
        if (token.value == TOKEN_EVENT_HANDLER) {
            hasCaseSensitiveEventHandler = true;
            break;
        }
    }
    for (int i = 0; i < tokens.length(); i++) {
        const Token &token = tokens[i];
        if (!bodyStarted) {
            if (token.type == Token::Type::Space) { continue; }
            if (token.type == Token::Type::EndOfLine) { continue; }
            if (token.type == Token::Type::Identifier) {
                if (hasCaseSensitiveEventHandler && token.value == TOKEN_EVENT_HANDLER) {
                    bodyStarted = true;
                } else if (!hasCaseSensitiveEventHandler && token.value.compare(TOKEN_EVENT_HANDLER, Qt::CaseSensitivity::CaseInsensitive) == 0) {
                    bodyStarted = true;
                    logSyntaxWarning(token, tr(R"(FKiSS specification requires a case-sensitive mark `EventHandler` to indicate the beginning of FKiSS Script, but encountered `%1` in this file. Ahh.... For compatibility, still accept this.)").arg(token.value));
                }
            } else {
                logSyntaxWarning(token, tr("Command starts before EventHandler, ignore."));
            }
            continue;
        }
        if (token.type == Token::Type::Space || token.type == Token::Type::EndOfLine) {
            continue;
        }
        // Parse a full command calling
        if (token.type == Token::Type::Identifier) {
            QString cmdName = token.value;
            if (caseSensitivity == QKiSS::Case::Insensitive && !commandIsEvent(cmdName) && !commandIsAction(cmdName)) {
                // FKiSS commands are all in lower case, and the original fkiss is case-sensitive. So accept other cases only when the global case sensitivity is Insensitive.
                QString lowerCmdName = cmdName.toLower();
                if (commandIsEvent(lowerCmdName) || commandIsAction(lowerCmdName)) {
                    logSyntaxWarning(token, tr("Command name `%1` is not in lower case. For compatibility, still accept it as `%2`.").arg(cmdName).arg(lowerCmdName));
                    cmdName = lowerCmdName;
                }
            }
            ++i;
            while (i+1 < tokens.length() && (tokens[i].type == Token::Type::Space)) {
	        // Add tolerance for some lunatics who add space after command (ex: `press	("hat.cel")`)
                ++i;
            }
            const Token &lPar = tokens[i];
            if (lPar.type != Token::Type::LParen) {
                logSyntaxWarning(token, tr("Command name `%1` must be followed by a left parenthesis, so assume it has no parameter.").arg(token.value));
                astCmds.append(std::make_shared<ASTCommand>(ASTCommand{cmdName, {}}));
                ++i;
                continue;
            }
            // Find out args list begin & end position.
            const int parenBeg = i;
            const int parenEnd = findNearestToken(tokens, Token::Type::RParen, parenBeg);
            if (parenEnd == -1) {
                logSyntaxError(lPar, tr("Cannot find the matched right parenthesis. Abort."));
                return astCmds;
            }
            i = parenEnd;   //
            QList<const Token*> argsTokensWithoutParensAndSpaces;
            for (int j = parenBeg + 1; j < parenEnd; j++) {
                const Token *tk = &tokens[j];
                if (tk->type == Token::Type::Space || tk->type == Token::Type::EndOfLine) {
                    continue;
                }
                argsTokensWithoutParensAndSpaces.append(tk);
            }
            QVector<FkissVariant> cmdArgs;
            bool waitComma = false;
            for (int j = 0; j < argsTokensWithoutParensAndSpaces.length(); j++) {
                const Token *tk = argsTokensWithoutParensAndSpaces[j];
                if (tk->type == Token::Type::Comma) {
                    if (waitComma) {
                        waitComma = false;
                    } else {
                        logSyntaxError(*tk, tr("Comma is required between two args."));
                    }
                } else  if (tk->type == Token::Type::Integer) {
                    cmdArgs.append(FkissVariant::fromInt(tk->value));
                    waitComma = true;
                } else if (tk->type == Token::Type::String) {
                    cmdArgs.append(FkissVariant::fromString(tk->value));
                    waitComma = true;
                } else if (tk->type == Token::Type::ObjectID) {
                    cmdArgs.append(FkissVariant::fromObjectId(tk->value));
                    waitComma = true;
                } else if (tk->type == Token::Type::Identifier) {
                    logSyntaxError(*tk, tr("Currently FKiSS3 variable, or FKiSS4 named label, is not supported."));
                    waitComma = true;
                }
            }
            // const auto &nextTk = tokens[i+1];
            // if (nextTk.type != Token::Type::EndOfLine && nextTk.type != Token::Type::Space) {
            //     // NOTE: Comment temporarily to reduce unnecessary warnings for debugging more easily...
            //     // logSyntaxWarning(nextTk, "A command must be followed by a space or an EOL. Still runnable but this is a wrong code.");
            // }
            astCmds.append(std::make_shared<ASTCommand>(ASTCommand{cmdName, cmdArgs}));
        }
    }
    return astCmds;
}

int FkissParser::findNearestToken(const QVector<Token> &tokens, Token::Type type, int fromIndex) const
{
    for (int i = fromIndex+1; i < tokens.length(); i++) {
        const Token &tk = tokens[i];
        if (tk.type == type) {
            return i;
        }
    }
    return -1;
}

QString FkissParser::removePrefixAndCommentOfLine(QString line) {
    if (line.startsWith(";@")) {
        line = line.mid(2);
    }
    bool inStr = false;
    for (int i = 0; i < line.length(); ++i) {
        if (line[i] == '"' && (i == 0 || line[i-1] != '\\')) {
            inStr = !inStr;
        } else if (line[i] == ';' && !inStr) {
            auto tmp = line.left(i);
            return tmp.trimmed();
        }
    }
    return line;
}

QString FkissParser::tryToAddMissingClosingQuote(const FkissLineInfo &lineInfo) const
{
    const QString &line = lineInfo.line;
    QList<int> quoteIdxs;
    for (int i = 0; i < line.length(); ++i) {
        if (line[i] == '"' && (i == 0 || line[i-1] != '\\')) {
            quoteIdxs.append(i);
        }
    }
    if (quoteIdxs.length() % 2 == 0) { return line; }
    static const QRegularExpression reFileNameBeforeRParenOrComma(R"#([a-z0-9_-]+[.](?:cel|wav)(?=[,)]))#", QRegularExpression::CaseInsensitiveOption);   // (?=)  means look ahead, (?<=) is behind
    for (int q = 0; q < quoteIdxs.length(); q += 2) {
        QRegularExpressionMatch m = reFileNameBeforeRParenOrComma.match(line, quoteIdxs[q] + 1, QRegularExpression::NormalMatch, QRegularExpression::AnchorAtOffsetMatchOption);
        if (m.hasMatch()) {
            QString fixedLine = line;
            fixedLine.insert(m.capturedEnd(), QChar('"'));
            logSyntaxWarning(Token(Token::Type::String, fixedLine, lineInfo.lineNoInCnf, m.capturedEnd()), tr("A string has no closing quote, so assume it ends after the file name."));
            return fixedLine;
        }
    }
    return line;
}

QVector<std::shared_ptr<ASTPairedCommands> > FkissParser::pairAstCommands(FkissProcedure &commands)
{
    QVector<std::shared_ptr<ASTPairedCommands>> evpairs;
    std::shared_ptr<ASTPairedCommands> curPair = std::make_shared<ASTPairedCommands>();  // create sentinel
    evpairs.append(curPair);   // add sentinel
    foreach (std::shared_ptr<ASTCommand> cmd, commands) {
        if (commandIsEvent(cmd->name)) {
            curPair = std::make_shared<ASTPairedCommands>();
            curPair->event = cmd;
            evpairs.append(curPair);
        } else if (commandIsAction(cmd->name)) {
            curPair->actions.append(cmd);
        } else {
            qkWarning.noquote() << tr(R"(Unknown command "%1", skip.)").arg(cmd->name);
        }
    }
    evpairs.removeFirst();  // remove sentinel
    return evpairs;
}

void FkissParser::logSyntaxWarning(const Token &token, QString msg) const
{
    qkWarning.noquote() << tr("[SyntaxWarning] %1   [L%2, C%3] %4")
                .arg(msg)
                .arg(token.lineNo())
                .arg(token.columnNo())
                .arg(token.value);
}

void FkissParser::logSyntaxError(const Token &token, QString msg) const
{
    qkCritical.noquote() << tr("[SyntaxError] %1   [L%2, C%3] %4")
                .arg(msg)
                .arg(token.lineNo())
                .arg(token.columnNo())
                .arg(token.value);
}
