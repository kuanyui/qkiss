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

struct ASTPairedCommands {
    std::shared_ptr<ASTCommand> event;
    QVector<std::shared_ptr<ASTCommand>> actions;
};


QDebug operator<<(QDebug debug, const ASTCommand& astCmd);

// ================================================
// Fkiss Engine
// ================================================

struct FkissLineInfo {
    size_t lineNoInCnf;
    QString line;
};

enum class FkissVersion {
    Unknown = -1,
    FKiSS1 = 0,
    FKiSS1b = 1,
    FKiSS2 = 2,
    FKiSS2_1 = 3,
    FKiSS3 = 4,
    FKiSS4 = 5,
};
struct FkissKeywordInfo {
    QString id;
    /**
     * Val	FKiSS Version
     * 0	FKiSS1
     * 1	FKiSS1b
     * 2	FKiSS2
     * 3	FKiSS2.1
     * 4	FKiSS3
     * 5	FKiSS4
     */
    FkissVersion fkissVersion;
    bool implemented;
    // QString specUrl;
};

class Fkiss : public QObject
{
    Q_OBJECT
public:
    explicit Fkiss(QObject *parent = nullptr);
    void compileScriptLines(const QList<FkissLineInfo> &lineInfoList);
    static bool commandIsEvent(const QString &command);
    static bool commandIsAction(const QString &command);
private:
    /** source code -> tokens */
    QVector<Token> tokenize(const QList<FkissLineInfo> &lineInfoList);
    /** tokens -> AST (flat, no semantic, so no nested structure (e.g. `alert() ...`, `ifeq() ...` )  */
    QVector<std::shared_ptr<ASTCommand> > parse0(const QVector<Token> &tokens);
    /** Returns the index of nearest Token. If not found, returns -1. */
    int findNearestToken(QVector<Token> tokens, Token::Type type, int fromIndex);
    /** Remove `;@` prefix and `; comment` suffix. */
    QString removePrefixAndCommentOfLine(QString line);
    void pairAstCommands(QVector<std::shared_ptr<ASTCommand> > &commands);
    void logSyntaxError(const Token &token, QString msg);
    QVector<std::shared_ptr<ASTPairedCommands>> m_evpairs;
    static const QHash<QString, FkissKeywordInfo> KEYWORDS_EVENT;
    static const QHash<QString, FkissKeywordInfo> KEYWORDS_ACTION;
signals:
};

/** The Fkiss event triggered by user interaction. Should be sent from QGraphicsItem. */
enum class FkissUserMovementEvent {
    /// The user clicks the mouse button on an object:
    Press,
    /// The user moves the mouse:
    Catch,
    Fixcatch,
    Unfix,
    /**
     * The user releases the mouse button:

    The object's original position (before dragging) is checked to see if it has collided with other objects or
    The object's new position (after dragging) is checked to see if it has collided with other objects

    And the following events are triggered:
    */
    Collide,
    Apart,
    In,
    Stillin,
    Out,
    Stillout,
    Release,
    Drop,
    Fixdrop
};



#endif // FKISS_H
