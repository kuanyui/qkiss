#include "fkiss.h"
#include <QRegularExpression>


Fkiss::Fkiss(QObject *parent)
    : QObject{parent}
{

}

void Fkiss::compileScriptLines(const QList<FkissLineInfo> &lineInfoList)
{
    QVector<Token> tokens = tokenize(lineInfoList);
    // qDebug() << "[Fkiss] parsed tokens count: " << tokens.length();
    // foreach (Token token, tokens) {
    //     qDebug() << "[Fkiss]" << token;
    // }
    QVector<ASTCommand> ast0 = parse0(tokens);
}

QVector<Token> Fkiss::tokenize(const QList<FkissLineInfo> &lineInfoList) {
    QVector<Token> tokens;
    for (int r = 0; r < lineInfoList.length(); ++r) {  // row index (line index)
        auto lineInfo = lineInfoList[r];
        QString line = removePrefixAndCommentOfLine(lineInfo.line);
        int c = 0;  // column index
        while (c < line.length()) {
            if (line[c].isSpace()) {
                int begC = c;
                c++;
                while (c < line.length() && line[c].isSpace()) {
                    c++;
                }
                tokens.append(Token(Token::Type::Space, line.mid(begC, c - begC), r, begC));
            } else if (line[c].isLetter()) {
                int begC = c;
                while (c < line.length() && (line[c].isLetterOrNumber() || line[c] == '_')) {
                    c++;
                }
                tokens.append(Token(Token::Type::Identifier, line.mid(begC, c - begC), r, begC));
            } else if (line[c] == '(') {
                tokens.append(Token(Token::Type::LParen, "(", r, c));
                c++;
            } else if (line[c] == ')') {
                tokens.append(Token(Token::Type::RParen, ")", r, c));
                c++;
            } else if (line[c] == ',') {
                tokens.append(Token(Token::Type::Comma, ",", r, c));
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
                        tokens.append(Token(Token::Type::String, cleanStr, r, begC));
                        break;  // end of a string
                    }
                    c++;
                }
                if (!foundStr) {
                    qCritical() << QStringLiteral("[ERROR][Fkiss::Tokenizer] Found a double quote mark, but cannot find another paired quote mark to form a string. SOURCE: ").arg(lineInfo.line);
                    break;
                }
            } else if (line[c] == '#') {
                int begC = c;
                c++;
                while (c < line.length() && line[c].isDigit()) {
                    c++;
                }
                tokens.append(Token(Token::Type::ObjectID, line.mid(begC, c - begC), r, begC));
            } else if (line[c].isDigit() || (c+1 < line.length() && line[c] == '-' && line[c+1].isDigit())) {
                int begC = c;
                c++;
                while (c < line.length() && line[c].isDigit()) {
                    c++;
                }
                tokens.append(Token(Token::Type::Integer, line.mid(begC, c - begC), r, begC));
            } else {
                qCritical() << QStringLiteral("[ERROR][Fkiss::Tokenizer] Unexpected character [%1:%2]: %3").arg(lineInfo.lineNoInCnf).arg(c).arg(lineInfo.line);
                c++;
            }
        }
        tokens.append(Token(Token::Type::EndOfLine, "\n", r, line.length()));
    }
    return tokens;
}

QVector<ASTCommand> Fkiss::parse0(const QVector<Token> &tokens)
{
    QVector<ASTCommand> astCmds;
    bool bodyStarted = false;
    for (int i = 0; i < tokens.length(); i++) {
        const Token &token = tokens[i];
        if (!bodyStarted) {
            if (token.type == Token::Type::Space) { continue; }
            if (token.type == Token::Type::EndOfLine) { continue; }
            if (token.type == Token::Type::Identifier && token.value == "EventHandler") {
                bodyStarted = true;
            } else {
                logSyntaxError(token, "Command starts before EventHandler, ignore.");
            }
            continue;
        }
        if (token.type == Token::Type::Space || token.type == Token::Type::EndOfLine) {
            continue;
        }
        // Parse a full command calling
        if (token.type == Token::Type::Identifier) {
            QString cmdName = token.value;
            const Token &lPar = tokens[++i];
            if (lPar.type != Token::Type::LParen) {
                logSyntaxError(lPar, "Command name must be followed by a left parenthesis. Abort.");
            }
            // Find out args list begin & end position.
            const int parenBeg = i;
            const int parenEnd = findNearestToken(tokens, Token::Type::RParen, parenBeg);
            if (parenEnd == -1) {
                logSyntaxError(lPar, "Cannot find the matched right parenthesis. Abort.");
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
            QVector<Variant> cmdArgs;
            bool waitComma = false;
            for (int j = 0; j < argsTokensWithoutParensAndSpaces.length(); j++) {
                const Token *tk = &tokens[j];
                if (tk->type == Token::Type::Comma) {
                    if (waitComma) {
                        waitComma = false;
                    } else {
                        logSyntaxError(*tk, "Comma must be a separator between two args.");
                    }
                } else  if (tk->type == Token::Type::Integer) {
                    cmdArgs.append(Variant::fromInt(tk->value));
                    waitComma = true;
                } else if (tk->type == Token::Type::String) {
                    cmdArgs.append(Variant::fromString(tk->value));
                    waitComma = true;
                } else if (tk->type == Token::Type::ObjectID) {
                    cmdArgs.append(Variant::fromObjectId(tk->value));
                    waitComma = true;
                } else if (tk->type == Token::Type::Identifier) {
                    logSyntaxError(*tk, "Currently FKiSS3 variable, or FKiSS4 named label, is not supported.");
                    waitComma = true;
                }
            }
            const auto &nextTk = tokens[i+1];
            if (nextTk.type != Token::Type::EndOfLine && nextTk.type != Token::Type::Space) {
                logSyntaxError(nextTk, "A command must be followed by a space or an EOL. Still runnable but this is a wrong code.");
            }
            astCmds.append(ASTCommand{cmdName, cmdArgs});
        }
    }
    return astCmds;
}

int Fkiss::findNearestToken(QVector<Token> tokens, Token::Type type, int fromIndex)
{
    for (int i = fromIndex+1; i < tokens.length(); i++) {
        const Token &tk = tokens[i];
        if (tk.type == type) {
            return i;
        }
    }
    return -1;
}



QString Fkiss::removePrefixAndCommentOfLine(QString line) {
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

void Fkiss::logSyntaxError(const Token &token, QString msg)
{
    qDebug() << QStringLiteral("[FKiSS][SyntaxError] %1   [L%2, C%3] %4")
                .arg(msg)
                .arg(token.lineNo())
                .arg(token.columnNo())
                .arg(token.value);
}

QDebug operator<<(QDebug debug, const Token &token) {
    return debug.nospace() << "Token { type: " << token.type << ", value: " << token.value << " }";
}
QDebug operator<<(QDebug debug, const Token::Type &tokenType) {
    QString str;
    switch (tokenType) {
    case Token::Type::Identifier: str = QStringLiteral("Identifier"); break;
    case Token::Type::Space: str = QStringLiteral("Space"); break;
    case Token::Type::LParen: str = QStringLiteral("LParen"); break;
    case Token::Type::RParen: str = QStringLiteral("RParen"); break;
    case Token::Type::Comma: str = QStringLiteral("Comma"); break;
    case Token::Type::String: str = QStringLiteral("String"); break;
    case Token::Type::Integer: str = QStringLiteral("Integer"); break;
    case Token::Type::ObjectID: str = QStringLiteral("ObjectID"); break;
    case Token::Type::EndOfLine: str = QStringLiteral("EndOfLine"); break;
    }
    return debug.nospace() << str;
}
