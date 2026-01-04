/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2024 - 2025 ono ono (kuanyui) <azazabc123@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef FKISS_H
#define FKISS_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QTimer>
#include "fkissvariant.h"


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

enum class FkissEventSource {
    /// interactively triggered directly by user
    User,
    /// programmatically triggered by FKiSS script.
    Script
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

/**
* [Event]    alarm(alarmId)
* [Action]   timer(alarmId, delayMs) - Sets the timer for an alarm
* @example
* ```
* initialize()
*   timer(5, 1000)  ; call alarm(5) after 1000ms
*
* alarm(5)   ; event
*   sound("bells.wav")
*
* ```
*/
struct FkissAlarmTimer {
    /// Primary key.
    /// If a timer for that alarm is already counting down, the new delay time will replace it, resetting the alarm's timer.
    int alarmId;
    std::unique_ptr<QTimer> qTimer;
    /// The event of this pair must be an alarm()
    /// std::shared_ptr<ASTPairedCommands> triggeredAlarm;
};

class GKissItem;
class GKissScene;
class FkissParser;
class FkissRuntime;

class Fkiss : public QObject
{
    Q_OBJECT
public:
    explicit Fkiss(QObject *parent = nullptr);
    ~Fkiss();
    static const QHash<QString, FkissKeywordInfo> KEYWORDS_EVENT;
    static const QHash<QString, FkissKeywordInfo> KEYWORDS_ACTION;
    void parseAndCacheFkissScript(QList<FkissLineInfo> fkissScriptLines);
    void startRuntime(GKissScene *kissScene);
    void stopRuntime();
    FkissRuntime *runtime();

