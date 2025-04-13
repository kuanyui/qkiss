#include "fkiss.h"
#include "fkissparser.h"
#include "fkissruntime.h"
#include <QRegularExpression>
#include <QHashIterator>
using namespace Qt::Literals::StringLiterals;

Fkiss::Fkiss(QObject *parent)
    : QObject{parent},
      m_parser{new FkissParser()},
      m_runtime(new FkissRuntime())
{
    qDebug() << "[fkiss] ctor";
}

Fkiss::~Fkiss()
{
    qDebug() << "[fkiss] dtor";
    delete m_parser;
    delete m_runtime;
}

void Fkiss::parseAndCacheFkissScript(QList<FkissLineInfo> fkissScriptLines)
{
    m_parsedScriptCache = m_parser->parseScriptLines(fkissScriptLines);
}

void Fkiss::startRuntime(GKissScene *kissScene)
{
    m_runtime->loadParsedFkissScript(m_parsedScriptCache);
    m_runtime->setCurKissScene(kissScene);
    m_runtime->triggerInitializeAndBegin();
}

void Fkiss::stopRuntime()
{
    m_runtime->triggerEnd();
    m_runtime->stopTimersAndUnloadFkissScript();
    m_runtime->setCurKissScene(nullptr);
}


FkissRuntime *Fkiss::runtime()
{
    return m_runtime;
}

FkissParser *Fkiss::parser()
{
    return m_parser;
}



QDebug operator<<(QDebug debug, const ASTCommand& astCmd) {
    auto dbg = debug.noquote().nospace() << "[ASTCommand] " << astCmd.name << "( ";
    foreach (auto arg, astCmd.args) {
        dbg << arg;
    }
    return dbg << ")";
};

