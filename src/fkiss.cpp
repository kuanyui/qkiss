#include "fkiss.h"
#include <QRegularExpression>

const QHash<QString, FkissKeywordInfo> Fkiss::KEYWORDS_EVENT = {
    // ### Unique ###
    // Only one of each of these events can be defined in a single KiSS Artform.
    { "initialize", { "initialize", FkissVersion::FKiSS1, false } },
    { "begin", { "begin", FkissVersion::FKiSS1, false } },
    { "end", { "end", FkissVersion::FKiSS1, false } },
    { "overflow", { "overflow", FkissVersion::FKiSS3, false } },
    { "never", { "never", FkissVersion::FKiSS1, false } },

    // ### Specific ###
    // These events are specific to sets, colors, and version.
    { "set", { "set", FkissVersion::FKiSS1, false } },
    { "col", { "col", FkissVersion::FKiSS1, false } },
    { "version", { "version", FkissVersion::FKiSS2, false } },

    // ### Named ###
    // These events are associated with named blocks of code.
    { "alarm", { "alarm", FkissVersion::FKiSS1, false } },
    { "label", { "label", FkissVersion::FKiSS1, false } },

    // ### Rectangle-based collision ###
    // These events are associated with collisions between rectangles.
    // Rectangle collision is faster than pixel-based collisions but not pixel accurate.
    { "in", { "in", FkissVersion::FKiSS2, false } },
    { "out", { "out", FkissVersion::FKiSS2, false } },

    { "stillin", { "stillin", FkissVersion::FKiSS2, false } },
    { "stillout", { "stillout", FkissVersion::FKiSS2, false } },

    // ### Pixel-based collision ###
    // These events are associated with collisions between pixels of cels.
    { "collide", { "collide", FkissVersion::FKiSS2, false } },
    { "apart", { "apart", FkissVersion::FKiSS2, false } },

    // ### Mouse-based collision ###
    // These events are associated with the movement of the mouse cursor.
    { "mousein", { "mousein", FkissVersion::Unknown, false } },
    { "mouseout", { "mouseout", FkissVersion::Unknown, false } },

    // ### Keyboard ###
    // These events are associated with pressing of keys.
    { "keypress", { "keypress", FkissVersion::Unknown, false } },
    { "keyrelease", { "keyrelease", FkissVersion::Unknown, false } },

    // ### Click-and-drag ###
    // These events are associated with clicking on cels as well as clicking and dragging cels around.
    { "press", { "press", FkissVersion::FKiSS1, false } },
    { "catch", { "catch", FkissVersion::FKiSS1, false } },   // catch(target) - The catch event is triggered when the user first begins to move the target.
    { "drop", { "drop", FkissVersion::FKiSS1, false } },   // drop(target) - The drop event is triggered when the user loses control of the target after attempting to move it.

    { "release", { "release", FkissVersion::FKiSS1, false } },
    { "fixcatch", { "fixcatch", FkissVersion::FKiSS1, false } },
    { "fixdrop", { "fixdrop", FkissVersion::FKiSS1, false } },

    // ### Fix ###
    // These events are associated with the fix value of an object.
    { "unfix", { "unfix", FkissVersion::FKiSS1, false } },

    // ### Attachment ###
    // This event is associated with detaching a target from a parent object.
    { "detached", { "detached", FkissVersion::Unknown, false }}
};

