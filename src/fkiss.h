#ifndef FKISS_H
#define FKISS_H

#include <QObject>
#include <QList>
#include <QHash>

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
    int lineNo() const { return lineIdx + 1; }
    int columnNo() const { return columnIdx + 1; }
    Type type;
    QString value;
    int lineIdx;
    int columnIdx;
};
QDebug operator<<(QDebug debug, const Token& token);
QDebug operator<<(QDebug debug, const Token::Type& tokenType);


class Fkiss : public QObject
{
    Q_OBJECT
public:
    explicit Fkiss(QObject *parent = nullptr);
    void compileScriptLines(const QList<FkissLineInfo> &lineInfoList);

private:
    /** source code -> tokens */
    QVector<Token> tokenize(const QList<FkissLineInfo> &lineInfoList);
    /** tokens -> AST */
    void parse(const QVector<Token> &tokens);
    /** Remove `;@` prefix and `; comment` suffix. */
    QString removePrefixAndCommentOfLine(QString line);
signals:

private:
    QVector<FkissEvent*> m_events;
    QVector<FkissAction*> m_actions;
};




#endif // FKISS_H