    FkissParser *parser();

signals:
    void fkissRunProcedure(const FkissProcedure &fkissProcedure);
private:
    FkissParser *m_parser;
    FkissRuntime *m_runtime;
    QVector<std::shared_ptr<ASTPairedCommands>> m_parsedScriptCache;
};

/// Fkiss Keywords
/// NOTE: Do not use UPPERCASE as symbol name here, because they may be #define & expanded by MSVC.
/// Instead, use Capitalize symbol name.
namespace FKK
{
    /// Events
    namespace E {
        inline const QString Initialize        = QStringLiteral("initialize");
        inline const QString Begin             = QStringLiteral("begin");
        inline const QString End               = QStringLiteral("end");
        inline const QString Overflow          = QStringLiteral("overflow");
        inline const QString Never             = QStringLiteral("never");
        inline const QString Set               = QStringLiteral("set");
        inline const QString Col               = QStringLiteral("col");
        inline const QString Version           = QStringLiteral("version");
        inline const QString Alarm             = QStringLiteral("alarm");
        inline const QString Label             = QStringLiteral("label");
        inline const QString In                = QStringLiteral("in");
        inline const QString Out               = QStringLiteral("out");
        inline const QString Stillin           = QStringLiteral("stillin");
        inline const QString Stillout          = QStringLiteral("stillout");
        inline const QString Collide           = QStringLiteral("collide");
        inline const QString Apart             = QStringLiteral("apart");
        inline const QString Mousein           = QStringLiteral("mousein");
        inline const QString Mouseout          = QStringLiteral("mouseout");
        inline const QString Keypress          = QStringLiteral("keypress");
        inline const QString Keyrelease        = QStringLiteral("keyrelease");
        inline const QString Press             = QStringLiteral("press");
        inline const QString Catch             = QStringLiteral("catch");
        inline const QString Drop              = QStringLiteral("drop");
        inline const QString Release           = QStringLiteral("release");
        inline const QString Fixcatch          = QStringLiteral("fixcatch");
        inline const QString Fixdrop           = QStringLiteral("fixdrop");
        inline const QString Unfix             = QStringLiteral("unfix");
        inline const QString Detached          = QStringLiteral("detached");
    }
    /// Actions
    namespace A {
        inline const QString Timer             = QStringLiteral("timer");
        inline const QString Randomtimer       = QStringLiteral("randomtimer");
        inline const QString Lettimer          = QStringLiteral("lettimer");
        inline const QString Iffixed           = QStringLiteral("iffixed");
        inline const QString Ifmapped          = QStringLiteral("ifmapped");
        inline const QString Ifmoved           = QStringLiteral("ifmoved");
        inline const QString Ifnotfixed        = QStringLiteral("ifnotfixed");
        inline const QString Ifnotmapped       = QStringLiteral("ifnotmapped");
        inline const QString Ifnotmoved        = QStringLiteral("ifnotmoved");
        inline const QString Exitevent         = QStringLiteral("exitevent");
        inline const QString Gosub             = QStringLiteral("gosub");
        inline const QString Gosubrandom       = QStringLiteral("gosubrandom");
        inline const QString Goto              = QStringLiteral("goto");
        inline const QString Gotorandom        = QStringLiteral("gotorandom");
        inline const QString Repeat            = QStringLiteral("repeat");
        inline const QString Exitloop          = QStringLiteral("exitloop");
        inline const QString Map               = QStringLiteral("map");
        inline const QString Unmap             = QStringLiteral("unmap");
        inline const QString Altmap            = QStringLiteral("altmap");
        inline const QString Letmapped         = QStringLiteral("letmapped");
        inline const QString Move              = QStringLiteral("move");
        inline const QString Moveto            = QStringLiteral("moveto");
        inline const QString Movebyx           = QStringLiteral("movebyx");
        inline const QString Movebyy           = QStringLiteral("movebyy");
        inline const QString Movetorand        = QStringLiteral("movetorand");
        inline const QString Moverandx         = QStringLiteral("moverandx");
        inline const QString Moverandy         = QStringLiteral("moverandy");
        inline const QString Restrictx         = QStringLiteral("restrictx");
        inline const QString Restricty         = QStringLiteral("restricty");
        inline const QString Letinitx          = QStringLiteral("letinitx");
        inline const QString Letinity          = QStringLiteral("letinity");
        inline const QString Letobjectx        = QStringLiteral("letobjectx");
        inline const QString Letobjecty        = QStringLiteral("letobjecty");
        inline const QString Letcatch          = QStringLiteral("letcatch");
        inline const QString Debug             = QStringLiteral("debug");
        inline const QString Notify            = QStringLiteral("notify");
        inline const QString Shell             = QStringLiteral("shell");
        inline const QString Music             = QStringLiteral("music");
        inline const QString Sound             = QStringLiteral("sound");
        inline const QString Transparent       = QStringLiteral("transparent");
        inline const QString Lettransparent    = QStringLiteral("lettransparent");
        inline const QString Attach            = QStringLiteral("attach");
        inline const QString Glue              = QStringLiteral("glue");
        inline const QString Detach            = QStringLiteral("detach");
        inline const QString Letchild          = QStringLiteral("letchild");
        inline const QString Letparent         = QStringLiteral("letparent");
        inline const QString Letsibling        = QStringLiteral("letsibling");
        inline const QString Setframe          = QStringLiteral("setframe");
        inline const QString Letframe          = QStringLiteral("letframe");
        inline const QString Add               = QStringLiteral("add");
        inline const QString Sub               = QStringLiteral("sub");
        inline const QString Mul               = QStringLiteral("mul");
        inline const QString Div               = QStringLiteral("div");
        inline const QString Mod               = QStringLiteral("mod");
        inline const QString Random            = QStringLiteral("random");
        inline const QString Setkcf            = QStringLiteral("setkcf");
        inline const QString Letkcf            = QStringLiteral("letkcf");
        inline const QString Setfix            = QStringLiteral("setfix");
        inline const QString Letfix            = QStringLiteral("letfix");
        inline const QString Ghost             = QStringLiteral("ghost");
        inline const QString Letwidth          = QStringLiteral("letwidth");
        inline const QString Letheight         = QStringLiteral("letheight");
        inline const QString Letcollide        = QStringLiteral("letcollide");
        inline const QString Letinside         = QStringLiteral("letinside");
        inline const QString Ifequal           = QStringLiteral("ifequal");
        inline const QString Ifnotequal        = QStringLiteral("ifnotequal");
        inline const QString Ifgreaterthan     = QStringLiteral("ifgreaterthan");
        inline const QString Iflessthan        = QStringLiteral("iflessthan");
        inline const QString Else              = QStringLiteral("else");
        inline const QString Elseifequal       = QStringLiteral("elseifequal");
        inline const QString Elseifnotequal    = QStringLiteral("elseifnotequal");
        inline const QString Elseifgreaterthan = QStringLiteral("elseifgreaterthan");
        inline const QString Elseiflessthan    = QStringLiteral("elseiflessthan");
        inline const QString Endif             = QStringLiteral("endif");
        inline const QString Letkey            = QStringLiteral("letkey");
        inline const QString Letkeymap         = QStringLiteral("letkeymap");
        inline const QString Letmousex         = QStringLiteral("letmousex");
        inline const QString Letmousey         = QStringLiteral("letmousey");
        inline const QString Changecol         = QStringLiteral("changecol");
        inline const QString Changeset         = QStringLiteral("changeset");
        inline const QString Letpal            = QStringLiteral("letpal");
        inline const QString Letset            = QStringLiteral("letset");
        inline const QString Windowsize        = QStringLiteral("windowsize");
        inline const QString Viewport          = QStringLiteral("viewport");
        inline const QString Nop               = QStringLiteral("nop");
        inline const QString Quit              = QStringLiteral("quit");
        inline const QString Valuepool         = QStringLiteral("valuepool");
        inline const QString Savevalue         = QStringLiteral("savevalue");
        inline const QString Loadvalue         = QStringLiteral("loadvalue");
        inline const QString Deletevalue       = QStringLiteral("deletevalue");
        inline const QString Let               = QStringLiteral("let");
    }
}

#endif // FKISS_H
