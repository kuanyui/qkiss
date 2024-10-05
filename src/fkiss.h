#ifndef FKISS_H
#define FKISS_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QTimer>
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

/** Without semantic of command */
struct ASTCommand {
    QString name;
    QList<FkissVariant> args;
};
using FkissProcedure = QVector<std::shared_ptr<ASTCommand>>;

/// ASTPairedCommands
struct ASTPairedCommands {
    std::shared_ptr<ASTCommand> event;
    FkissProcedure actions;
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

/**
 * The Fkiss event triggered by user interaction. Should be sent from QGraphicsItem.
 * The events are triggered order-sensitive. Please see the docstrings of each field.
 */
enum class FkissUserMovementEvent {

    // ===============================================================
    // The user clicks the mouse button on an object:
    // ===============================================================

    Press,

    // ===============================================================
    // The user moves the mouse:
    // ===============================================================

    /// 1. catch(target) event is triggered
    Catch,
    /// 2. fixcatch(target) event is triggered, if fix value of object is greater than 1
    Fixcatch,
    /// 3. unfix(target) event is triggered, if fix value of object is equal to 1
    Unfix,
    /// (4. If the fix value of the object is greater than 0 but less than the max fix value, the fix value is reduced by 1.)


    // ===============================================================
    // The user **releases** the mouse button:
    //
    // The object's original position (before dragging) is checked to see if it has collided with other objects or
    // The object's new position (after dragging) is checked to see if it has collided with other objects
    //
    // And the following events are triggered:
    // ===============================================================

