#include "fkiss.h"

const QHash<QString, FkissKeywordInfo> Fkiss::KEYWORDS_EVENT = {
    // ### Unique ###
    // Only one of each of these events can be defined in a single KiSS Artform.
    { FKK::INITIALIZE       , { FKK::INITIALIZE, FkissVersion::FKiSS1, false } }, // initialize() - Called exactly once before the artform is displayed on the screen for the first time.
    { FKK::BEGIN            , { FKK::BEGIN, FkissVersion::FKiSS1, false } }, // begin() - Called exactly once immediately after the artform is displayed on the screen for the first time.
    { FKK::END              , { FKK::END, FkissVersion::FKiSS1, false } }, // end() - Called exactly once immediately before the user quits the doll.
    { FKK::OVERFLOW         , { FKK::OVERFLOW, FkissVersion::FKiSS3, false } }, // overflow() - Called when a mathematical action attempts to divide by zero or when the result would be outside the range of a 32-bit integer.
    { FKK::NEVER            , { FKK::NEVER, FkissVersion::FKiSS1, false } }, // never() - Never called. Used to temporarily block segments of FKiSS code.

    // ### Specific ###
    // These events are specific to sets, colors, and version.
    { FKK::SET              , { FKK::SET, FkissVersion::FKiSS1, false } }, // set(page number) - Called when the viewer changes to the specified set (page).
    { FKK::COL              , { FKK::COL, FkissVersion::FKiSS1, false } }, // col(palette number) - Called when the viewer changes to the specified palette.
    { FKK::VERSION          , { FKK::VERSION, FkissVersion::FKiSS2, false } }, // version(version number) - Called immediately after the begin event if the specified version of FKiSS is supported.

    // ### Named ###
    // These events are associated with named blocks of code.
    { FKK::ALARM            , { FKK::ALARM, FkissVersion::FKiSS1, false } }, // alarm(alarm ID) - Called when the timer for a specified alarm reaches zero.
    { FKK::LABEL            , { FKK::LABEL, FkissVersion::FKiSS3, false } }, // label(label ID) - Defines a named block of code that can be called from other parts of the script.

    // ### Rectangle-based collision ###
    // These events are associated with collisions between rectangles.
    // Rectangle collision is faster than pixel-based collisions but not pixel accurate.
    { FKK::IN               , { FKK::IN, FkissVersion::FKiSS2, false } }, // in(target1, target2) - Triggered when target1 overlaps target2 and previously did not.
    { FKK::OUT              , { FKK::OUT, FkissVersion::FKiSS2, false } }, // out(target1, target2) - Triggered when target1 no longer overlaps target2 and previously did.

    { FKK::STILLIN          , { FKK::STILLIN, FkissVersion::FKiSS2, false } }, // stillin(target1, target2) - Triggered when target1 still overlaps target2, regardless of previous state.
    { FKK::STILLOUT         , { FKK::STILLOUT, FkissVersion::FKiSS2, false } }, // stillout(target1, target2) - Triggered when target1 still does not overlap target2, regardless of previous state.

    // ### Pixel-based collision ###
    // These events are associated with collisions between pixels of cels.
    { FKK::COLLIDE          , { FKK::COLLIDE, FkissVersion::FKiSS2, false } }, // collide(target1, target2) - Triggered when pixels of target1 overlap pixels of target2 and previously did not.
    { FKK::APART            , { FKK::APART, FkissVersion::FKiSS2, false } }, // apart(target1, target2) - Triggered when pixels of target1 no longer overlap pixels of target2 and previously did.

    // ### Mouse-based collision ###
    // These events are associated with the movement of the mouse cursor.
    { FKK::MOUSEIN          , { FKK::MOUSEIN, FkissVersion::FKiSS4, false } }, // mousein(target) - Triggered when the mouse passes into the bounding rectangle for target.
    { FKK::MOUSEOUT         , { FKK::MOUSEOUT, FkissVersion::FKiSS4, false } }, // mouseout(target) - Triggered when the mouse passes out of the bounding rectangle for target.

    // ### Keyboard ###
    // These events are associated with pressing of keys.
    { FKK::KEYPRESS         , { FKK::KEYPRESS, FkissVersion::FKiSS4, false } }, // keypress(keyname) - Triggered when the user presses the corresponding key on the keyboard.
    { FKK::KEYRELEASE       , { FKK::KEYRELEASE, FkissVersion::FKiSS4, false } }, // keyrelease(keyname) - Triggered when the user releases the corresponding key on the keyboard.

    // ### Click-and-drag ###
    // These events are associated with clicking on cels as well as clicking and dragging cels around.
    { FKK::PRESS            , { FKK::PRESS, FkissVersion::FKiSS1, false } }, // press(target) - Triggered when the user clicks on the target.
    { FKK::CATCH            , { FKK::CATCH, FkissVersion::FKiSS1, false } },   // catch(target) - The catch event is triggered when the user first begins to move the target.
    { FKK::DROP             , { FKK::DROP, FkissVersion::FKiSS1, false } },   // drop(target) - The drop event is triggered when the user loses control of the target after attempting to move it.

    { FKK::RELEASE          , { FKK::RELEASE, FkissVersion::FKiSS1, false } }, // release(target) - Triggered when the user releases the mouse button after having clicked on the target.
    { FKK::FIXCATCH         , { FKK::FIXCATCH, FkissVersion::FKiSS1, false } }, // fixcatch(target) - Triggered when the user first begins to move a target which has a non-zero fix value.
    { FKK::FIXDROP          , { FKK::FIXDROP, FkissVersion::FKiSS1, false } }, // fixdrop(target) - Triggered when the user loses control of the target after attempting to move it due to the fix value of the object.

    // ### Fix ###
    // These events are associated with the fix value of an object.
    { FKK::UNFIX            , { FKK::UNFIX, FkissVersion::FKiSS1, false } }, // unfix(target) - Triggered when the target's fix value reaches 0.

    // ### Attachment ###
    // This event is associated with detaching a target from a parent object.
    { FKK::DETACHED         , { FKK::DETACHED, FkissVersion::FKiSS4, false }} // detached(target) - Triggered when the target is detached from its parent object.
};


