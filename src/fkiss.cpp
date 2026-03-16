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

#include "fkiss.h"
#include "fkissparser.h"
#include "fkissruntime.h"
#include "logging.h"
#include <QRegularExpression>
#include <QHashIterator>
using namespace Qt::Literals::StringLiterals;

Fkiss::Fkiss(QObject *parent)
    : QObject{parent},
      m_parser{new FkissParser()},
      m_runtime(new FkissRuntime())
{
    qkDebug << "[fkiss] ctor";
}

Fkiss::~Fkiss()
{
    qkDebug << "[fkiss] dtor";
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
    { FKK::E::Initialize       , { FKK::E::Initialize       , FkissVersion::FKiSS1, 1 } }, // initialize() - Called exactly once before the artform is displayed on the screen for the first time.
    { FKK::E::Begin            , { FKK::E::Begin            , FkissVersion::FKiSS1, 1 } }, // begin() - Called exactly once immediately after the artform is displayed on the screen for the first time.
    { FKK::E::End              , { FKK::E::End              , FkissVersion::FKiSS1, 1 } }, // end() - Called exactly once immediately before the user quits the doll.
    { FKK::E::Overflow         , { FKK::E::Overflow         , FkissVersion::FKiSS3, false } }, // overflow() - Called when a mathematical action attempts to divide by zero or when the result would be outside the range of a 32-bit integer.
    { FKK::E::Never            , { FKK::E::Never            , FkissVersion::FKiSS1, false } }, // never() - Never called. Used to temporarily block segments of FKiSS code.

    // ### Specific ###
    // These events are specific to sets, colors, and version.
    { FKK::E::Set              , { FKK::E::Set              , FkissVersion::FKiSS1, 1 } }, // set(page number) - Called when the viewer changes to the specified set (page).
    { FKK::E::Col              , { FKK::E::Col              , FkissVersion::FKiSS1, 1 } }, // col(palette number) - Called when the viewer changes to the specified palette.
    { FKK::E::Version          , { FKK::E::Version          , FkissVersion::FKiSS2, 1 } }, // version(version number) - Called immediately after the begin event if the specified version of FKiSS is supported.

    // ### Named ###
    // These events are associated with named blocks of code.
    { FKK::E::Alarm            , { FKK::E::Alarm            , FkissVersion::FKiSS1, 1 } }, // alarm(alarm ID) - Called when the timer for a specified alarm reaches zero.
    { FKK::E::Label            , { FKK::E::Label            , FkissVersion::FKiSS3, false } }, // label(label ID) - Defines a named block of code that can be called from other parts of the script.

    // ### Rectangle-based collision ###
    // These events are associated with collisions between rectangles.
    // Rectangle collision is faster than pixel-based collisions but not pixel accurate.
    { FKK::E::In               , { FKK::E::In               , FkissVersion::FKiSS2, 1 } }, // in(target1, target2) - Triggered when target1 overlaps target2 and previously did not.
    { FKK::E::Out              , { FKK::E::Out              , FkissVersion::FKiSS2, 1 } }, // out(target1, target2) - Triggered when target1 no longer overlaps target2 and previously did.

    { FKK::E::Stillin          , { FKK::E::Stillin          , FkissVersion::FKiSS2, 1 } }, // stillin(target1, target2) - Triggered when target1 still overlaps target2, regardless of previous state.
    { FKK::E::Stillout         , { FKK::E::Stillout         , FkissVersion::FKiSS2, 1 } }, // stillout(target1, target2) - Triggered when target1 still does not overlap target2, regardless of previous state.

    // ### Pixel-based collision ###
    // These events are associated with collisions between pixels of cels.
    { FKK::E::Collide          , { FKK::E::Collide          , FkissVersion::FKiSS2, 1 } }, // collide(target1, target2) - Triggered when pixels of target1 overlap pixels of target2 and previously did not.
    { FKK::E::Apart            , { FKK::E::Apart            , FkissVersion::FKiSS2, 1 } }, // apart(target1, target2) - Triggered when pixels of target1 no longer overlap pixels of target2 and previously did.

    // ### Mouse-based collision ###
    // These events are associated with the movement of the mouse cursor.
    { FKK::E::Mousein          , { FKK::E::Mousein          , FkissVersion::FKiSS4, false } }, // mousein(target) - Triggered when the mouse passes into the bounding rectangle for target.
    { FKK::E::Mouseout         , { FKK::E::Mouseout         , FkissVersion::FKiSS4, false } }, // mouseout(target) - Triggered when the mouse passes out of the bounding rectangle for target.

    // ### Keyboard ###
    // These events are associated with pressing of keys.
    { FKK::E::Keypress         , { FKK::E::Keypress         , FkissVersion::FKiSS4, false } }, // keypress(keyname) - Triggered when the user presses the corresponding key on the keyboard.
    { FKK::E::Keyrelease       , { FKK::E::Keyrelease       , FkissVersion::FKiSS4, false } }, // keyrelease(keyname) - Triggered when the user releases the corresponding key on the keyboard.

    // ### Click-and-drag ###
    // These events are associated with clicking on cels as well as clicking and dragging cels around.
    { FKK::E::Press            , { FKK::E::Press            , FkissVersion::FKiSS1, 1 } }, // press(target) - Triggered when the user clicks on the target.
    { FKK::E::Catch            , { FKK::E::Catch            , FkissVersion::FKiSS1, 1 } }, // catch(target) - The catch event is triggered when the user first begins to move the target.
    { FKK::E::Drop             , { FKK::E::Drop             , FkissVersion::FKiSS1, 1 } }, // drop(target) - The drop event is triggered when the user loses control of the target after attempting to move it.

    { FKK::E::Release          , { FKK::E::Release          , FkissVersion::FKiSS1, 1 } }, // release(target) - Triggered when the user releases the mouse button after having clicked on the target.
    { FKK::E::Fixcatch         , { FKK::E::Fixcatch         , FkissVersion::FKiSS1, 1 } }, // fixcatch(target) - Triggered when the user first begins to move a target which has a non-zero fix value.
    { FKK::E::Fixdrop          , { FKK::E::Fixdrop          , FkissVersion::FKiSS1, 1 } }, // fixdrop(target) - Triggered when the user loses control of the target after attempting to move it due to the fix value of the object.

    // ### Fix ###
    // These events are associated with the fix value of an object.
    { FKK::E::Unfix            , { FKK::E::Unfix            , FkissVersion::FKiSS1, 1 } }, // unfix(target) - Triggered when the target's fix value reaches 0.

    // ### Attachment ###
    // This event is associated with detaching a target from a parent object.
    { FKK::E::Detached         , { FKK::E::Detached         , FkissVersion::FKiSS4, false }} // detached(target) - Triggered when the target is detached from its parent object.
};