const QHash<QString, FkissKeywordInfo> Fkiss::KEYWORDS_ACTION = {
    // ### Alarms ###
    // These actions set up alarms, which are events triggered when certain conditions occur, such as a timer going off.
    { "timer", { "timer", FkissVersion::FKiSS1, false } },
    { "randomtimer", { "randomtimer", FkissVersion::FKiSS1, false } },
    { "lettimer", { "lettimer", FkissVersion::Unknown, false } },
    { "iffixed", { "iffixed", FkissVersion::FKiSS2, false } },
    { "ifmapped", { "ifmapped", FkissVersion::FKiSS2, false } },
    { "ifmoved", { "ifmoved", FkissVersion::FKiSS2, false } },
    { "ifnotfixed", { "ifnotfixed", FkissVersion::FKiSS2, false } },
    { "ifnotmapped", { "ifnotmapped", FkissVersion::FKiSS2, false } },
    { "ifnotmoved", { "ifnotmoved", FkissVersion::FKiSS2, false } },
    { "exitevent", { "exitevent", FkissVersion::Unknown, false } },

    // ### Labels ###
    // These actions use labels that mark a block of actions to execute that block of actions from several different places.
    { "gosub", { "gosub", FkissVersion::Unknown, false } },
    { "gosubrandom", { "gosubrandom", FkissVersion::Unknown, false } },
    { "goto", { "goto", FkissVersion::Unknown, false } },
    { "gotorandom", { "gotorandom", FkissVersion::Unknown, false } },
    { "repeat", { "repeat", FkissVersion::Unknown, false } },
    { "exitloop", { "exitloop", FkissVersion::Unknown, false } },

    // ### Mapping ###
    // These actions control the visibility of a cel. A mapped cel is visible while an unmapped cel is not visible.
    { "map", { "map", FkissVersion::FKiSS1, false } },
    { "unmap", { "unmap", FkissVersion::FKiSS1, false } },
    { "altmap", { "altmap", FkissVersion::FKiSS1, false } },
    { "letmapped", { "letmapped", FkissVersion::Unknown, false } },

    // ### Movement and Position ###
    // These actions control the position of a target and allow an action to move a cel in response to an event.
    { "move", { "move", FkissVersion::FKiSS1, false } },
    { "moveto", { "moveto", FkissVersion::FKiSS2, false } },
    { "movebyx", { "movebyx", FkissVersion::FKiSS2, false } },
    { "movebyy", { "movebyy", FkissVersion::FKiSS2, false } },
    { "movetorand", { "movetorand", FkissVersion::FKiSS2, false } },
    { "moverandx", { "moverandx", FkissVersion::FKiSS2, false } },
    { "moverandy", { "moverandy", FkissVersion::FKiSS2, false } },
    { "restrictx", { "restrictx", FkissVersion::Unknown, false } },
    { "restricty", { "restricty", FkissVersion::Unknown, false } },
    { "letinitx", { "letinitx", FkissVersion::Unknown, false } },
    { "letinity", { "letinity", FkissVersion::Unknown, false } },
    { "letobjectx", { "letobjectx", FkissVersion::Unknown, false } },
    { "letobjecty", { "letobjecty", FkissVersion::Unknown, false } },
    { "letcatch", { "letcatch", FkissVersion::Unknown, false } },

    // ### Message ###
    // These actions show messages to the user or trigger a shell command.
    { "debug", { "debug", FkissVersion::FKiSS1, false } },
    { "notify", { "notify", FkissVersion::FKiSS2, false } },
    { "shell", { "shell", FkissVersion::FKiSS1, false } },

    // ### Sound and Music ###
    // These actions control music and sound.
    { "music", { "music", FkissVersion::FKiSS2, false } },
    { "sound", { "sound", FkissVersion::FKiSS1, false } },

    // ### Translucency ###
    // These actions control the translucency of a target.
    { "transparent", { "transparent", FkissVersion::FKiSS1, false } },
    { "lettransparent", { "lettransparent", FkissVersion::Unknown, false } },

    // ### Attachment ###
    // These actions control the process of attaching or detaching an object to another object.
    { "attach", { "attach", FkissVersion::Unknown, false } },
    { "glue", { "glue", FkissVersion::Unknown, false } },
    { "detach", { "detach", FkissVersion::Unknown, false } },
    { "letchild", { "letchild", FkissVersion::Unknown, false } },
    { "letparent", { "letparent", FkissVersion::Unknown, false } },
    { "letsibling", { "letsibling", FkissVersion::Unknown, false } },

    // ### Group Frame ###
    // These actions change the frame that is displayed for a group of cels.
    { "setframe", { "setframe", FkissVersion::Unknown, false } },
    { "letframe", { "letframe", FkissVersion::Unknown, false } },

    // ### Arithmetic ###
    // These actions perform arithmetic operations on a variable.
    { "add", { "add", FkissVersion::Unknown, false } },
    { "sub", { "sub", FkissVersion::Unknown, false } },
    { "mul", { "mul", FkissVersion::Unknown, false } },
    { "div", { "div", FkissVersion::Unknown, false } },
    { "mod", { "mod", FkissVersion::Unknown, false } },
    { "random", { "random", FkissVersion::Unknown, false } },

    // ### Object/Cel State ###
    // These actions control or read the state of an object.
    { "setkcf", { "setkcf", FkissVersion::Unknown, false } },
    { "letkcf", { "letkcf", FkissVersion::Unknown, false } },
    { "setfix", { "setfix", FkissVersion::FKiSS2, false } },
    { "letfix", { "letfix", FkissVersion::Unknown, false } },
    { "ghost", { "ghost", FkissVersion::Unknown, false } },
    { "letwidth", { "letwidth", FkissVersion::Unknown, false } },
    { "letheight", { "letheight", FkissVersion::Unknown, false } },
    { "letcollide", { "letcollide", FkissVersion::Unknown, false } },
    { "letinside", { "letinside", FkissVersion::Unknown, false } },

    // ### Conditionals ###
    // These actions control the flow of execution through a block of actions.
    { "ifequal", { "ifequal", FkissVersion::Unknown, false } },
    { "ifnotequal", { "ifnotequal", FkissVersion::Unknown, false } },
    { "ifgreaterthan", { "ifgreaterthan", FkissVersion::Unknown, false } },
    { "iflessthan", { "iflessthan", FkissVersion::Unknown, false } },
    { "else", { "else", FkissVersion::Unknown, false } },
    { "elseifequal", { "elseifequal", FkissVersion::Unknown, false } },
    { "elseifnotequal", { "elseifnotequal", FkissVersion::Unknown, false } },
    { "elseifgreaterthan", { "elseifgreaterthan", FkissVersion::Unknown, false } },
    { "elseiflessthan", { "elseiflessthan", FkissVersion::Unknown, false } },
    { "endif", { "endif", FkissVersion::Unknown, false } },

    // ### Keyboard ###
    // These actions read the keyboard.
    { "letkey", { "letkey", FkissVersion::Unknown, false } },
    { "letkeymap", { "letkeymap", FkissVersion::Unknown, false } },

    // ### Mouse ###
    // These actions read the position of the mouse cursor.
    { "letmousex", { "letmousex", FkissVersion::Unknown, false } },
    { "letmousey", { "letmousey", FkissVersion::Unknown, false } },

    // ### Environment ###
    // These actions affect the environment, set, or play field.
    { "changecol", { "changecol", FkissVersion::FKiSS1, false } },
    { "changeset", { "changeset", FkissVersion::FKiSS1, false } },
    { "letpal", { "letpal", FkissVersion::Unknown, false } },
    { "letset", { "letset", FkissVersion::Unknown, false } },

    // ### Viewport ###
    // These actions control the size of the window and view on that window.
    { "windowsize", { "windowsize", FkissVersion::FKiSS1, false } },
    { "viewport", { "viewport", FkissVersion::FKiSS1, false } },

    // ### Miscellaneous ###
    // These actions fall in no particular category.
    { "nop", { "nop", FkissVersion::FKiSS1, false } },
    { "quit", { "quit", FkissVersion::FKiSS1, false } },

    // ### Pool Values ###
    // These actions control access to value pools.
    { "valuepool", { "valuepool", FkissVersion::Unknown, false } },
    { "savevalue", { "savevalue", FkissVersion::Unknown, false } },
    { "loadvalue", { "loadvalue", FkissVersion::Unknown, false } },
    { "deletevalue", { "deletevalue", FkissVersion::Unknown, false } },

    // ### Variables ###
    // These actions affect variables.
    { "let", { "let", FkissVersion::Unknown, false } }
};

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
    QVector<std::shared_ptr<ASTCommand>> ast0 = parse0(tokens);
    // foreach (ASTCommand astNode, ast0) {
    //     qDebug() << "[Fkiss]" << astNode;
    // }
    pairAstCommands(ast0);
}

