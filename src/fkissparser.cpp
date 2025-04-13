#include "fkissparser.h"


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
    // qDebug() << "[Fkiss] parsed tokens count: " << tokens.length();
    // foreach (Token token, tokens) {
    //     qDebug() << "[Fkiss]" << token;
    // }
    FkissProcedure ast0 = parse0(tokens);
    // foreach (ASTCommand astNode, ast0) {
    //     qDebug() << "[Fkiss]" << astNode;
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
        QString line = removePrefixAndCommentOfLine(lineInfo.line);
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
                    qCritical() << QStringLiteral("[ERROR][Fkiss::Tokenizer] Found a double quote mark, but cannot find another paired quote mark to form a string. SOURCE: ").arg(lineInfo.line);
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
                qCritical() << QStringLiteral("[ERROR][Fkiss::Tokenizer] Ignore this character because it is unexpected [%1:%2]: `%3`").arg(lineInfo.lineNoInCnf).arg(c).arg(lineInfo.line);
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
    // foreach (const Token &token , tokens) {
    //     qDebug() << "[DEBUG] Current Token = " << token;
    // }
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
            QVector<FkissVariant> cmdArgs;
            bool waitComma = false;
            for (int j = 0; j < argsTokensWithoutParensAndSpaces.length(); j++) {
                const Token *tk = argsTokensWithoutParensAndSpaces[j];
                if (tk->type == Token::Type::Comma) {
                    if (waitComma) {
                        waitComma = false;
                    } else {
                        logSyntaxError(*tk, "Comma must be a separator between two args.");
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
                    logSyntaxError(*tk, "Currently FKiSS3 variable, or FKiSS4 named label, is not supported.");
                    waitComma = true;
                }
            }
            const auto &nextTk = tokens[i+1];
            if (nextTk.type != Token::Type::EndOfLine && nextTk.type != Token::Type::Space) {
                // NOTE: Comment temporarily to reduce unwanted warnings for debugging more easily...
                // logSyntaxError(nextTk, "A command must be followed by a space or an EOL. Still runnable but this is a wrong code.");
            }
            astCmds.append(std::make_shared<ASTCommand>(ASTCommand{cmdName, cmdArgs}));
        }
    }
    return astCmds;
}

int FkissParser::findNearestToken(QVector<Token> tokens, Token::Type type, int fromIndex)
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
            qWarning().noquote() << QStringLiteral("[WARNING][Fkiss] Unknown command \"%1\", skip.").arg(cmd->name);
        }
    }
    evpairs.removeFirst();  // remove sentinel
    return evpairs;
}

void FkissParser::logSyntaxError(const Token &token, QString msg)
{
    qDebug() << QStringLiteral("[FKiSS][SyntaxError] %1   [L%2, C%3] %4")
                .arg(msg)
                .arg(token.lineNo())
                .arg(token.columnNo())
                .arg(token.value);
}
