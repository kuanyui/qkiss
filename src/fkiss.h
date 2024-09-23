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

/// Fkiss Keywords
namespace FKK {
    const QString INITIALIZE        = QStringLiteral("initialize");
    const QString BEGIN             = QStringLiteral("begin");
    const QString END               = QStringLiteral("end");
    const QString OVERFLOW          = QStringLiteral("overflow");
    const QString NEVER             = QStringLiteral("never");
    const QString SET               = QStringLiteral("set");
    const QString COL               = QStringLiteral("col");
    const QString VERSION           = QStringLiteral("version");
    const QString ALARM             = QStringLiteral("alarm");
    const QString LABEL             = QStringLiteral("label");
    const QString IN                = QStringLiteral("in");
    const QString OUT               = QStringLiteral("out");
    const QString STILLIN           = QStringLiteral("stillin");
    const QString STILLOUT          = QStringLiteral("stillout");
    const QString COLLIDE           = QStringLiteral("collide");
    const QString APART             = QStringLiteral("apart");
    const QString MOUSEIN           = QStringLiteral("mousein");
    const QString MOUSEOUT          = QStringLiteral("mouseout");
    const QString KEYPRESS          = QStringLiteral("keypress");
    const QString KEYRELEASE        = QStringLiteral("keyrelease");
    const QString PRESS             = QStringLiteral("press");
    const QString CATCH             = QStringLiteral("catch");
    const QString DROP              = QStringLiteral("drop");
    const QString RELEASE           = QStringLiteral("release");
    const QString FIXCATCH          = QStringLiteral("fixcatch");
    const QString FIXDROP           = QStringLiteral("fixdrop");
    const QString UNFIX             = QStringLiteral("unfix");
    const QString DETACHED          = QStringLiteral("detached");
    const QString TIMER             = QStringLiteral("timer");
    const QString RANDOMTIMER       = QStringLiteral("randomtimer");
    const QString LETTIMER          = QStringLiteral("lettimer");
    const QString IFFIXED           = QStringLiteral("iffixed");
    const QString IFMAPPED          = QStringLiteral("ifmapped");
    const QString IFMOVED           = QStringLiteral("ifmoved");
    const QString IFNOTFIXED        = QStringLiteral("ifnotfixed");
    const QString IFNOTMAPPED       = QStringLiteral("ifnotmapped");
    const QString IFNOTMOVED        = QStringLiteral("ifnotmoved");
    const QString EXITEVENT         = QStringLiteral("exitevent");
    const QString GOSUB             = QStringLiteral("gosub");
    const QString GOSUBRANDOM       = QStringLiteral("gosubrandom");
    const QString GOTO              = QStringLiteral("goto");
    const QString GOTORANDOM        = QStringLiteral("gotorandom");
    const QString REPEAT            = QStringLiteral("repeat");
    const QString EXITLOOP          = QStringLiteral("exitloop");
    const QString MAP               = QStringLiteral("map");
    const QString UNMAP             = QStringLiteral("unmap");
    const QString ALTMAP            = QStringLiteral("altmap");
    const QString LETMAPPED         = QStringLiteral("letmapped");
    const QString MOVE              = QStringLiteral("move");
    const QString MOVETO            = QStringLiteral("moveto");
    const QString MOVEBYX           = QStringLiteral("movebyx");
    const QString MOVEBYY           = QStringLiteral("movebyy");
    const QString MOVETORAND        = QStringLiteral("movetorand");
    const QString MOVERANDX         = QStringLiteral("moverandx");
    const QString MOVERANDY         = QStringLiteral("moverandy");
    const QString RESTRICTX         = QStringLiteral("restrictx");
    const QString RESTRICTY         = QStringLiteral("restricty");
    const QString LETINITX          = QStringLiteral("letinitx");
    const QString LETINITY          = QStringLiteral("letinity");
    const QString LETOBJECTX        = QStringLiteral("letobjectx");
    const QString LETOBJECTY        = QStringLiteral("letobjecty");
    const QString LETCATCH          = QStringLiteral("letcatch");
    const QString DEBUG             = QStringLiteral("debug");
    const QString NOTIFY            = QStringLiteral("notify");
    const QString SHELL             = QStringLiteral("shell");
    const QString MUSIC             = QStringLiteral("music");
    const QString SOUND             = QStringLiteral("sound");
    const QString TRANSPARENT       = QStringLiteral("transparent");
    const QString LETTRANSPARENT    = QStringLiteral("lettransparent");
    const QString ATTACH            = QStringLiteral("attach");
    const QString GLUE              = QStringLiteral("glue");
    const QString DETACH            = QStringLiteral("detach");
    const QString LETCHILD          = QStringLiteral("letchild");
    const QString LETPARENT         = QStringLiteral("letparent");
    const QString LETSIBLING        = QStringLiteral("letsibling");
    const QString SETFRAME          = QStringLiteral("setframe");
    const QString LETFRAME          = QStringLiteral("letframe");
    const QString ADD               = QStringLiteral("add");
    const QString SUB               = QStringLiteral("sub");
    const QString MUL               = QStringLiteral("mul");
    const QString DIV               = QStringLiteral("div");
    const QString MOD               = QStringLiteral("mod");
    const QString RANDOM            = QStringLiteral("random");
    const QString SETKCF            = QStringLiteral("setkcf");
    const QString LETKCF            = QStringLiteral("letkcf");
    const QString SETFIX            = QStringLiteral("setfix");
    const QString LETFIX            = QStringLiteral("letfix");
    const QString GHOST             = QStringLiteral("ghost");
    const QString LETWIDTH          = QStringLiteral("letwidth");
    const QString LETHEIGHT         = QStringLiteral("letheight");
    const QString LETCOLLIDE        = QStringLiteral("letcollide");
    const QString LETINSIDE         = QStringLiteral("letinside");
    const QString IFEQUAL           = QStringLiteral("ifequal");
    const QString IFNOTEQUAL        = QStringLiteral("ifnotequal");
    const QString IFGREATERTHAN     = QStringLiteral("ifgreaterthan");
    const QString IFLESSTHAN        = QStringLiteral("iflessthan");
    const QString ELSE              = QStringLiteral("else");
    const QString ELSEIFEQUAL       = QStringLiteral("elseifequal");
    const QString ELSEIFNOTEQUAL    = QStringLiteral("elseifnotequal");
    const QString ELSEIFGREATERTHAN = QStringLiteral("elseifgreaterthan");
    const QString ELSEIFLESSTHAN    = QStringLiteral("elseiflessthan");
    const QString ENDIF             = QStringLiteral("endif");
    const QString LETKEY            = QStringLiteral("letkey");
    const QString LETKEYMAP         = QStringLiteral("letkeymap");
    const QString LETMOUSEX         = QStringLiteral("letmousex");
    const QString LETMOUSEY         = QStringLiteral("letmousey");
    const QString CHANGECOL         = QStringLiteral("changecol");
    const QString CHANGESET         = QStringLiteral("changeset");
    const QString LETPAL            = QStringLiteral("letpal");
    const QString LETSET            = QStringLiteral("letset");
    const QString WINDOWSIZE        = QStringLiteral("windowsize");
    const QString VIEWPORT          = QStringLiteral("viewport");
    const QString NOP               = QStringLiteral("nop");
    const QString QUIT              = QStringLiteral("quit");
    const QString VALUEPOOL         = QStringLiteral("valuepool");
    const QString SAVEVALUE         = QStringLiteral("savevalue");
    const QString LOADVALUE         = QStringLiteral("loadvalue");
    const QString DELETEVALUE       = QStringLiteral("deletevalue");
    const QString LET               = QStringLiteral("let");
}

#endif // FKISS_H