const QHash<QString, FkissKeywordInfo> Fkiss::KEYWORDS_EVENT = {
    // ### Unique ###
    // Only one of each of these events can be defined in a single KiSS Artform.
    { FKK::E::INITIALIZE       , { FKK::E::INITIALIZE       , FkissVersion::FKiSS1, 1 } }, // initialize() - Called exactly once before the artform is displayed on the screen for the first time.
    { FKK::E::BEGIN            , { FKK::E::BEGIN            , FkissVersion::FKiSS1, 1 } }, // begin() - Called exactly once immediately after the artform is displayed on the screen for the first time.
    { FKK::E::END              , { FKK::E::END              , FkissVersion::FKiSS1, 1 } }, // end() - Called exactly once immediately before the user quits the doll.
    { FKK::E::OVERFLOW         , { FKK::E::OVERFLOW         , FkissVersion::FKiSS3, false } }, // overflow() - Called when a mathematical action attempts to divide by zero or when the result would be outside the range of a 32-bit integer.
    { FKK::E::NEVER            , { FKK::E::NEVER            , FkissVersion::FKiSS1, false } }, // never() - Never called. Used to temporarily block segments of FKiSS code.

    // ### Specific ###
    // These events are specific to sets, colors, and version.
    { FKK::E::SET              , { FKK::E::SET              , FkissVersion::FKiSS1, 1 } }, // set(page number) - Called when the viewer changes to the specified set (page).
    { FKK::E::COL              , { FKK::E::COL              , FkissVersion::FKiSS1, 1 } }, // col(palette number) - Called when the viewer changes to the specified palette.
    { FKK::E::VERSION          , { FKK::E::VERSION          , FkissVersion::FKiSS2, 1 } }, // version(version number) - Called immediately after the begin event if the specified version of FKiSS is supported.

    // ### Named ###
    // These events are associated with named blocks of code.
    { FKK::E::ALARM            , { FKK::E::ALARM            , FkissVersion::FKiSS1, 1 } }, // alarm(alarm ID) - Called when the timer for a specified alarm reaches zero.
    { FKK::E::LABEL            , { FKK::E::LABEL            , FkissVersion::FKiSS3, false } }, // label(label ID) - Defines a named block of code that can be called from other parts of the script.

    // ### Rectangle-based collision ###
    // These events are associated with collisions between rectangles.
    // Rectangle collision is faster than pixel-based collisions but not pixel accurate.
    { FKK::E::IN               , { FKK::E::IN               , FkissVersion::FKiSS2, 1 } }, // in(target1, target2) - Triggered when target1 overlaps target2 and previously did not.
    { FKK::E::OUT              , { FKK::E::OUT              , FkissVersion::FKiSS2, 1 } }, // out(target1, target2) - Triggered when target1 no longer overlaps target2 and previously did.

    { FKK::E::STILLIN          , { FKK::E::STILLIN          , FkissVersion::FKiSS2, 1 } }, // stillin(target1, target2) - Triggered when target1 still overlaps target2, regardless of previous state.
    { FKK::E::STILLOUT         , { FKK::E::STILLOUT         , FkissVersion::FKiSS2, 1 } }, // stillout(target1, target2) - Triggered when target1 still does not overlap target2, regardless of previous state.

    // ### Pixel-based collision ###
    // These events are associated with collisions between pixels of cels.
    { FKK::E::COLLIDE          , { FKK::E::COLLIDE          , FkissVersion::FKiSS2, 1 } }, // collide(target1, target2) - Triggered when pixels of target1 overlap pixels of target2 and previously did not.
    { FKK::E::APART            , { FKK::E::APART            , FkissVersion::FKiSS2, 1 } }, // apart(target1, target2) - Triggered when pixels of target1 no longer overlap pixels of target2 and previously did.

    // ### Mouse-based collision ###
    // These events are associated with the movement of the mouse cursor.
    { FKK::E::MOUSEIN          , { FKK::E::MOUSEIN          , FkissVersion::FKiSS4, false } }, // mousein(target) - Triggered when the mouse passes into the bounding rectangle for target.
    { FKK::E::MOUSEOUT         , { FKK::E::MOUSEOUT         , FkissVersion::FKiSS4, false } }, // mouseout(target) - Triggered when the mouse passes out of the bounding rectangle for target.

    // ### Keyboard ###
    // These events are associated with pressing of keys.
    { FKK::E::KEYPRESS         , { FKK::E::KEYPRESS         , FkissVersion::FKiSS4, false } }, // keypress(keyname) - Triggered when the user presses the corresponding key on the keyboard.
    { FKK::E::KEYRELEASE       , { FKK::E::KEYRELEASE       , FkissVersion::FKiSS4, false } }, // keyrelease(keyname) - Triggered when the user releases the corresponding key on the keyboard.

    // ### Click-and-drag ###
    // These events are associated with clicking on cels as well as clicking and dragging cels around.
    { FKK::E::PRESS            , { FKK::E::PRESS            , FkissVersion::FKiSS1, 1 } }, // press(target) - Triggered when the user clicks on the target.
    { FKK::E::CATCH            , { FKK::E::CATCH            , FkissVersion::FKiSS1, 1 } }, // catch(target) - The catch event is triggered when the user first begins to move the target.
    { FKK::E::DROP             , { FKK::E::DROP             , FkissVersion::FKiSS1, 1 } }, // drop(target) - The drop event is triggered when the user loses control of the target after attempting to move it.

    { FKK::E::RELEASE          , { FKK::E::RELEASE          , FkissVersion::FKiSS1, 1 } }, // release(target) - Triggered when the user releases the mouse button after having clicked on the target.
    { FKK::E::FIXCATCH         , { FKK::E::FIXCATCH         , FkissVersion::FKiSS1, 1 } }, // fixcatch(target) - Triggered when the user first begins to move a target which has a non-zero fix value.
    { FKK::E::FIXDROP          , { FKK::E::FIXDROP          , FkissVersion::FKiSS1, 1 } }, // fixdrop(target) - Triggered when the user loses control of the target after attempting to move it due to the fix value of the object.

    // ### Fix ###
    // These events are associated with the fix value of an object.
    { FKK::E::UNFIX            , { FKK::E::UNFIX            , FkissVersion::FKiSS1, 1 } }, // unfix(target) - Triggered when the target's fix value reaches 0.

    // ### Attachment ###
    // This event is associated with detaching a target from a parent object.
    { FKK::E::DETACHED         , { FKK::E::DETACHED         , FkissVersion::FKiSS4, false }} // detached(target) - Triggered when the target is detached from its parent object.
};


