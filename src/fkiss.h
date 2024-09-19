#ifndef FKISS_H
#define FKISS_H

#include <QObject>
#include <QList>
#include <QHash>
#include "fkissvariant.h"
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


// ================================================
// AST0 (flat, no nested structure (e.g. `alert() ...`, `ifeq() ...` ) )
// ================================================

using Variant = FkissVariant;

/** Without semantic of command */
struct ASTCommand {
    QString name;
    QList<Variant> args;
};

QDebug operator<<(QDebug debug, const ASTCommand& astCmd);

// ================================================
// Fkiss Engine
// ================================================

struct FkissLineInfo {
    size_t lineNoInCnf;
    QString line;
};

struct FkissEvent {
    QString name;
    QStringList args;
};

struct FkissAction {
    QString name;
    QStringList args;
};

class Fkiss : public QObject
{
    Q_OBJECT
public:
    explicit Fkiss(QObject *parent = nullptr);
    void compileScriptLines(const QList<FkissLineInfo> &lineInfoList);

private:
    /** source code -> tokens */
    QVector<Token> tokenize(const QList<FkissLineInfo> &lineInfoList);
    /** tokens -> AST (flat, no semantic, so no nested structure (e.g. `alert() ...`, `ifeq() ...` )  */
    QVector<ASTCommand> parse0(const QVector<Token> &tokens);
    /** Returns the index of nearest Token. If not found, returns -1. */
    int findNearestToken(QVector<Token> tokens, Token::Type type, int fromIndex);
    /** Remove `;@` prefix and `; comment` suffix. */
    QString removePrefixAndCommentOfLine(QString line);
    void logSyntaxError(const Token &token, QString msg);
signals:

private:
    QVector<FkissEvent*> m_events;
    QVector<FkissAction*> m_actions;
};




#endif // FKISS_H
