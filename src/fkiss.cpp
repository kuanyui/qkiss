#include "fkiss.h"
#include "gkissitem.h"
#include "gkissscene.h"
#include <QRegularExpression>
#include <QHashIterator>
using namespace Qt::Literals::StringLiterals;

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
    FkissProcedure ast0 = parse0(tokens);
    // foreach (ASTCommand astNode, ast0) {
    //     qDebug() << "[Fkiss]" << astNode;
    // }
    pairAstCommands(ast0);
    tryToRunActionsByEvent(FKK::INITIALIZE, {});
}

bool Fkiss::commandIsEvent(const QString &command)
{
    return Fkiss::KEYWORDS_EVENT.contains(command);
}

bool Fkiss::commandIsAction(const QString &command)
{
    return Fkiss::KEYWORDS_ACTION.contains(command);
}

void Fkiss::receiveUserEventFromGraphicsItem(FkissUserMovementEvent evtType, QVector<FkissVariant> vars)
{
    // All events are generated from GKissItem (instead of FKiSS script), so the items counts should not need to be checked here.
    switch (evtType) {
    case FkissUserMovementEvent::Press:    tryToRunActionsByEvent(FKK::PRESS, vars); break;
    case FkissUserMovementEvent::Catch:    tryToRunActionsByEvent(FKK::CATCH, vars); break;
    case FkissUserMovementEvent::Fixcatch: tryToRunActionsByEvent(FKK::FIXCATCH, vars); break;
    case FkissUserMovementEvent::Unfix:    tryToRunActionsByEvent(FKK::UNFIX, vars); break;
    case FkissUserMovementEvent::Collide:  tryToRunActionsByEvent(FKK::COLLIDE, vars); break;
    case FkissUserMovementEvent::Apart:    tryToRunActionsByEvent(FKK::APART, vars); break;
    case FkissUserMovementEvent::In:       tryToRunActionsByEvent(FKK::IN, vars); break;
    case FkissUserMovementEvent::Stillin:  tryToRunActionsByEvent(FKK::STILLIN, vars); break;
    case FkissUserMovementEvent::Out:      tryToRunActionsByEvent(FKK::OUT, vars); break;
    case FkissUserMovementEvent::Stillout: tryToRunActionsByEvent(FKK::STILLOUT, vars); break;
    case FkissUserMovementEvent::Release:  tryToRunActionsByEvent(FKK::RELEASE, vars); break;
    case FkissUserMovementEvent::Drop:     tryToRunActionsByEvent(FKK::DROP, vars); break;
    case FkissUserMovementEvent::Fixdrop:  tryToRunActionsByEvent(FKK::FIXDROP, vars); break;
    }
    return;
}

void Fkiss::runMap(FkissVariant target)
{
    m_rtMapState.insert(target, true);
    if (m_curKissScene) {
        m_curKissScene->setItemsVisibility(target, true);
    }
}

void Fkiss::runUnmap(FkissVariant target)
{
    m_rtMapState.insert(target, false);
    if (m_curKissScene) {
        m_curKissScene->setItemsVisibility(target, false);
    }
}

void Fkiss::runAltmap(FkissVariant target)
{
    bool oldVal = getMap(target);
    bool newVal = !oldVal;
    m_rtMapState.insert(target, newVal);
    if (m_curKissScene) {
        m_curKissScene->setItemsVisibility(target, newVal);
    }
}