const QHash<QString, FkissKeywordInfo> Fkiss::KEYWORDS_ACTION = {
    // ### Alarms ###
    // These actions set up alarms, which are events triggered when certain conditions occur, such as a timer going off.
    { FKK::A::Timer            , { FKK::A::Timer            , FkissVersion::FKiSS1, 1 } }, // timer(alarm ID, delay) - Sets the timer for an alarm. (delay = 0 means cancel the timer)
    { FKK::A::Randomtimer      , { FKK::A::Randomtimer      , FkissVersion::FKiSS1, 1 } }, // randomtimer(alarm ID, delay, range) - Sets the timer for an alarm with a random delay.
    { FKK::A::Lettimer         , { FKK::A::Lettimer         , FkissVersion::FKiSS4, false } }, // lettimer(variable, alarm ID) - Sets the value of variable to the amount of time remaining before the timer for alarm ID reaches zero.
    { FKK::A::Iffixed          , { FKK::A::Iffixed          , FkissVersion::FKiSS2, 1 } }, // iffixed(target, alarm ID, delay) - Sets the timer for an alarm if the fix value for the object specified by target is non-zero.
    { FKK::A::Ifmapped         , { FKK::A::Ifmapped         , FkissVersion::FKiSS2, 1 } }, // ifmapped(target, alarm ID, delay) - Sets the timer for an alarm if any cel specified by target is currently mapped.
    { FKK::A::Ifmoved          , { FKK::A::Ifmoved          , FkissVersion::FKiSS2, 1 } }, // ifmoved(target, alarm ID, delay) - Sets the timer for an alarm if the object specified by target is not in its initial position on the current set.
    { FKK::A::Ifnotfixed       , { FKK::A::Ifnotfixed       , FkissVersion::FKiSS2, 1 } }, // ifnotfixed(target, alarm ID, delay) - Sets the timer for an alarm if the fix value for the object specified by target is zero.
    { FKK::A::Ifnotmapped      , { FKK::A::Ifnotmapped      , FkissVersion::FKiSS2, 1 } }, // ifnotmapped(target, alarm ID, delay) - Sets the timer for an alarm if no cel or no cel in the object or group specified by target is currently mapped.
    { FKK::A::Ifnotmoved       , { FKK::A::Ifnotmoved       , FkissVersion::FKiSS2, 1 } }, // ifnotmoved(target, alarm ID, delay) - Sets the timer for an alarm if the object specified by target is in its initial position on the current set.
    { FKK::A::Exitevent        , { FKK::A::Exitevent        , FkissVersion::FKiSS3, false } }, // exitevent() - Exits the current event. No more code in this event will be processed.

    // ### Labels ###
    // These actions use labels that mark a block of actions to execute that block of actions from several different places.
    { FKK::A::Gosub            , { FKK::A::Gosub            , FkissVersion::FKiSS3, false } }, // gosub(label ID) - Jumps to the specified label and returns when that label finishes.
    { FKK::A::Gosubrandom      , { FKK::A::Gosubrandom      , FkissVersion::FKiSS3, false } }, // gosubrandom(chance, label ID A, label ID B) - Randomly jumps to one of two specified labels and returns when that label finishes.
    { FKK::A::Goto             , { FKK::A::Goto             , FkissVersion::FKiSS3, false } }, // goto(label ID) - Jumps to the specified label without returning.
    { FKK::A::Gotorandom       , { FKK::A::Gotorandom       , FkissVersion::FKiSS3, false } }, // gotorandom(chance, label ID A, label ID B) - Randomly jumps to one of two specified labels without returning.
    { FKK::A::Repeat           , { FKK::A::Repeat           , FkissVersion::FKiSS4, false } }, // repeat(label ID, times, count) - Calls the specified label a given number of times.
    { FKK::A::Exitloop         , { FKK::A::Exitloop         , FkissVersion::FKiSS4, false } }, // exitloop() - Exits the current event and terminates the current repeat() loop.

    // ### Mapping ###
    // These actions control the visibility of a cel. A mapped cel is visible while an unmapped cel is not visible.
    { FKK::A::Map              , { FKK::A::Map              , FkissVersion::FKiSS1, 1 } }, // map(target) - Makes target visible on all sets in which it is present.
    { FKK::A::Unmap            , { FKK::A::Unmap            , FkissVersion::FKiSS1, 1 } }, // unmap(target) - Makes target invisible on all sets in which it is present.
    { FKK::A::Altmap           , { FKK::A::Altmap           , FkissVersion::FKiSS1, 1 } }, // altmap(target) - Toggles the visibility of target on all sets in which it is present.
    { FKK::A::Letmapped        , { FKK::A::Letmapped        , FkissVersion::FKiSS3, false } }, // letmapped(variable, target) - Sets the value of variable to 1 if target is mapped or to 0 if it is unmapped.

    // ### Movement and Position ###
    // These actions control the position of a target and allow an action to move a cel in response to an event.
    { FKK::A::Move             , { FKK::A::Move             , FkissVersion::FKiSS1, 1 } }, // move(target, X, Y) - Moves target X pixels horizontally and Y pixels vertically relative to its current position.
    { FKK::A::Moveto           , { FKK::A::Moveto           , FkissVersion::FKiSS2, 1 } }, // moveto(target, X, Y) - Moves target to absolute play field position <X, Y>.
    { FKK::A::Movebyx          , { FKK::A::Movebyx          , FkissVersion::FKiSS2, false } }, // movebyx(target, relative, X) - Moves target (obj only) to a position X pixels horizontally from the position of object relative.
    { FKK::A::Movebyy          , { FKK::A::Movebyy          , FkissVersion::FKiSS2, false } }, // movebyy(target, relative, Y) - Moves target (obj only) to a position Y pixels vertically from the position of object relative.
    { FKK::A::Movetorand       , { FKK::A::Movetorand       , FkissVersion::FKiSS2, false } }, // movetorand(target) - Moves target to a random position within the play field or its restricted range of movement.
    { FKK::A::Moverandx        , { FKK::A::Moverandx        , FkissVersion::FKiSS2, false } }, // moverandx(target, min, max) - Moves target by a random horizontal distance in the range min to max relative to its current position.
    { FKK::A::Moverandy        , { FKK::A::Moverandy        , FkissVersion::FKiSS2, false } }, // moverandy(target, min, max) - Moves target by a random vertical distance in the range min to max relative to its current position.
    { FKK::A::Restrictx        , { FKK::A::Restrictx        , FkissVersion::FKiSS4, false } }, // restrictx(target, min, max) - Changes the horizontal movement limits of target to the absolute range of min to max.
    { FKK::A::Restricty        , { FKK::A::Restricty        , FkissVersion::FKiSS4, false } }, // restricty(target, min, max) - Changes the vertical movement limits of target to the absolute range min to max.
    { FKK::A::Letinitx         , { FKK::A::Letinitx         , FkissVersion::FKiSS4, false } }, // letinitx(variable, target) - Sets the value of variable to the initial horizontal position of target for the current set.
    { FKK::A::Letinity         , { FKK::A::Letinity         , FkissVersion::FKiSS4, false } }, // letinity(variable, target) - Sets the value of variable to the initial vertical position of target for the current set.
    { FKK::A::Letobjectx       , { FKK::A::Letobjectx       , FkissVersion::FKiSS3, false } }, // letobjectx(variable, target) - Sets the value of variable to the current horizontal position of target for the current set.
    { FKK::A::Letobjecty       , { FKK::A::Letobjecty       , FkissVersion::FKiSS3, false } }, // letobjecty(variable, target) - Sets the value of variable to the current vertical position of target for the current set.
    { FKK::A::Letcatch         , { FKK::A::Letcatch         , FkissVersion::FKiSS3, false } }, // letcatch(variable) - Sets the value of variable to number of the object currently being dragged by the user.

    // ### Message ###
    // These actions show messages to the user or trigger a shell command.
    { FKK::A::Debug            , { FKK::A::Debug            , FkissVersion::FKiSS1, false } }, // debug(string) - Sends the string to debug output for that KiSS viewer.
    { FKK::A::Notify           , { FKK::A::Notify           , FkissVersion::FKiSS2, false } }, // notify(string) - Sends the string as a message to the user.
    { FKK::A::Shell            , { FKK::A::Shell            , FkissVersion::FKiSS1, false } }, // shell(string) - Sends the string as a shell command to the system.

    // ### Sound and Music ###
    // These actions control music and sound.
    { FKK::A::Music            , { FKK::A::Music            , FkissVersion::FKiSS2, false } }, // music(filename) - Begins playback of the MIDI filename.
    { FKK::A::Sound            , { FKK::A::Sound            , FkissVersion::FKiSS1, 1 } }, // sound(filename) - Begins playback of sound file filename.

    // ### Translucency ###
    // These actions control the translucency of a target.
    { FKK::A::Transparent      , { FKK::A::Transparent      , FkissVersion::FKiSS1b, 1 } },   // transparent(target, N) - The transparent action adds N to the translucency value of target.
    { FKK::A::Lettransparent   , { FKK::A::Lettransparent   , FkissVersion::FKiSS3, false } },   // lettransparent(variable, target) - The lettransparent action sets the value of variable to the translucency value of target.

    // ### Attachment ###
    // These actions control the process of attaching or detaching an object to another object.
    { FKK::A::Attach           , { FKK::A::Attach           , FkissVersion::FKiSS4, false } },   // attach(target, parent) - The attach action attaches target to parent, becoming a child object of parent.
    { FKK::A::Glue             , { FKK::A::Glue             , FkissVersion::FKiSS4, false } },   // glue(target, parent) - The glue action attaches target to parent, becoming a child object of parent that can only be detached by the detach action.
    { FKK::A::Detach           , { FKK::A::Detach           , FkissVersion::FKiSS4, false } },   // detach(target) - The detach action detaches target from its parent object.
    { FKK::A::Letchild         , { FKK::A::Letchild         , FkissVersion::FKiSS4, false } },   // letchild(variable, target) - The letchild action sets the value of variable to the lowest-numbered attached child object for target.
    { FKK::A::Letparent        , { FKK::A::Letparent        , FkissVersion::FKiSS4, false } },   // letparent(variable, target) - The letparent action sets the value of variable to the current attached parent object for target.
    { FKK::A::Letsibling       , { FKK::A::Letsibling       , FkissVersion::FKiSS4, false } },   // letsibling(variable, target) - The letsibling action sets the value of variable to the lowest-numbered object attached to the same parent object as target but with a higher object number than target.

    // ### Group Frame ###
    // These actions change the frame that is displayed for a group of cels.
    { FKK::A::Setframe         , { FKK::A::Setframe         , FkissVersion::FKiSS4, false } },   // setframe(group, frame) - The setframe action sets the current group frame number of group to frame.
    { FKK::A::Letframe         , { FKK::A::Letframe         , FkissVersion::FKiSS4, false } },   // letframe(variable, group) - The letframe action sets the value of variable to the current group frame number of group.

    // ### Arithmetic ###
    // These actions perform arithmetic operations on a variable.
    { FKK::A::Add              , { FKK::A::Add              , FkissVersion::FKiSS3, false } },   // add(variable, A, B) - The add action sets the value of variable to the value of (A + B).
    { FKK::A::Sub              , { FKK::A::Sub              , FkissVersion::FKiSS3, false } },   // sub(variable, A, B) - The sub action sets the value of variable to the value of (A - B).
    { FKK::A::Mul              , { FKK::A::Mul              , FkissVersion::FKiSS3, false } },   // mul(variable, A, B) - The mul action sets the value of variable to the value of (A x B).
    { FKK::A::Div              , { FKK::A::Div              , FkissVersion::FKiSS3, false } },   // div(variable, A, B) - The div action sets the value of variable to the value of (A / B).
    { FKK::A::Mod              , { FKK::A::Mod              , FkissVersion::FKiSS3, false } },   // mod(variable, A, B) - The mod action sets the value of variable to the value of (A modulo B).
    { FKK::A::Random           , { FKK::A::Random           , FkissVersion::FKiSS3, false } },   // random(variable, min, max) - The random action sets the value of variable to a random number in the range of min to max, inclusive.

    // ### Object/Cel State ###
    // These actions control or read the state of an object.
    { FKK::A::Setkcf           , { FKK::A::Setkcf           , FkissVersion::FKiSS4, false } },   // setkcf(target, N) - The setkcf action sets all cels in target to use the KCF N.
    { FKK::A::Letkcf           , { FKK::A::Letkcf           , FkissVersion::FKiSS4, false } },   // letkcf(variable, target) - The letkcf action sets the value of variable to the index of the KCF used by target.
    { FKK::A::Setfix           , { FKK::A::Setfix           , FkissVersion::FKiSS2, false } },   // setfix(target, N) - The setfix action sets the fix value for target to N.
    { FKK::A::Letfix           , { FKK::A::Letfix           , FkissVersion::FKiSS3, false } },   // letfix(variable, target) - The letfix action sets the value of variable to the fix value of target.
    { FKK::A::Ghost            , { FKK::A::Ghost            , FkissVersion::FKiSS3, false } },   // ghost(target, N) - The ghost action changes the ghost-state of target.
    { FKK::A::Letwidth         , { FKK::A::Letwidth         , FkissVersion::FKiSS4, false } },   // letwidth(variable, target) - The letwidth action sets the value of variable to the total width of object target, counting only cels which are mapped and present in the current set (page).
    { FKK::A::Letheight        , { FKK::A::Letheight        , FkissVersion::FKiSS4, false } },   // letheight(variable, target) - The letheight action sets the value of variable to the total height of object target, counting only cels which are mapped and present in the current set (page).
    { FKK::A::Letcollide       , { FKK::A::Letcollide       , FkissVersion::FKiSS3, false } },   // letcollide(variable, target1, target2) - The letcollide action makes a comparison based on the individual, non-transparent pixels of the two targets.
    { FKK::A::Letinside        , { FKK::A::Letinside        , FkissVersion::FKiSS3, false } },   // letinside(variable, target1, target2) - The letinside action makes a comparison based on the individual bounding rectangles of the two targets.

    // ### Conditionals ###
    // These actions control the flow of execution through a block of actions.
    { FKK::A::Ifequal          , { FKK::A::Ifequal          , FkissVersion::FKiSS3, false } },   // ifequal(A, B) - The ifequal begins a structured if-endif block. The conditional block is executed if A is equal to B.
    { FKK::A::Ifnotequal       , { FKK::A::Ifnotequal       , FkissVersion::FKiSS3, false } },   // ifnotequal(A, B) - The ifnotequal begins a structured if-endif block. The conditional block is executed if A is not equal to B.
    { FKK::A::Ifgreaterthan    , { FKK::A::Ifgreaterthan    , FkissVersion::FKiSS3, false } },   // ifgreaterthan(A, B) - The ifgreaterthan begins a structured if-endif block. The conditional block is executed if A is greater than B.
    { FKK::A::Iflessthan       , { FKK::A::Iflessthan       , FkissVersion::FKiSS3, false } },   // iflessthan(A, B) - The iflessthan begins a structured if-endif block. The conditional block is executed if A is less than B.
    { FKK::A::Else             , { FKK::A::Else             , FkissVersion::FKiSS3, false } },   // else() - The else forms the middle of a structured if-else-endif block.
    { FKK::A::Elseifequal      , { FKK::A::Elseifequal      , FkissVersion::FKiSS4, false } },   // elseifequal(A, B) - The elseifequal forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is equal to B.
    { FKK::A::Elseifnotequal   , { FKK::A::Elseifnotequal   , FkissVersion::FKiSS4, false } },   // elseifnotequal(A, B) - The elseifnotequal forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is not equal to B.
    { FKK::A::Elseifgreaterthan, { FKK::A::Elseifgreaterthan, FkissVersion::FKiSS4, false } },   // elseifgreaterthan(A, B) - The elseifgreaterthan forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is greater than B.
    { FKK::A::Elseiflessthan   , { FKK::A::Elseiflessthan   , FkissVersion::FKiSS4, false } },   // elseiflessthan(A, B) - The elseiflessthan forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is less than B.
    { FKK::A::Endif            , { FKK::A::Endif            , FkissVersion::FKiSS3, false } },   // endif() - The endif forms the end of a structured if-else-endif block.

    // ### Keyboard ###
    // These actions read the keyboard.
    { FKK::A::Letkey           , { FKK::A::Letkey           , FkissVersion::FKiSS4, false } },   // letkey(variable, keystring) - The letkey action sets the value of variable to the index of the first key in keystring which is pressed.
    { FKK::A::Letkeymap        , { FKK::A::Letkeymap        , FkissVersion::FKiSS4, false } },   // letkeymap(variable, keystring) - The letkeymap action sets each bit in variable if the corresponding key in keystring is pressed.

    // ### Mouse ###
    // These actions read the position of the mouse cursor.
    { FKK::A::Letmousex        , { FKK::A::Letmousex        , FkissVersion::FKiSS3, false } },   // letmousex(variable) - The letmousex action sets the value of variable to the current horizontal position of the mouse pointer.
    { FKK::A::Letmousey        , { FKK::A::Letmousey        , FkissVersion::FKiSS3, false } },   // letmousey(variable) - The letmousey action sets the value of variable to the current vertical position of the mouse pointer.

    // ### Environment ###
    // These actions affect the environment, set, or play field.
    { FKK::A::Changecol        , { FKK::A::Changecol        , FkissVersion::FKiSS1, 1 } },   // changecol(N) - The changecol action sets the palette for the current page to N.
    { FKK::A::Changeset        , { FKK::A::Changeset        , FkissVersion::FKiSS1, 1 } },   // changeset(N) - The changeset action changes to the set (page) specified by N.
    { FKK::A::Letpal           , { FKK::A::Letpal           , FkissVersion::FKiSS3, false } },   // letpal(variable) - The letpal action sets the value of variable to the current palette for the current set (page).
    { FKK::A::Letset           , { FKK::A::Letset           , FkissVersion::FKiSS3, false } },   // letset(variable) - The letset action sets the value of variable to the current set (page).

    // ### Viewport ###
    // These actions control the size of the window and view on that window.
    { FKK::A::Windowsize       , { FKK::A::Windowsize       , FkissVersion::FKiSS1, false } },   // windowsize(W, H) - The windowsize action sets the size of the viewport to <W x H>.
    { FKK::A::Viewport         , { FKK::A::Viewport         , FkissVersion::FKiSS1, false } },   // viewport(X, Y) - The viewport action sets the play field position of the top-left corner of the viewport to <X, Y>.

    // ### Miscellaneous ###
    // These actions fall in no particular category.
    { FKK::A::Nop              , { FKK::A::Nop              , FkissVersion::FKiSS1, false } },   // nop() - The nop action does nothing. It can be used as a placeholder in an event that may not have any actions in it.
    { FKK::A::Quit             , { FKK::A::Quit             , FkissVersion::FKiSS1b, false } },   // quit() - The quit action closes the KiSS doll.

    // ### Pool Values ###
    // These actions control access to value pools.
    { FKK::A::Valuepool        , { FKK::A::Valuepool        , FkissVersion::FKiSS4, false } },   // valuepool(pool) - The valuepool action sets the current value pool where data will be saved, loaded, or deleted.
    { FKK::A::Savevalue        , { FKK::A::Savevalue        , FkissVersion::FKiSS4, false } },   // savevalue(name, value) - The savevalue action saves a value to disk which can be loaded again later, even during future sessions.
    { FKK::A::Loadvalue        , { FKK::A::Loadvalue        , FkissVersion::FKiSS4, false } },   // loadvalue(variable, name) - The loadvalue action loads a value from disk which could have been saved earlier, even during previous sessions.
    { FKK::A::Deletevalue      , { FKK::A::Deletevalue      , FkissVersion::FKiSS4, false } },   // deletevalue(name) - The deletevalue action removes a named value from the current value pool.

    // ### Variables ###
    // These actions affect variables.
    { FKK::A::Let              , { FKK::A::Let              , FkissVersion::FKiSS3, false } }   // let(variable, A) - The let action sets the value of variable to the value of A.
};
