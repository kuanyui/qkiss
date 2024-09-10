#include "fkiss.h"
#include <QRegularExpression>


Fkiss::Fkiss(QObject *parent)
    : QObject{parent}
{

}

void Fkiss::parseLines(const QList<FkissLineInfo> &lineInfoList)
{
    QVector<Token> tokens = tokenize(lineInfoList);
    // qDebug() << "[Fkiss] parsed tokens count: " << tokens.length();
    // foreach (Token token, tokens) {
    //     qDebug() << "[Fkiss]" << token;
    // }
}

QVector<Token> Fkiss::tokenize(const QList<FkissLineInfo> &lineInfoList) {
    QVector<Token> tokens;
    for (int r = 0; r < lineInfoList.length(); ++r) {  // row index (line index)
        auto lineInfo = lineInfoList[r];
        QString line = removePrefixAndCommentOfLine(lineInfo.line);
        int c = 0;  // column index
        // bool inCmdArgs = false;   // TODO: Too lazy...
        while (c < line.length()) {
            if (line[c].isSpace()) {
                c++;
                continue;
            }
            if (line[c].isLetter()) {
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
        // tokens.append(Token(Token::Type::EndOfLine, "\n", r, line.length()));
    }
    return tokens;
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

QDebug operator<<(QDebug debug, const Token &token) {
    return debug.nospace() << "Token { type: " << token.type << ", value: " << token.value << " }";
}
QDebug operator<<(QDebug debug, const Token::Type &tokenType) {
    QString str;
    switch (tokenType) {
    case Token::Type::Identifier: str = QStringLiteral("Identifier"); break;
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