bool Fkiss::getMap(FkissVariant target)
{
    return m_rtMapState.value(target, true);
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

FkissProcedure Fkiss::parse0(const QVector<Token> &tokens)
{
    FkissProcedure astCmds;
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

void Fkiss::pairAstCommands(FkissProcedure &commands)
{
    m_evpairs.clear();
    std::shared_ptr<ASTPairedCommands> curPair = std::make_shared<ASTPairedCommands>();  // create sentinel
    m_evpairs.append(curPair);   // add sentinel
    foreach (std::shared_ptr<ASTCommand> cmd, commands) {
        if (commandIsEvent(cmd->name)) {
            curPair = std::make_shared<ASTPairedCommands>();
            curPair->event = cmd;
            m_evpairs.append(curPair);
        } else if (commandIsAction(cmd->name)) {
            curPair->actions.append(cmd);
        } else {
            qWarning() << QStringLiteral("[WARNING][Fkiss] Unknown command \"%1\", skip.").arg(cmd->name);
        }
    }
    m_evpairs.removeFirst();  // remove sentinel
}

void Fkiss::logSyntaxError(const Token &token, QString msg)
{
    qDebug() << QStringLiteral("[FKiSS][SyntaxError] %1   [L%2, C%3] %4")
                .arg(msg)
                .arg(token.lineNo())
                .arg(token.columnNo())
                .arg(token.value);
}

std::shared_ptr<ASTPairedCommands> Fkiss::getEventActionPair(const QString &eventName, const QList<FkissVariant> &args)
{
    foreach (std::shared_ptr<ASTPairedCommands> pair, m_evpairs) {
        if (pair->event->name == eventName) {
            if (pair->event->args == args) {
                return pair;
            }
        }
    }
    return nullptr;
}

bool Fkiss::tryToRunActionsByEvent(const QString &eventName, const QList<FkissVariant> &eventArgs)
{
    qDebug() << "[Fkiss] tryToRunActionsByEvent" << eventName << eventArgs;
    auto _evpair = findEvpair(eventName, eventArgs);
    if (!_evpair.has_value()) { return false; }
    auto evpair = _evpair.value();
    foreach (auto act, evpair->actions) {
        qDebug() << "[Fkiss]   tryToRunActionsByEvent(): execute action" << act->name << act->args;
        auto actArgs = act->args;
        if (act->name == FKK::MAP) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid map() args count, skip."_L1; continue; }
            runMap(actArgs[0]);
        } else if (act->name == FKK::UNMAP) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid unmap() args count, skip."_L1; continue; }
            runUnmap(act->args[0]);
        } else if (act->name == FKK::ALTMAP) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid altmap() args count, skip."_L1; continue; }
            runAltmap(act->args[0]);
        }
    }
    return true;
}

std::optional<std::shared_ptr<ASTPairedCommands>> Fkiss::findEvpair(const QString &eventName, const QList<FkissVariant> &args)
{
    static QStringList EVENTS_WITH_ORDER_INSENSITIVE_ARGS {
        FKK::COLLIDE,
        FKK::APART,
        FKK::IN,
        FKK::STILLIN,
        FKK::OUT,
        FKK::STILLOUT
    };
    bool isOrderInsensitiveArgs = EVENTS_WITH_ORDER_INSENSITIVE_ARGS.contains(eventName);
    // TODO: some events' args are order-insensitive. e.g. collide(), in()
    foreach (auto evpair, m_evpairs) {
        if (evpair->event->name != eventName) { continue; }
        if (evpair->event->args == args) {
            return evpair;
        }
        // TODO: optimize this shit, please.
        if (isOrderInsensitiveArgs && evpair->event->args.length() == args.length()) {
            QSet<FkissVariant> s1 { evpair->event->args.begin(), evpair->event->args.end() };
            QSet<FkissVariant> s2 { args.begin(), args.end() };
            if (s1 == s2) {
                return evpair;
            }
        }
    }
    return std::nullopt;
}



GKissScene *Fkiss::curKissScene() const
{
    return m_curKissScene;
}

void Fkiss::setCurKissScene(GKissScene *newCurKissScene)
{
    m_curKissScene = newCurKissScene;
    if (m_curKissScene == nullptr) { return; }
    QHashIterator<FkissVariant, bool> it(m_rtMapState);
    while (it.hasNext()) {
        it.next();
        const FkissVariant &key = it.key();
        bool visible = it.value();
        if (key.type() == FkissVariant::Type::String) {
            auto _item = m_curKissScene->getItemByCelName(key.getString());
            if (!_item.has_value()) { qDebug() << "WTF???"; continue; }
            _item.value()->setVisible(visible);
        } else if (key.type() == FkissVariant::Type::Object) {
            std::optional<QVector<GKissItem *> *> _items = m_curKissScene->getItemsByObjId(key.getObjectId());
            if (!_items.has_value()) { qDebug() << "WTF???"; continue; }
            auto *items = _items.value();
            foreach (auto *item, *items) {
                item->setVisible(visible);
            }
        }
    }
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