const QHash<QString, FkissKeywordInfo> Fkiss::KEYWORDS_ACTION = {
    // ### Alarms ###
    // These actions set up alarms, which are events triggered when certain conditions occur, such as a timer going off.
    { FKK::TIMER            , { FKK::TIMER, FkissVersion::FKiSS1, false } }, // timer(alarm ID, delay) - Sets the timer for an alarm.
    { FKK::RANDOMTIMER      , { FKK::RANDOMTIMER, FkissVersion::FKiSS1, false } }, // randomtimer(alarm ID, delay, range) - Sets the timer for an alarm with a random delay.
    { FKK::LETTIMER         , { FKK::LETTIMER, FkissVersion::FKiSS4, false } }, // lettimer(variable, alarm ID) - Sets the value of variable to the amount of time remaining before the timer for alarm ID reaches zero.
    { FKK::IFFIXED          , { FKK::IFFIXED, FkissVersion::FKiSS2, false } }, // iffixed(target, alarm ID, delay) - Sets the timer for an alarm if the fix value for the object specified by target is non-zero.
    { FKK::IFMAPPED         , { FKK::IFMAPPED, FkissVersion::FKiSS2, false } }, // ifmapped(target, alarm ID, delay) - Sets the timer for an alarm if any cel specified by target is currently mapped.
    { FKK::IFMOVED          , { FKK::IFMOVED, FkissVersion::FKiSS2, false } }, // ifmoved(target, alarm ID, delay) - Sets the timer for an alarm if the object specified by target is not in its initial position on the current set.
    { FKK::IFNOTFIXED       , { FKK::IFNOTFIXED, FkissVersion::FKiSS2, false } }, // ifnotfixed(target, alarm ID, delay) - Sets the timer for an alarm if the fix value for the object specified by target is zero.
    { FKK::IFNOTMAPPED      , { FKK::IFNOTMAPPED, FkissVersion::FKiSS2, false } }, // ifnotmapped(target, alarm ID, delay) - Sets the timer for an alarm if no cel or no cel in the object or group specified by target is currently mapped.
    { FKK::IFNOTMOVED       , { FKK::IFNOTMOVED, FkissVersion::FKiSS2, false } }, // ifnotmoved(target, alarm ID, delay) - Sets the timer for an alarm if the object specified by target is in its initial position on the current set.
    { FKK::EXITEVENT        , { FKK::EXITEVENT, FkissVersion::FKiSS3, false } }, // exitevent() - Exits the current event. No more code in this event will be processed.

    // ### Labels ###
    // These actions use labels that mark a block of actions to execute that block of actions from several different places.
    { FKK::GOSUB            , { FKK::GOSUB, FkissVersion::FKiSS3, false } }, // gosub(label ID) - Jumps to the specified label and returns when that label finishes.
    { FKK::GOSUBRANDOM      , { FKK::GOSUBRANDOM, FkissVersion::FKiSS3, false } }, // gosubrandom(chance, label ID A, label ID B) - Randomly jumps to one of two specified labels and returns when that label finishes.
    { FKK::GOTO             , { FKK::GOTO, FkissVersion::FKiSS3, false } }, // goto(label ID) - Jumps to the specified label without returning.
    { FKK::GOTORANDOM       , { FKK::GOTORANDOM, FkissVersion::FKiSS3, false } }, // gotorandom(chance, label ID A, label ID B) - Randomly jumps to one of two specified labels without returning.
    { FKK::REPEAT           , { FKK::REPEAT, FkissVersion::FKiSS4, false } }, // repeat(label ID, times, count) - Calls the specified label a given number of times.
    { FKK::EXITLOOP         , { FKK::EXITLOOP, FkissVersion::FKiSS4, false } }, // exitloop() - Exits the current event and terminates the current repeat() loop.

    // ### Mapping ###
    // These actions control the visibility of a cel. A mapped cel is visible while an unmapped cel is not visible.
    { FKK::MAP              , { FKK::MAP, FkissVersion::FKiSS1, false } }, // map(target) - Makes target visible on all sets in which it is present.
    { FKK::UNMAP            , { FKK::UNMAP, FkissVersion::FKiSS1, false } }, // unmap(target) - Makes target invisible on all sets in which it is present.
    { FKK::ALTMAP           , { FKK::ALTMAP, FkissVersion::FKiSS1, false } }, // altmap(target) - Toggles the visibility of target on all sets in which it is present.
    { FKK::LETMAPPED        , { FKK::LETMAPPED, FkissVersion::FKiSS3, false } }, // letmapped(variable, target) - Sets the value of variable to 1 if target is mapped or to 0 if it is unmapped.

    // ### Movement and Position ###
    // These actions control the position of a target and allow an action to move a cel in response to an event.
    { FKK::MOVE             , { FKK::MOVE, FkissVersion::FKiSS1, false } }, // move(target, X, Y) - Moves target X pixels horizontally and Y pixels vertically relative to its current position.
    { FKK::MOVETO           , { FKK::MOVETO, FkissVersion::FKiSS2, false } }, // moveto(target, X, Y) - Moves target to absolute play field position <X, Y>.
    { FKK::MOVEBYX          , { FKK::MOVEBYX, FkissVersion::FKiSS2, false } }, // movebyx(target, relative, X) - Moves target to a position X pixels horizontally from the position of object relative.
    { FKK::MOVEBYY          , { FKK::MOVEBYY, FkissVersion::FKiSS2, false } }, // movebyy(target, relative, Y) - Moves target to a position Y pixels vertically from the position of object relative.
    { FKK::MOVETORAND       , { FKK::MOVETORAND, FkissVersion::FKiSS2, false } }, // movetorand(target) - Moves target to a random position within the play field or its restricted range of movement.
    { FKK::MOVERANDX        , { FKK::MOVERANDX, FkissVersion::FKiSS2, false } }, // moverandx(target, min, max) - Moves target by a random horizontal distance in the range min to max relative to its current position.
    { FKK::MOVERANDY        , { FKK::MOVERANDY, FkissVersion::FKiSS2, false } }, // moverandy(target, min, max) - Moves target by a random vertical distance in the range min to max relative to its current position.
    { FKK::RESTRICTX        , { FKK::RESTRICTX, FkissVersion::FKiSS4, false } }, // restrictx(target, min, max) - Changes the horizontal movement limits of target to the absolute range of min to max.
    { FKK::RESTRICTY        , { FKK::RESTRICTY, FkissVersion::FKiSS4, false } }, // restricty(target, min, max) - Changes the vertical movement limits of target to the absolute range min to max.
    { FKK::LETINITX         , { FKK::LETINITX, FkissVersion::FKiSS4, false } }, // letinitx(variable, target) - Sets the value of variable to the initial horizontal position of target for the current set.
    { FKK::LETINITY         , { FKK::LETINITY, FkissVersion::FKiSS4, false } }, // letinity(variable, target) - Sets the value of variable to the initial vertical position of target for the current set.
    { FKK::LETOBJECTX       , { FKK::LETOBJECTX, FkissVersion::FKiSS3, false } }, // letobjectx(variable, target) - Sets the value of variable to the current horizontal position of target for the current set.
    { FKK::LETOBJECTY       , { FKK::LETOBJECTY, FkissVersion::FKiSS3, false } }, // letobjecty(variable, target) - Sets the value of variable to the current vertical position of target for the current set.
    { FKK::LETCATCH         , { FKK::LETCATCH, FkissVersion::FKiSS3, false } }, // letcatch(variable) - Sets the value of variable to number of the object currently being dragged by the user.

    // ### Message ###
    // These actions show messages to the user or trigger a shell command.
    { FKK::DEBUG            , { FKK::DEBUG, FkissVersion::FKiSS1, false } }, // debug(string) - Sends the string to debug output for that KiSS viewer.
    { FKK::NOTIFY           , { FKK::NOTIFY, FkissVersion::FKiSS2, false } }, // notify(string) - Sends the string as a message to the user.
    { FKK::SHELL            , { FKK::SHELL, FkissVersion::FKiSS1, false } }, // shell(string) - Sends the string as a shell command to the system.

    // ### Sound and Music ###
    // These actions control music and sound.
    { FKK::MUSIC            , { FKK::MUSIC, FkissVersion::FKiSS2, false } }, // music(filename) - Begins playback of the MIDI filename.
    { FKK::SOUND            , { FKK::SOUND, FkissVersion::FKiSS1, false } }, // sound(filename) - Begins playback of sound file filename.

    // ### Translucency ###
    // These actions control the translucency of a target.
    { FKK::TRANSPARENT      , { FKK::TRANSPARENT, FkissVersion::FKiSS1b, false } },   // transparent(target, N) - The transparent action adds N to the translucency value of target.
    { FKK::LETTRANSPARENT   , { FKK::LETTRANSPARENT, FkissVersion::FKiSS3, false } },   // lettransparent(variable, target) - The lettransparent action sets the value of variable to the translucency value of target.

    // ### Attachment ###
    // These actions control the process of attaching or detaching an object to another object.
    { FKK::ATTACH           , { FKK::ATTACH, FkissVersion::FKiSS4, false } },   // attach(target, parent) - The attach action attaches target to parent, becoming a child object of parent.
    { FKK::GLUE             , { FKK::GLUE, FkissVersion::FKiSS4, false } },   // glue(target, parent) - The glue action attaches target to parent, becoming a child object of parent that can only be detached by the detach action.
    { FKK::DETACH           , { FKK::DETACH, FkissVersion::FKiSS4, false } },   // detach(target) - The detach action detaches target from its parent object.
    { FKK::LETCHILD         , { FKK::LETCHILD, FkissVersion::FKiSS4, false } },   // letchild(variable, target) - The letchild action sets the value of variable to the lowest-numbered attached child object for target.
    { FKK::LETPARENT        , { FKK::LETPARENT, FkissVersion::FKiSS4, false } },   // letparent(variable, target) - The letparent action sets the value of variable to the current attached parent object for target.
    { FKK::LETSIBLING       , { FKK::LETSIBLING, FkissVersion::FKiSS4, false } },   // letsibling(variable, target) - The letsibling action sets the value of variable to the lowest-numbered object attached to the same parent object as target but with a higher object number than target.

    // ### Group Frame ###
    // These actions change the frame that is displayed for a group of cels.
    { FKK::SETFRAME         , { FKK::SETFRAME, FkissVersion::FKiSS4, false } },   // setframe(group, frame) - The setframe action sets the current group frame number of group to frame.
    { FKK::LETFRAME         , { FKK::LETFRAME, FkissVersion::FKiSS4, false } },   // letframe(variable, group) - The letframe action sets the value of variable to the current group frame number of group.

    // ### Arithmetic ###
    // These actions perform arithmetic operations on a variable.
    { FKK::ADD              , { FKK::ADD, FkissVersion::FKiSS3, false } },   // add(variable, A, B) - The add action sets the value of variable to the value of (A + B).
    { FKK::SUB              , { FKK::SUB, FkissVersion::FKiSS3, false } },   // sub(variable, A, B) - The sub action sets the value of variable to the value of (A - B).
    { FKK::MUL              , { FKK::MUL, FkissVersion::FKiSS3, false } },   // mul(variable, A, B) - The mul action sets the value of variable to the value of (A x B).
    { FKK::DIV              , { FKK::DIV, FkissVersion::FKiSS3, false } },   // div(variable, A, B) - The div action sets the value of variable to the value of (A / B).
    { FKK::MOD              , { FKK::MOD, FkissVersion::FKiSS3, false } },   // mod(variable, A, B) - The mod action sets the value of variable to the value of (A modulo B).
    { FKK::RANDOM           , { FKK::RANDOM, FkissVersion::FKiSS3, false } },   // random(variable, min, max) - The random action sets the value of variable to a random number in the range of min to max, inclusive.

    // ### Object/Cel State ###
    // These actions control or read the state of an object.
    { FKK::SETKCF           , { FKK::SETKCF, FkissVersion::FKiSS4, false } },   // setkcf(target, N) - The setkcf action sets all cels in target to use the KCF N.
    { FKK::LETKCF           , { FKK::LETKCF, FkissVersion::FKiSS4, false } },   // letkcf(variable, target) - The letkcf action sets the value of variable to the index of the KCF used by target.
    { FKK::SETFIX           , { FKK::SETFIX, FkissVersion::FKiSS2, false } },   // setfix(target, N) - The setfix action sets the fix value for target to N.
    { FKK::LETFIX           , { FKK::LETFIX, FkissVersion::FKiSS3, false } },   // letfix(variable, target) - The letfix action sets the value of variable to the fix value of target.
    { FKK::GHOST            , { FKK::GHOST, FkissVersion::FKiSS3, false } },   // ghost(target, N) - The ghost action changes the ghost-state of target.
    { FKK::LETWIDTH         , { FKK::LETWIDTH, FkissVersion::FKiSS4, false } },   // letwidth(variable, target) - The letwidth action sets the value of variable to the total width of object target, counting only cels which are mapped and present in the current set (page).
    { FKK::LETHEIGHT        , { FKK::LETHEIGHT, FkissVersion::FKiSS4, false } },   // letheight(variable, target) - The letheight action sets the value of variable to the total height of object target, counting only cels which are mapped and present in the current set (page).
    { FKK::LETCOLLIDE       , { FKK::LETCOLLIDE, FkissVersion::FKiSS3, false } },   // letcollide(variable, target1, target2) - The letcollide action makes a comparison based on the individual, non-transparent pixels of the two targets.
    { FKK::LETINSIDE        , { FKK::LETINSIDE, FkissVersion::FKiSS3, false } },   // letinside(variable, target1, target2) - The letinside action makes a comparison based on the individual bounding rectangles of the two targets.

    // ### Conditionals ###
    // These actions control the flow of execution through a block of actions.
    { FKK::IFEQUAL          , { FKK::IFEQUAL, FkissVersion::FKiSS3, false } },   // ifequal(A, B) - The ifequal begins a structured if-endif block. The conditional block is executed if A is equal to B.
    { FKK::IFNOTEQUAL       , { FKK::IFNOTEQUAL, FkissVersion::FKiSS3, false } },   // ifnotequal(A, B) - The ifnotequal begins a structured if-endif block. The conditional block is executed if A is not equal to B.
    { FKK::IFGREATERTHAN    , { FKK::IFGREATERTHAN, FkissVersion::FKiSS3, false } },   // ifgreaterthan(A, B) - The ifgreaterthan begins a structured if-endif block. The conditional block is executed if A is greater than B.
    { FKK::IFLESSTHAN       , { FKK::IFLESSTHAN, FkissVersion::FKiSS3, false } },   // iflessthan(A, B) - The iflessthan begins a structured if-endif block. The conditional block is executed if A is less than B.
    { FKK::ELSE             , { FKK::ELSE, FkissVersion::FKiSS3, false } },   // else() - The else forms the middle of a structured if-else-endif block.
    { FKK::ELSEIFEQUAL      , { FKK::ELSEIFEQUAL, FkissVersion::FKiSS4, false } },   // elseifequal(A, B) - The elseifequal forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is equal to B.
    { FKK::ELSEIFNOTEQUAL   , { FKK::ELSEIFNOTEQUAL, FkissVersion::FKiSS4, false } },   // elseifnotequal(A, B) - The elseifnotequal forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is not equal to B.
    { FKK::ELSEIFGREATERTHAN, { FKK::ELSEIFGREATERTHAN, FkissVersion::FKiSS4, false } },   // elseifgreaterthan(A, B) - The elseifgreaterthan forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is greater than B.
    { FKK::ELSEIFLESSTHAN   , { FKK::ELSEIFLESSTHAN, FkissVersion::FKiSS4, false } },   // elseiflessthan(A, B) - The elseiflessthan forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is less than B.
    { FKK::ENDIF            , { FKK::ENDIF, FkissVersion::FKiSS3, false } },   // endif() - The endif forms the end of a structured if-else-endif block.

    // ### Keyboard ###
    // These actions read the keyboard.
    { FKK::LETKEY           , { FKK::LETKEY, FkissVersion::FKiSS4, false } },   // letkey(variable, keystring) - The letkey action sets the value of variable to the index of the first key in keystring which is pressed.
    { FKK::LETKEYMAP        , { FKK::LETKEYMAP, FkissVersion::FKiSS4, false } },   // letkeymap(variable, keystring) - The letkeymap action sets each bit in variable if the corresponding key in keystring is pressed.

    // ### Mouse ###
    // These actions read the position of the mouse cursor.
    { FKK::LETMOUSEX        , { FKK::LETMOUSEX, FkissVersion::FKiSS3, false } },   // letmousex(variable) - The letmousex action sets the value of variable to the current horizontal position of the mouse pointer.
    { FKK::LETMOUSEY        , { FKK::LETMOUSEY, FkissVersion::FKiSS3, false } },   // letmousey(variable) - The letmousey action sets the value of variable to the current vertical position of the mouse pointer.

    // ### Environment ###
    // These actions affect the environment, set, or play field.
    { FKK::CHANGECOL        , { FKK::CHANGECOL, FkissVersion::FKiSS1, false } },   // changecol(N) - The changecol action sets the palette for the current page to N.
    { FKK::CHANGESET        , { FKK::CHANGESET, FkissVersion::FKiSS1, false } },   // changeset(N) - The changeset action changes to the set (page) specified by N.
    { FKK::LETPAL           , { FKK::LETPAL, FkissVersion::FKiSS3, false } },   // letpal(variable) - The letpal action sets the value of variable to the current palette for the current set (page).
    { FKK::LETSET           , { FKK::LETSET, FkissVersion::FKiSS3, false } },   // letset(variable) - The letset action sets the value of variable to the current set (page).

    // ### Viewport ###
    // These actions control the size of the window and view on that window.
    { FKK::WINDOWSIZE       , { FKK::WINDOWSIZE, FkissVersion::FKiSS1, false } },   // windowsize(W, H) - The windowsize action sets the size of the viewport to <W x H>.
    { FKK::VIEWPORT         , { FKK::VIEWPORT, FkissVersion::FKiSS1, false } },   // viewport(X, Y) - The viewport action sets the play field position of the top-left corner of the viewport to <X, Y>.

    // ### Miscellaneous ###
    // These actions fall in no particular category.
    { FKK::NOP              , { FKK::NOP, FkissVersion::FKiSS1, false } },   // nop() - The nop action does nothing. It can be used as a placeholder in an event that may not have any actions in it.
    { FKK::QUIT             , { FKK::QUIT, FkissVersion::FKiSS1b, false } },   // quit() - The quit action closes the KiSS doll.

    // ### Pool Values ###
    // These actions control access to value pools.
    { FKK::VALUEPOOL        , { FKK::VALUEPOOL, FkissVersion::FKiSS4, false } },   // valuepool(pool) - The valuepool action sets the current value pool where data will be saved, loaded, or deleted.
    { FKK::SAVEVALUE        , { FKK::SAVEVALUE, FkissVersion::FKiSS4, false } },   // savevalue(name, value) - The savevalue action saves a value to disk which can be loaded again later, even during future sessions.
    { FKK::LOADVALUE        , { FKK::LOADVALUE, FkissVersion::FKiSS4, false } },   // loadvalue(variable, name) - The loadvalue action loads a value from disk which could have been saved earlier, even during previous sessions.
    { FKK::DELETEVALUE      , { FKK::DELETEVALUE, FkissVersion::FKiSS4, false } },   // deletevalue(name) - The deletevalue action removes a named value from the current value pool.

    // ### Variables ###
    // These actions affect variables.
    { FKK::LET              , { FKK::LET, FkissVersion::FKiSS3, false } }   // let(variable, A) - The let action sets the value of variable to the value of A.
};