    /// collide(target1, target2) - (Pixel) Triggered when pixels of target1 overlap pixels of target2 and previously did not.
    Collide,
    /// apart(target1, target2) - (Pixel) Triggered when pixels of target1 no longer overlap pixels of target2 and previously did.
    Apart,
    /// TODO: in(target1, target2) - (Bounding-rect) Triggered when target1 overlaps target2 and previously did not.
    In,
    /// TODO: stillin(target1, target2) - (Bounding-rect) Triggered when target1 still overlaps target2, regardless of previous state.
    Stillin,
    /// TODO: out(target1, target2) - (Bounding-rect) Triggered when target1 no longer overlaps target2 and previously did.
    Out,
    /// stillout(target1, target2) - (Bounding-rect) Triggered when target1 still does not overlap target2, regardless of previous state.
    Stillout,
    /// release(target)
    Release,
    /// drop(target) event is triggered, if fix value of object is equal to 0
    Drop,
    /// fixdrop(target) event is triggered, if fix value of object is greater than 0
    Fixdrop
};

/// Event:  alarm(alarmId)
/// Action: timer(alarmId, delayMs) - Sets the timer for an alarm
struct FkissAlarmTimer {
    /// Primary key.
    /// If a timer for that alarm is already counting down, the new delay time will replace it, resetting the alarm's timer.
    int alarmId;
    std::unique_ptr<QTimer> qTimer;
    std::shared_ptr<ASTPairedCommands> triggedTimer;
};

class GKissItem;
class GKissScene;

class Fkiss : public QObject
{
    Q_OBJECT
public:
    explicit Fkiss(QObject *parent = nullptr);
    void compileScriptLines(const QList<FkissLineInfo> &lineInfoList);
    static bool commandIsEvent(const QString &command);
    static bool commandIsAction(const QString &command);
    void receiveUserEventFromGraphicsItem(FkissUserMovementEvent evtType, QVector<FkissVariant> vars);
    /// run map(), unmap(), altmap()
    void runMap(FkissVariant target);
    void runUnmap(FkissVariant target);
    void runAltmap(FkissVariant target);
    /// for map(), unmap(), altmap()
    bool getMap(FkissVariant target);
    GKissScene *curKissScene() const;
    void setCurKissScene(GKissScene *newCurKissScene);

private:
    /** source code -> tokens */
    QVector<Token> tokenize(const QList<FkissLineInfo> &lineInfoList);
    /** tokens -> AST (flat, no semantic, so no nested structure (e.g. `alert() ...`, `ifeq() ...` )  */
    QVector<std::shared_ptr<ASTCommand> > parse0(const QVector<Token> &tokens);
    /** Returns the index of nearest Token. If not found, returns -1. */
    int findNearestToken(QVector<Token> tokens, Token::Type type, int fromIndex);
    /** Remove `;@` prefix and `; comment` suffix. */
    QString removePrefixAndCommentOfLine(QString line);
    /// Traverse across all commands, bind each event + its action(s) (procedure) pair, and store to pairs list.
    void pairAstCommands(QVector<std::shared_ptr<ASTCommand> > &commands);
    void logSyntaxError(const Token &token, QString msg);
    std::shared_ptr<ASTPairedCommands> getEventActionPair(const QString &eventName, const QList<FkissVariant> &args);
    bool tryToRunActionsByEvent(const QString &eventName, const QList<FkissVariant> &eventArgs);
    /// ATTENTION: may return nullptr.   // FIXME: Use std::optional?
    std::optional<std::shared_ptr<ASTPairedCommands>> findEvpair(const QString &eventName, const QList<FkissVariant> &args);
    QVector<std::shared_ptr<ASTPairedCommands>> m_evpairs;
    static const QHash<QString, FkissKeywordInfo> KEYWORDS_EVENT;
    static const QHash<QString, FkissKeywordInfo> KEYWORDS_ACTION;
    /// Currently active scene, all operations and manipulations will be applied on this scene.
    /// @borrowed
    GKissScene *m_curKissScene = nullptr;
    /// Key is alarmId. Runtime.
    QHash<int, std::shared_ptr<FkissAlarmTimer>> m_rtAlarmTimerPool;
    /// Store the visibility modified via FKiSS script (e.g. map(), unmap()). May be a cel or an obj.
    QHash<FkissVariant, bool> m_rtMapState;
signals:
    void fkissRunProcedure(const FkissProcedure &fkissProcedure);
};


/// Fkiss Keywords
namespace FKK {
    inline const QString INITIALIZE        = QStringLiteral("initialize");
    inline const QString BEGIN             = QStringLiteral("begin");
    inline const QString END               = QStringLiteral("end");
    inline const QString OVERFLOW          = QStringLiteral("overflow");
    inline const QString NEVER             = QStringLiteral("never");
    inline const QString SET               = QStringLiteral("set");
    inline const QString COL               = QStringLiteral("col");
    inline const QString VERSION           = QStringLiteral("version");
    inline const QString ALARM             = QStringLiteral("alarm");
    inline const QString LABEL             = QStringLiteral("label");
    inline const QString IN                = QStringLiteral("in");
    inline const QString OUT               = QStringLiteral("out");
    inline const QString STILLIN           = QStringLiteral("stillin");
    inline const QString STILLOUT          = QStringLiteral("stillout");
    inline const QString COLLIDE           = QStringLiteral("collide");
    inline const QString APART             = QStringLiteral("apart");
    inline const QString MOUSEIN           = QStringLiteral("mousein");
    inline const QString MOUSEOUT          = QStringLiteral("mouseout");
    inline const QString KEYPRESS          = QStringLiteral("keypress");
    inline const QString KEYRELEASE        = QStringLiteral("keyrelease");
    inline const QString PRESS             = QStringLiteral("press");
    inline const QString CATCH             = QStringLiteral("catch");
    inline const QString DROP              = QStringLiteral("drop");
    inline const QString RELEASE           = QStringLiteral("release");
    inline const QString FIXCATCH          = QStringLiteral("fixcatch");
    inline const QString FIXDROP           = QStringLiteral("fixdrop");
    inline const QString UNFIX             = QStringLiteral("unfix");
    inline const QString DETACHED          = QStringLiteral("detached");
    inline const QString TIMER             = QStringLiteral("timer");
    inline const QString RANDOMTIMER       = QStringLiteral("randomtimer");
    inline const QString LETTIMER          = QStringLiteral("lettimer");
    inline const QString IFFIXED           = QStringLiteral("iffixed");
    inline const QString IFMAPPED          = QStringLiteral("ifmapped");
    inline const QString IFMOVED           = QStringLiteral("ifmoved");
    inline const QString IFNOTFIXED        = QStringLiteral("ifnotfixed");
    inline const QString IFNOTMAPPED       = QStringLiteral("ifnotmapped");
    inline const QString IFNOTMOVED        = QStringLiteral("ifnotmoved");
    inline const QString EXITEVENT         = QStringLiteral("exitevent");
    inline const QString GOSUB             = QStringLiteral("gosub");
    inline const QString GOSUBRANDOM       = QStringLiteral("gosubrandom");
    inline const QString GOTO              = QStringLiteral("goto");
    inline const QString GOTORANDOM        = QStringLiteral("gotorandom");
    inline const QString REPEAT            = QStringLiteral("repeat");
    inline const QString EXITLOOP          = QStringLiteral("exitloop");
    inline const QString MAP               = QStringLiteral("map");
    inline const QString UNMAP             = QStringLiteral("unmap");
    inline const QString ALTMAP            = QStringLiteral("altmap");
    inline const QString LETMAPPED         = QStringLiteral("letmapped");
    inline const QString MOVE              = QStringLiteral("move");
    inline const QString MOVETO            = QStringLiteral("moveto");
    inline const QString MOVEBYX           = QStringLiteral("movebyx");
    inline const QString MOVEBYY           = QStringLiteral("movebyy");
    inline const QString MOVETORAND        = QStringLiteral("movetorand");
    inline const QString MOVERANDX         = QStringLiteral("moverandx");
    inline const QString MOVERANDY         = QStringLiteral("moverandy");
    inline const QString RESTRICTX         = QStringLiteral("restrictx");
    inline const QString RESTRICTY         = QStringLiteral("restricty");
    inline const QString LETINITX          = QStringLiteral("letinitx");
    inline const QString LETINITY          = QStringLiteral("letinity");
    inline const QString LETOBJECTX        = QStringLiteral("letobjectx");
    inline const QString LETOBJECTY        = QStringLiteral("letobjecty");
    inline const QString LETCATCH          = QStringLiteral("letcatch");
    inline const QString DEBUG             = QStringLiteral("debug");
    inline const QString NOTIFY            = QStringLiteral("notify");
    inline const QString SHELL             = QStringLiteral("shell");
    inline const QString MUSIC             = QStringLiteral("music");
    inline const QString SOUND             = QStringLiteral("sound");
    inline const QString TRANSPARENT       = QStringLiteral("transparent");
    inline const QString LETTRANSPARENT    = QStringLiteral("lettransparent");
    inline const QString ATTACH            = QStringLiteral("attach");
    inline const QString GLUE              = QStringLiteral("glue");
    inline const QString DETACH            = QStringLiteral("detach");
    inline const QString LETCHILD          = QStringLiteral("letchild");
    inline const QString LETPARENT         = QStringLiteral("letparent");
    inline const QString LETSIBLING        = QStringLiteral("letsibling");
    inline const QString SETFRAME          = QStringLiteral("setframe");
    inline const QString LETFRAME          = QStringLiteral("letframe");
    inline const QString ADD               = QStringLiteral("add");
    inline const QString SUB               = QStringLiteral("sub");
    inline const QString MUL               = QStringLiteral("mul");
    inline const QString DIV               = QStringLiteral("div");
    inline const QString MOD               = QStringLiteral("mod");
    inline const QString RANDOM            = QStringLiteral("random");
    inline const QString SETKCF            = QStringLiteral("setkcf");
    inline const QString LETKCF            = QStringLiteral("letkcf");
    inline const QString SETFIX            = QStringLiteral("setfix");
    inline const QString LETFIX            = QStringLiteral("letfix");
    inline const QString GHOST             = QStringLiteral("ghost");
    inline const QString LETWIDTH          = QStringLiteral("letwidth");
    inline const QString LETHEIGHT         = QStringLiteral("letheight");
    inline const QString LETCOLLIDE        = QStringLiteral("letcollide");
    inline const QString LETINSIDE         = QStringLiteral("letinside");
    inline const QString IFEQUAL           = QStringLiteral("ifequal");
    inline const QString IFNOTEQUAL        = QStringLiteral("ifnotequal");
    inline const QString IFGREATERTHAN     = QStringLiteral("ifgreaterthan");
    inline const QString IFLESSTHAN        = QStringLiteral("iflessthan");
    inline const QString ELSE              = QStringLiteral("else");
    inline const QString ELSEIFEQUAL       = QStringLiteral("elseifequal");
    inline const QString ELSEIFNOTEQUAL    = QStringLiteral("elseifnotequal");
    inline const QString ELSEIFGREATERTHAN = QStringLiteral("elseifgreaterthan");
    inline const QString ELSEIFLESSTHAN    = QStringLiteral("elseiflessthan");
    inline const QString ENDIF             = QStringLiteral("endif");
    inline const QString LETKEY            = QStringLiteral("letkey");
    inline const QString LETKEYMAP         = QStringLiteral("letkeymap");
    inline const QString LETMOUSEX         = QStringLiteral("letmousex");
    inline const QString LETMOUSEY         = QStringLiteral("letmousey");
    inline const QString CHANGECOL         = QStringLiteral("changecol");
    inline const QString CHANGESET         = QStringLiteral("changeset");
    inline const QString LETPAL            = QStringLiteral("letpal");
    inline const QString LETSET            = QStringLiteral("letset");
    inline const QString WINDOWSIZE        = QStringLiteral("windowsize");
    inline const QString VIEWPORT          = QStringLiteral("viewport");
    inline const QString NOP               = QStringLiteral("nop");
    inline const QString QUIT              = QStringLiteral("quit");
    inline const QString VALUEPOOL         = QStringLiteral("valuepool");
    inline const QString SAVEVALUE         = QStringLiteral("savevalue");
    inline const QString LOADVALUE         = QStringLiteral("loadvalue");
    inline const QString DELETEVALUE       = QStringLiteral("deletevalue");
    inline const QString LET               = QStringLiteral("let");
}

#endif // FKISS_H