const QHash<QString, FkissKeywordInfo> Fkiss::KEYWORDS_ACTION = {
    // ### Alarms ###
    // These actions set up alarms, which are events triggered when certain conditions occur, such as a timer going off.
    { FKK::A::TIMER            , { FKK::A::TIMER            , FkissVersion::FKiSS1, 1 } }, // timer(alarm ID, delay) - Sets the timer for an alarm. (delay = 0 means cancel the timer)
    { FKK::A::RANDOMTIMER      , { FKK::A::RANDOMTIMER      , FkissVersion::FKiSS1, 1 } }, // randomtimer(alarm ID, delay, range) - Sets the timer for an alarm with a random delay.
    { FKK::A::LETTIMER         , { FKK::A::LETTIMER         , FkissVersion::FKiSS4, false } }, // lettimer(variable, alarm ID) - Sets the value of variable to the amount of time remaining before the timer for alarm ID reaches zero.
    { FKK::A::IFFIXED          , { FKK::A::IFFIXED          , FkissVersion::FKiSS2, false } }, // iffixed(target, alarm ID, delay) - Sets the timer for an alarm if the fix value for the object specified by target is non-zero.
    { FKK::A::IFMAPPED         , { FKK::A::IFMAPPED         , FkissVersion::FKiSS2, false } }, // ifmapped(target, alarm ID, delay) - Sets the timer for an alarm if any cel specified by target is currently mapped.
    { FKK::A::IFMOVED          , { FKK::A::IFMOVED          , FkissVersion::FKiSS2, false } }, // ifmoved(target, alarm ID, delay) - Sets the timer for an alarm if the object specified by target is not in its initial position on the current set.
    { FKK::A::IFNOTFIXED       , { FKK::A::IFNOTFIXED       , FkissVersion::FKiSS2, false } }, // ifnotfixed(target, alarm ID, delay) - Sets the timer for an alarm if the fix value for the object specified by target is zero.
    { FKK::A::IFNOTMAPPED      , { FKK::A::IFNOTMAPPED      , FkissVersion::FKiSS2, false } }, // ifnotmapped(target, alarm ID, delay) - Sets the timer for an alarm if no cel or no cel in the object or group specified by target is currently mapped.
    { FKK::A::IFNOTMOVED       , { FKK::A::IFNOTMOVED       , FkissVersion::FKiSS2, false } }, // ifnotmoved(target, alarm ID, delay) - Sets the timer for an alarm if the object specified by target is in its initial position on the current set.
    { FKK::A::EXITEVENT        , { FKK::A::EXITEVENT        , FkissVersion::FKiSS3, false } }, // exitevent() - Exits the current event. No more code in this event will be processed.

    // ### Labels ###
    // These actions use labels that mark a block of actions to execute that block of actions from several different places.
    { FKK::A::GOSUB            , { FKK::A::GOSUB            , FkissVersion::FKiSS3, false } }, // gosub(label ID) - Jumps to the specified label and returns when that label finishes.
    { FKK::A::GOSUBRANDOM      , { FKK::A::GOSUBRANDOM      , FkissVersion::FKiSS3, false } }, // gosubrandom(chance, label ID A, label ID B) - Randomly jumps to one of two specified labels and returns when that label finishes.
    { FKK::A::GOTO             , { FKK::A::GOTO             , FkissVersion::FKiSS3, false } }, // goto(label ID) - Jumps to the specified label without returning.
    { FKK::A::GOTORANDOM       , { FKK::A::GOTORANDOM       , FkissVersion::FKiSS3, false } }, // gotorandom(chance, label ID A, label ID B) - Randomly jumps to one of two specified labels without returning.
    { FKK::A::REPEAT           , { FKK::A::REPEAT           , FkissVersion::FKiSS4, false } }, // repeat(label ID, times, count) - Calls the specified label a given number of times.
    { FKK::A::EXITLOOP         , { FKK::A::EXITLOOP         , FkissVersion::FKiSS4, false } }, // exitloop() - Exits the current event and terminates the current repeat() loop.

    // ### Mapping ###
    // These actions control the visibility of a cel. A mapped cel is visible while an unmapped cel is not visible.
    { FKK::A::MAP              , { FKK::A::MAP              , FkissVersion::FKiSS1, 1 } }, // map(target) - Makes target visible on all sets in which it is present.
    { FKK::A::UNMAP            , { FKK::A::UNMAP            , FkissVersion::FKiSS1, 1 } }, // unmap(target) - Makes target invisible on all sets in which it is present.
    { FKK::A::ALTMAP           , { FKK::A::ALTMAP           , FkissVersion::FKiSS1, 1 } }, // altmap(target) - Toggles the visibility of target on all sets in which it is present.
    { FKK::A::LETMAPPED        , { FKK::A::LETMAPPED        , FkissVersion::FKiSS3, false } }, // letmapped(variable, target) - Sets the value of variable to 1 if target is mapped or to 0 if it is unmapped.

    // ### Movement and Position ###
    // These actions control the position of a target and allow an action to move a cel in response to an event.
    { FKK::A::MOVE             , { FKK::A::MOVE             , FkissVersion::FKiSS1, 1 } }, // move(target, X, Y) - Moves target X pixels horizontally and Y pixels vertically relative to its current position.
    { FKK::A::MOVETO           , { FKK::A::MOVETO           , FkissVersion::FKiSS2, 1 } }, // moveto(target, X, Y) - Moves target to absolute play field position <X, Y>.
    { FKK::A::MOVEBYX          , { FKK::A::MOVEBYX          , FkissVersion::FKiSS2, false } }, // movebyx(target, relative, X) - Moves target (obj only) to a position X pixels horizontally from the position of object relative.
    { FKK::A::MOVEBYY          , { FKK::A::MOVEBYY          , FkissVersion::FKiSS2, false } }, // movebyy(target, relative, Y) - Moves target (obj only) to a position Y pixels vertically from the position of object relative.
    { FKK::A::MOVETORAND       , { FKK::A::MOVETORAND       , FkissVersion::FKiSS2, false } }, // movetorand(target) - Moves target to a random position within the play field or its restricted range of movement.
    { FKK::A::MOVERANDX        , { FKK::A::MOVERANDX        , FkissVersion::FKiSS2, false } }, // moverandx(target, min, max) - Moves target by a random horizontal distance in the range min to max relative to its current position.
    { FKK::A::MOVERANDY        , { FKK::A::MOVERANDY        , FkissVersion::FKiSS2, false } }, // moverandy(target, min, max) - Moves target by a random vertical distance in the range min to max relative to its current position.
    { FKK::A::RESTRICTX        , { FKK::A::RESTRICTX        , FkissVersion::FKiSS4, false } }, // restrictx(target, min, max) - Changes the horizontal movement limits of target to the absolute range of min to max.
    { FKK::A::RESTRICTY        , { FKK::A::RESTRICTY        , FkissVersion::FKiSS4, false } }, // restricty(target, min, max) - Changes the vertical movement limits of target to the absolute range min to max.
    { FKK::A::LETINITX         , { FKK::A::LETINITX         , FkissVersion::FKiSS4, false } }, // letinitx(variable, target) - Sets the value of variable to the initial horizontal position of target for the current set.
    { FKK::A::LETINITY         , { FKK::A::LETINITY         , FkissVersion::FKiSS4, false } }, // letinity(variable, target) - Sets the value of variable to the initial vertical position of target for the current set.
    { FKK::A::LETOBJECTX       , { FKK::A::LETOBJECTX       , FkissVersion::FKiSS3, false } }, // letobjectx(variable, target) - Sets the value of variable to the current horizontal position of target for the current set.
    { FKK::A::LETOBJECTY       , { FKK::A::LETOBJECTY       , FkissVersion::FKiSS3, false } }, // letobjecty(variable, target) - Sets the value of variable to the current vertical position of target for the current set.
    { FKK::A::LETCATCH         , { FKK::A::LETCATCH         , FkissVersion::FKiSS3, false } }, // letcatch(variable) - Sets the value of variable to number of the object currently being dragged by the user.

    // ### Message ###
    // These actions show messages to the user or trigger a shell command.
    { FKK::A::DEBUG            , { FKK::A::DEBUG            , FkissVersion::FKiSS1, false } }, // debug(string) - Sends the string to debug output for that KiSS viewer.
    { FKK::A::NOTIFY           , { FKK::A::NOTIFY           , FkissVersion::FKiSS2, false } }, // notify(string) - Sends the string as a message to the user.
    { FKK::A::SHELL            , { FKK::A::SHELL            , FkissVersion::FKiSS1, false } }, // shell(string) - Sends the string as a shell command to the system.

    // ### Sound and Music ###
    // These actions control music and sound.
    { FKK::A::MUSIC            , { FKK::A::MUSIC            , FkissVersion::FKiSS2, false } }, // music(filename) - Begins playback of the MIDI filename.
    { FKK::A::SOUND            , { FKK::A::SOUND            , FkissVersion::FKiSS1, 1 } }, // sound(filename) - Begins playback of sound file filename.

    // ### Translucency ###
    // These actions control the translucency of a target.
    { FKK::A::TRANSPARENT      , { FKK::A::TRANSPARENT      , FkissVersion::FKiSS1b, 1 } },   // transparent(target, N) - The transparent action adds N to the translucency value of target.
    { FKK::A::LETTRANSPARENT   , { FKK::A::LETTRANSPARENT   , FkissVersion::FKiSS3, false } },   // lettransparent(variable, target) - The lettransparent action sets the value of variable to the translucency value of target.

    // ### Attachment ###
    // These actions control the process of attaching or detaching an object to another object.
    { FKK::A::ATTACH           , { FKK::A::ATTACH           , FkissVersion::FKiSS4, false } },   // attach(target, parent) - The attach action attaches target to parent, becoming a child object of parent.
    { FKK::A::GLUE             , { FKK::A::GLUE             , FkissVersion::FKiSS4, false } },   // glue(target, parent) - The glue action attaches target to parent, becoming a child object of parent that can only be detached by the detach action.
    { FKK::A::DETACH           , { FKK::A::DETACH           , FkissVersion::FKiSS4, false } },   // detach(target) - The detach action detaches target from its parent object.
    { FKK::A::LETCHILD         , { FKK::A::LETCHILD         , FkissVersion::FKiSS4, false } },   // letchild(variable, target) - The letchild action sets the value of variable to the lowest-numbered attached child object for target.
    { FKK::A::LETPARENT        , { FKK::A::LETPARENT        , FkissVersion::FKiSS4, false } },   // letparent(variable, target) - The letparent action sets the value of variable to the current attached parent object for target.
    { FKK::A::LETSIBLING       , { FKK::A::LETSIBLING       , FkissVersion::FKiSS4, false } },   // letsibling(variable, target) - The letsibling action sets the value of variable to the lowest-numbered object attached to the same parent object as target but with a higher object number than target.

    // ### Group Frame ###
    // These actions change the frame that is displayed for a group of cels.
    { FKK::A::SETFRAME         , { FKK::A::SETFRAME         , FkissVersion::FKiSS4, false } },   // setframe(group, frame) - The setframe action sets the current group frame number of group to frame.
    { FKK::A::LETFRAME         , { FKK::A::LETFRAME         , FkissVersion::FKiSS4, false } },   // letframe(variable, group) - The letframe action sets the value of variable to the current group frame number of group.

    // ### Arithmetic ###
    // These actions perform arithmetic operations on a variable.
    { FKK::A::ADD              , { FKK::A::ADD              , FkissVersion::FKiSS3, false } },   // add(variable, A, B) - The add action sets the value of variable to the value of (A + B).
    { FKK::A::SUB              , { FKK::A::SUB              , FkissVersion::FKiSS3, false } },   // sub(variable, A, B) - The sub action sets the value of variable to the value of (A - B).
    { FKK::A::MUL              , { FKK::A::MUL              , FkissVersion::FKiSS3, false } },   // mul(variable, A, B) - The mul action sets the value of variable to the value of (A x B).
    { FKK::A::DIV              , { FKK::A::DIV              , FkissVersion::FKiSS3, false } },   // div(variable, A, B) - The div action sets the value of variable to the value of (A / B).
    { FKK::A::MOD              , { FKK::A::MOD              , FkissVersion::FKiSS3, false } },   // mod(variable, A, B) - The mod action sets the value of variable to the value of (A modulo B).
    { FKK::A::RANDOM           , { FKK::A::RANDOM           , FkissVersion::FKiSS3, false } },   // random(variable, min, max) - The random action sets the value of variable to a random number in the range of min to max, inclusive.

    // ### Object/Cel State ###
    // These actions control or read the state of an object.
    { FKK::A::SETKCF           , { FKK::A::SETKCF           , FkissVersion::FKiSS4, false } },   // setkcf(target, N) - The setkcf action sets all cels in target to use the KCF N.
    { FKK::A::LETKCF           , { FKK::A::LETKCF           , FkissVersion::FKiSS4, false } },   // letkcf(variable, target) - The letkcf action sets the value of variable to the index of the KCF used by target.
    { FKK::A::SETFIX           , { FKK::A::SETFIX           , FkissVersion::FKiSS2, false } },   // setfix(target, N) - The setfix action sets the fix value for target to N.
    { FKK::A::LETFIX           , { FKK::A::LETFIX           , FkissVersion::FKiSS3, false } },   // letfix(variable, target) - The letfix action sets the value of variable to the fix value of target.
    { FKK::A::GHOST            , { FKK::A::GHOST            , FkissVersion::FKiSS3, false } },   // ghost(target, N) - The ghost action changes the ghost-state of target.
    { FKK::A::LETWIDTH         , { FKK::A::LETWIDTH         , FkissVersion::FKiSS4, false } },   // letwidth(variable, target) - The letwidth action sets the value of variable to the total width of object target, counting only cels which are mapped and present in the current set (page).
    { FKK::A::LETHEIGHT        , { FKK::A::LETHEIGHT        , FkissVersion::FKiSS4, false } },   // letheight(variable, target) - The letheight action sets the value of variable to the total height of object target, counting only cels which are mapped and present in the current set (page).
    { FKK::A::LETCOLLIDE       , { FKK::A::LETCOLLIDE       , FkissVersion::FKiSS3, false } },   // letcollide(variable, target1, target2) - The letcollide action makes a comparison based on the individual, non-transparent pixels of the two targets.
    { FKK::A::LETINSIDE        , { FKK::A::LETINSIDE        , FkissVersion::FKiSS3, false } },   // letinside(variable, target1, target2) - The letinside action makes a comparison based on the individual bounding rectangles of the two targets.

    // ### Conditionals ###
    // These actions control the flow of execution through a block of actions.
    { FKK::A::IFEQUAL          , { FKK::A::IFEQUAL          , FkissVersion::FKiSS3, false } },   // ifequal(A, B) - The ifequal begins a structured if-endif block. The conditional block is executed if A is equal to B.
    { FKK::A::IFNOTEQUAL       , { FKK::A::IFNOTEQUAL       , FkissVersion::FKiSS3, false } },   // ifnotequal(A, B) - The ifnotequal begins a structured if-endif block. The conditional block is executed if A is not equal to B.
    { FKK::A::IFGREATERTHAN    , { FKK::A::IFGREATERTHAN    , FkissVersion::FKiSS3, false } },   // ifgreaterthan(A, B) - The ifgreaterthan begins a structured if-endif block. The conditional block is executed if A is greater than B.
    { FKK::A::IFLESSTHAN       , { FKK::A::IFLESSTHAN       , FkissVersion::FKiSS3, false } },   // iflessthan(A, B) - The iflessthan begins a structured if-endif block. The conditional block is executed if A is less than B.
    { FKK::A::ELSE             , { FKK::A::ELSE             , FkissVersion::FKiSS3, false } },   // else() - The else forms the middle of a structured if-else-endif block.
    { FKK::A::ELSEIFEQUAL      , { FKK::A::ELSEIFEQUAL      , FkissVersion::FKiSS4, false } },   // elseifequal(A, B) - The elseifequal forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is equal to B.
    { FKK::A::ELSEIFNOTEQUAL   , { FKK::A::ELSEIFNOTEQUAL   , FkissVersion::FKiSS4, false } },   // elseifnotequal(A, B) - The elseifnotequal forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is not equal to B.
    { FKK::A::ELSEIFGREATERTHAN, { FKK::A::ELSEIFGREATERTHAN, FkissVersion::FKiSS4, false } },   // elseifgreaterthan(A, B) - The elseifgreaterthan forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is greater than B.
    { FKK::A::ELSEIFLESSTHAN   , { FKK::A::ELSEIFLESSTHAN   , FkissVersion::FKiSS4, false } },   // elseiflessthan(A, B) - The elseiflessthan forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is less than B.
    { FKK::A::ENDIF            , { FKK::A::ENDIF            , FkissVersion::FKiSS3, false } },   // endif() - The endif forms the end of a structured if-else-endif block.

    // ### Keyboard ###
    // These actions read the keyboard.
    { FKK::A::LETKEY           , { FKK::A::LETKEY           , FkissVersion::FKiSS4, false } },   // letkey(variable, keystring) - The letkey action sets the value of variable to the index of the first key in keystring which is pressed.
    { FKK::A::LETKEYMAP        , { FKK::A::LETKEYMAP        , FkissVersion::FKiSS4, false } },   // letkeymap(variable, keystring) - The letkeymap action sets each bit in variable if the corresponding key in keystring is pressed.

    // ### Mouse ###
    // These actions read the position of the mouse cursor.
    { FKK::A::LETMOUSEX        , { FKK::A::LETMOUSEX        , FkissVersion::FKiSS3, false } },   // letmousex(variable) - The letmousex action sets the value of variable to the current horizontal position of the mouse pointer.
    { FKK::A::LETMOUSEY        , { FKK::A::LETMOUSEY        , FkissVersion::FKiSS3, false } },   // letmousey(variable) - The letmousey action sets the value of variable to the current vertical position of the mouse pointer.

    // ### Environment ###
    // These actions affect the environment, set, or play field.
    { FKK::A::CHANGECOL        , { FKK::A::CHANGECOL        , FkissVersion::FKiSS1, 1 } },   // changecol(N) - The changecol action sets the palette for the current page to N.
    { FKK::A::CHANGESET        , { FKK::A::CHANGESET        , FkissVersion::FKiSS1, 1 } },   // changeset(N) - The changeset action changes to the set (page) specified by N.
    { FKK::A::LETPAL           , { FKK::A::LETPAL           , FkissVersion::FKiSS3, false } },   // letpal(variable) - The letpal action sets the value of variable to the current palette for the current set (page).
    { FKK::A::LETSET           , { FKK::A::LETSET           , FkissVersion::FKiSS3, false } },   // letset(variable) - The letset action sets the value of variable to the current set (page).

    // ### Viewport ###
    // These actions control the size of the window and view on that window.
    { FKK::A::WINDOWSIZE       , { FKK::A::WINDOWSIZE       , FkissVersion::FKiSS1, false } },   // windowsize(W, H) - The windowsize action sets the size of the viewport to <W x H>.
    { FKK::A::VIEWPORT         , { FKK::A::VIEWPORT         , FkissVersion::FKiSS1, false } },   // viewport(X, Y) - The viewport action sets the play field position of the top-left corner of the viewport to <X, Y>.

    // ### Miscellaneous ###
    // These actions fall in no particular category.
    { FKK::A::NOP              , { FKK::A::NOP              , FkissVersion::FKiSS1, false } },   // nop() - The nop action does nothing. It can be used as a placeholder in an event that may not have any actions in it.
    { FKK::A::QUIT             , { FKK::A::QUIT             , FkissVersion::FKiSS1b, false } },   // quit() - The quit action closes the KiSS doll.

    // ### Pool Values ###
    // These actions control access to value pools.
    { FKK::A::VALUEPOOL        , { FKK::A::VALUEPOOL        , FkissVersion::FKiSS4, false } },   // valuepool(pool) - The valuepool action sets the current value pool where data will be saved, loaded, or deleted.
    { FKK::A::SAVEVALUE        , { FKK::A::SAVEVALUE        , FkissVersion::FKiSS4, false } },   // savevalue(name, value) - The savevalue action saves a value to disk which can be loaded again later, even during future sessions.
    { FKK::A::LOADVALUE        , { FKK::A::LOADVALUE        , FkissVersion::FKiSS4, false } },   // loadvalue(variable, name) - The loadvalue action loads a value from disk which could have been saved earlier, even during previous sessions.
    { FKK::A::DELETEVALUE      , { FKK::A::DELETEVALUE      , FkissVersion::FKiSS4, false } },   // deletevalue(name) - The deletevalue action removes a named value from the current value pool.

    // ### Variables ###
    // These actions affect variables.
    { FKK::A::LET              , { FKK::A::LET              , FkissVersion::FKiSS3, false } }   // let(variable, A) - The let action sets the value of variable to the value of A.
};