bool Fkiss::commandIsEvent(const QString &command)
{
    return Fkiss::KEYWORDS_EVENT.contains(command);
}

bool Fkiss::commandIsAction(const QString &command)
{
    return Fkiss::KEYWORDS_ACTION.contains(command);
}

QVector<Token> Fkiss::tokenize(const QList<FkissLineInfo> &lineInfoList) {
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
                qCritical() << QStringLiteral("[ERROR][Fkiss::Tokenizer] Unexpected character [%1:%2]: %3").arg(lineInfo.lineNoInCnf).arg(c).arg(lineInfo.line);
                c++;
            }
        }
        tokens.append(Token(Token::Type::EndOfLine, "\n", oriLineNo, line.length()));
    }
    return tokens;
}

QVector<std::shared_ptr<ASTCommand>> Fkiss::parse0(const QVector<Token> &tokens)
{
    QVector<std::shared_ptr<ASTCommand>> astCmds;
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
                const Token *tk = argsTokensWithoutParensAndSpaces[j];
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
            astCmds.append(std::make_shared<ASTCommand>(ASTCommand{cmdName, cmdArgs}));
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

void Fkiss::pairAstCommands(QVector<std::shared_ptr<ASTCommand>> &commands)
{
    m_evpairs.clear();
    std::shared_ptr<ASTPairedCommands> curPair;
    foreach (std::shared_ptr<ASTCommand> cmd, commands) {
        if (commandIsEvent(cmd->name)) {
            m_evpairs.append(curPair);
            curPair = std::make_shared<ASTPairedCommands>();
            curPair->event = cmd;
        } else if (commandIsAction(cmd->name)) {
            curPair->actions.append(cmd);
        } else {
            qWarning() << QStringLiteral("[WARNING][Fkiss] Unknown command \"%1\", skip.").arg(cmd->name);
        }
    }
    if (m_evpairs.length() != 0) {
        m_evpairs.removeFirst();
    }
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

QDebug operator<<(QDebug debug, const ASTCommand& astCmd) {
    auto dbg = debug.noquote().nospace() << "[ASTCommand] " << astCmd.name << "( ";
    foreach (auto arg, astCmd.args) {
        dbg << arg;
    }
    return dbg << ")";
};

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
}
