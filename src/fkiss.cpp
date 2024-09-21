#include "fkiss.h"
#include <QRegularExpression>

const QHash<QString, FkissKeywordInfo> Fkiss::KEYWORDS_EVENT = {
    // ### Unique ###
    // Only one of each of these events can be defined in a single KiSS Artform.
    { "initialize", { "initialize", FkissVersion::FKiSS1, false } }, // initialize() - Called exactly once before the artform is displayed on the screen for the first time.
    { "begin", { "begin", FkissVersion::FKiSS1, false } }, // begin() - Called exactly once immediately after the artform is displayed on the screen for the first time.
    { "end", { "end", FkissVersion::FKiSS1, false } }, // end() - Called exactly once immediately before the user quits the doll.
    { "overflow", { "overflow", FkissVersion::FKiSS3, false } }, // overflow() - Called when a mathematical action attempts to divide by zero or when the result would be outside the range of a 32-bit integer.
    { "never", { "never", FkissVersion::FKiSS1, false } }, // never() - Never called. Used to temporarily block segments of FKiSS code.

    // ### Specific ###
    // These events are specific to sets, colors, and version.
    { "set", { "set", FkissVersion::FKiSS1, false } }, // set(page number) - Called when the viewer changes to the specified set (page).
    { "col", { "col", FkissVersion::FKiSS1, false } }, // col(palette number) - Called when the viewer changes to the specified palette.
    { "version", { "version", FkissVersion::FKiSS2, false } }, // version(version number) - Called immediately after the begin event if the specified version of FKiSS is supported.

    // ### Named ###
    // These events are associated with named blocks of code.
    { "alarm", { "alarm", FkissVersion::FKiSS1, false } }, // alarm(alarm ID) - Called when the timer for a specified alarm reaches zero.
    { "label", { "label", FkissVersion::FKiSS3, false } }, // label(label ID) - Defines a named block of code that can be called from other parts of the script.

    // ### Rectangle-based collision ###
    // These events are associated with collisions between rectangles.
    // Rectangle collision is faster than pixel-based collisions but not pixel accurate.
    { "in", { "in", FkissVersion::FKiSS2, false } }, // in(target1, target2) - Triggered when target1 overlaps target2 and previously did not.
    { "out", { "out", FkissVersion::FKiSS2, false } }, // out(target1, target2) - Triggered when target1 no longer overlaps target2 and previously did.

    { "stillin", { "stillin", FkissVersion::FKiSS2, false } }, // stillin(target1, target2) - Triggered when target1 still overlaps target2, regardless of previous state.
    { "stillout", { "stillout", FkissVersion::FKiSS2, false } }, // stillout(target1, target2) - Triggered when target1 still does not overlap target2, regardless of previous state.

    // ### Pixel-based collision ###
    // These events are associated with collisions between pixels of cels.
    { "collide", { "collide", FkissVersion::FKiSS2, false } }, // collide(target1, target2) - Triggered when pixels of target1 overlap pixels of target2 and previously did not.
    { "apart", { "apart", FkissVersion::FKiSS2, false } }, // apart(target1, target2) - Triggered when pixels of target1 no longer overlap pixels of target2 and previously did.

    // ### Mouse-based collision ###
    // These events are associated with the movement of the mouse cursor.
    { "mousein", { "mousein", FkissVersion::FKiSS4, false } }, // mousein(target) - Triggered when the mouse passes into the bounding rectangle for target.
    { "mouseout", { "mouseout", FkissVersion::FKiSS4, false } }, // mouseout(target) - Triggered when the mouse passes out of the bounding rectangle for target.

    // ### Keyboard ###
    // These events are associated with pressing of keys.
    { "keypress", { "keypress", FkissVersion::FKiSS4, false } }, // keypress(keyname) - Triggered when the user presses the corresponding key on the keyboard.
    { "keyrelease", { "keyrelease", FkissVersion::FKiSS4, false } }, // keyrelease(keyname) - Triggered when the user releases the corresponding key on the keyboard.

    // ### Click-and-drag ###
    // These events are associated with clicking on cels as well as clicking and dragging cels around.
    { "press", { "press", FkissVersion::FKiSS1, false } }, // press(target) - Triggered when the user clicks on the target.
    { "catch", { "catch", FkissVersion::FKiSS1, false } },   // catch(target) - The catch event is triggered when the user first begins to move the target.
    { "drop", { "drop", FkissVersion::FKiSS1, false } },   // drop(target) - The drop event is triggered when the user loses control of the target after attempting to move it.

    { "release", { "release", FkissVersion::FKiSS1, false } }, // release(target) - Triggered when the user releases the mouse button after having clicked on the target.
    { "fixcatch", { "fixcatch", FkissVersion::FKiSS1, false } }, // fixcatch(target) - Triggered when the user first begins to move a target which has a non-zero fix value.
    { "fixdrop", { "fixdrop", FkissVersion::FKiSS1, false } }, // fixdrop(target) - Triggered when the user loses control of the target after attempting to move it due to the fix value of the object.

    // ### Fix ###
    // These events are associated with the fix value of an object.
    { "unfix", { "unfix", FkissVersion::FKiSS1, false } }, // unfix(target) - Triggered when the target's fix value reaches 0.

    // ### Attachment ###
    // This event is associated with detaching a target from a parent object.
    { "detached", { "detached", FkissVersion::FKiSS4, false }} // detached(target) - Triggered when the target is detached from its parent object.
};


const QHash<QString, FkissKeywordInfo> Fkiss::KEYWORDS_ACTION = {
    // ### Alarms ###
    // These actions set up alarms, which are events triggered when certain conditions occur, such as a timer going off.
    { "timer", { "timer", FkissVersion::FKiSS1, false } }, // timer(alarm ID, delay) - Sets the timer for an alarm.
    { "randomtimer", { "randomtimer", FkissVersion::FKiSS1, false } }, // randomtimer(alarm ID, delay, range) - Sets the timer for an alarm with a random delay.
    { "lettimer", { "lettimer", FkissVersion::FKiSS4, false } }, // lettimer(variable, alarm ID) - Sets the value of variable to the amount of time remaining before the timer for alarm ID reaches zero.
    { "iffixed", { "iffixed", FkissVersion::FKiSS2, false } }, // iffixed(target, alarm ID, delay) - Sets the timer for an alarm if the fix value for the object specified by target is non-zero.
    { "ifmapped", { "ifmapped", FkissVersion::FKiSS2, false } }, // ifmapped(target, alarm ID, delay) - Sets the timer for an alarm if any cel specified by target is currently mapped.
    { "ifmoved", { "ifmoved", FkissVersion::FKiSS2, false } }, // ifmoved(target, alarm ID, delay) - Sets the timer for an alarm if the object specified by target is not in its initial position on the current set.
    { "ifnotfixed", { "ifnotfixed", FkissVersion::FKiSS2, false } }, // ifnotfixed(target, alarm ID, delay) - Sets the timer for an alarm if the fix value for the object specified by target is zero.
    { "ifnotmapped", { "ifnotmapped", FkissVersion::FKiSS2, false } }, // ifnotmapped(target, alarm ID, delay) - Sets the timer for an alarm if no cel or no cel in the object or group specified by target is currently mapped.
    { "ifnotmoved", { "ifnotmoved", FkissVersion::FKiSS2, false } }, // ifnotmoved(target, alarm ID, delay) - Sets the timer for an alarm if the object specified by target is in its initial position on the current set.
    { "exitevent", { "exitevent", FkissVersion::FKiSS3, false } }, // exitevent() - Exits the current event. No more code in this event will be processed.

    // ### Labels ###
    // These actions use labels that mark a block of actions to execute that block of actions from several different places.
    { "gosub", { "gosub", FkissVersion::FKiSS3, false } }, // gosub(label ID) - Jumps to the specified label and returns when that label finishes.
    { "gosubrandom", { "gosubrandom", FkissVersion::FKiSS3, false } }, // gosubrandom(chance, label ID A, label ID B) - Randomly jumps to one of two specified labels and returns when that label finishes.
    { "goto", { "goto", FkissVersion::FKiSS3, false } }, // goto(label ID) - Jumps to the specified label without returning.
    { "gotorandom", { "gotorandom", FkissVersion::FKiSS3, false } }, // gotorandom(chance, label ID A, label ID B) - Randomly jumps to one of two specified labels without returning.
    { "repeat", { "repeat", FkissVersion::FKiSS4, false } }, // repeat(label ID, times, count) - Calls the specified label a given number of times.
    { "exitloop", { "exitloop", FkissVersion::FKiSS4, false } }, // exitloop() - Exits the current event and terminates the current repeat() loop.

    // ### Mapping ###
    // These actions control the visibility of a cel. A mapped cel is visible while an unmapped cel is not visible.
    { "map", { "map", FkissVersion::FKiSS1, false } }, // map(target) - Makes target visible on all sets in which it is present.
    { "unmap", { "unmap", FkissVersion::FKiSS1, false } }, // unmap(target) - Makes target invisible on all sets in which it is present.
    { "altmap", { "altmap", FkissVersion::FKiSS1, false } }, // altmap(target) - Toggles the visibility of target on all sets in which it is present.
    { "letmapped", { "letmapped", FkissVersion::FKiSS3, false } }, // letmapped(variable, target) - Sets the value of variable to 1 if target is mapped or to 0 if it is unmapped.

    // ### Movement and Position ###
    // These actions control the position of a target and allow an action to move a cel in response to an event.
    { "move", { "move", FkissVersion::FKiSS1, false } }, // move(target, X, Y) - Moves target X pixels horizontally and Y pixels vertically relative to its current position.
    { "moveto", { "moveto", FkissVersion::FKiSS2, false } }, // moveto(target, X, Y) - Moves target to absolute play field position <X, Y>.
    { "movebyx", { "movebyx", FkissVersion::FKiSS2, false } }, // movebyx(target, relative, X) - Moves target to a position X pixels horizontally from the position of object relative.
    { "movebyy", { "movebyy", FkissVersion::FKiSS2, false } }, // movebyy(target, relative, Y) - Moves target to a position Y pixels vertically from the position of object relative.
    { "movetorand", { "movetorand", FkissVersion::FKiSS2, false } }, // movetorand(target) - Moves target to a random position within the play field or its restricted range of movement.
    { "moverandx", { "moverandx", FkissVersion::FKiSS2, false } }, // moverandx(target, min, max) - Moves target by a random horizontal distance in the range min to max relative to its current position.
    { "moverandy", { "moverandy", FkissVersion::FKiSS2, false } }, // moverandy(target, min, max) - Moves target by a random vertical distance in the range min to max relative to its current position.
    { "restrictx", { "restrictx", FkissVersion::FKiSS4, false } }, // restrictx(target, min, max) - Changes the horizontal movement limits of target to the absolute range of min to max.
    { "restricty", { "restricty", FkissVersion::FKiSS4, false } }, // restricty(target, min, max) - Changes the vertical movement limits of target to the absolute range min to max.
    { "letinitx", { "letinitx", FkissVersion::FKiSS4, false } }, // letinitx(variable, target) - Sets the value of variable to the initial horizontal position of target for the current set.
    { "letinity", { "letinity", FkissVersion::FKiSS4, false } }, // letinity(variable, target) - Sets the value of variable to the initial vertical position of target for the current set.
    { "letobjectx", { "letobjectx", FkissVersion::FKiSS3, false } }, // letobjectx(variable, target) - Sets the value of variable to the current horizontal position of target for the current set.
    { "letobjecty", { "letobjecty", FkissVersion::FKiSS3, false } }, // letobjecty(variable, target) - Sets the value of variable to the current vertical position of target for the current set.
    { "letcatch", { "letcatch", FkissVersion::FKiSS3, false } }, // letcatch(variable) - Sets the value of variable to number of the object currently being dragged by the user.

    // ### Message ###
    // These actions show messages to the user or trigger a shell command.
    { "debug", { "debug", FkissVersion::FKiSS1, false } }, // debug(string) - Sends the string to debug output for that KiSS viewer.
    { "notify", { "notify", FkissVersion::FKiSS2, false } }, // notify(string) - Sends the string as a message to the user.
    { "shell", { "shell", FkissVersion::FKiSS1, false } }, // shell(string) - Sends the string as a shell command to the system.

    // ### Sound and Music ###
    // These actions control music and sound.
    { "music", { "music", FkissVersion::FKiSS2, false } }, // music(filename) - Begins playback of the MIDI filename.
    { "sound", { "sound", FkissVersion::FKiSS1, false } }, // sound(filename) - Begins playback of sound file filename.

    // ### Translucency ###
    // These actions control the translucency of a target.
    { "transparent", { "transparent", FkissVersion::FKiSS1b, false } },   // transparent(target, N) - The transparent action adds N to the translucency value of target.
    { "lettransparent", { "lettransparent", FkissVersion::FKiSS3, false } },   // lettransparent(variable, target) - The lettransparent action sets the value of variable to the translucency value of target.

    // ### Attachment ###
    // These actions control the process of attaching or detaching an object to another object.
    { "attach", { "attach", FkissVersion::FKiSS4, false } },   // attach(target, parent) - The attach action attaches target to parent, becoming a child object of parent.
    { "glue", { "glue", FkissVersion::FKiSS4, false } },   // glue(target, parent) - The glue action attaches target to parent, becoming a child object of parent that can only be detached by the detach action.
    { "detach", { "detach", FkissVersion::FKiSS4, false } },   // detach(target) - The detach action detaches target from its parent object.
    { "letchild", { "letchild", FkissVersion::FKiSS4, false } },   // letchild(variable, target) - The letchild action sets the value of variable to the lowest-numbered attached child object for target.
    { "letparent", { "letparent", FkissVersion::FKiSS4, false } },   // letparent(variable, target) - The letparent action sets the value of variable to the current attached parent object for target.
    { "letsibling", { "letsibling", FkissVersion::FKiSS4, false } },   // letsibling(variable, target) - The letsibling action sets the value of variable to the lowest-numbered object attached to the same parent object as target but with a higher object number than target.

    // ### Group Frame ###
    // These actions change the frame that is displayed for a group of cels.
    { "setframe", { "setframe", FkissVersion::FKiSS4, false } },   // setframe(group, frame) - The setframe action sets the current group frame number of group to frame.
    { "letframe", { "letframe", FkissVersion::FKiSS4, false } },   // letframe(variable, group) - The letframe action sets the value of variable to the current group frame number of group.

    // ### Arithmetic ###
    // These actions perform arithmetic operations on a variable.
    { "add", { "add", FkissVersion::FKiSS3, false } },   // add(variable, A, B) - The add action sets the value of variable to the value of (A + B).
    { "sub", { "sub", FkissVersion::FKiSS3, false } },   // sub(variable, A, B) - The sub action sets the value of variable to the value of (A - B).
    { "mul", { "mul", FkissVersion::FKiSS3, false } },   // mul(variable, A, B) - The mul action sets the value of variable to the value of (A x B).
    { "div", { "div", FkissVersion::FKiSS3, false } },   // div(variable, A, B) - The div action sets the value of variable to the value of (A / B).
    { "mod", { "mod", FkissVersion::FKiSS3, false } },   // mod(variable, A, B) - The mod action sets the value of variable to the value of (A modulo B).
    { "random", { "random", FkissVersion::FKiSS3, false } },   // random(variable, min, max) - The random action sets the value of variable to a random number in the range of min to max, inclusive.

    // ### Object/Cel State ###
    // These actions control or read the state of an object.
    { "setkcf", { "setkcf", FkissVersion::FKiSS4, false } },   // setkcf(target, N) - The setkcf action sets all cels in target to use the KCF N.
    { "letkcf", { "letkcf", FkissVersion::FKiSS4, false } },   // letkcf(variable, target) - The letkcf action sets the value of variable to the index of the KCF used by target.
    { "setfix", { "setfix", FkissVersion::FKiSS2, false } },   // setfix(target, N) - The setfix action sets the fix value for target to N.
    { "letfix", { "letfix", FkissVersion::FKiSS3, false } },   // letfix(variable, target) - The letfix action sets the value of variable to the fix value of target.
    { "ghost", { "ghost", FkissVersion::FKiSS3, false } },   // ghost(target, N) - The ghost action changes the ghost-state of target.
    { "letwidth", { "letwidth", FkissVersion::FKiSS4, false } },   // letwidth(variable, target) - The letwidth action sets the value of variable to the total width of object target, counting only cels which are mapped and present in the current set (page).
    { "letheight", { "letheight", FkissVersion::FKiSS4, false } },   // letheight(variable, target) - The letheight action sets the value of variable to the total height of object target, counting only cels which are mapped and present in the current set (page).
    { "letcollide", { "letcollide", FkissVersion::FKiSS3, false } },   // letcollide(variable, target1, target2) - The letcollide action makes a comparison based on the individual, non-transparent pixels of the two targets.
    { "letinside", { "letinside", FkissVersion::FKiSS3, false } },   // letinside(variable, target1, target2) - The letinside action makes a comparison based on the individual bounding rectangles of the two targets.

    // ### Conditionals ###
    // These actions control the flow of execution through a block of actions.
    { "ifequal", { "ifequal", FkissVersion::FKiSS3, false } },   // ifequal(A, B) - The ifequal begins a structured if-endif block. The conditional block is executed if A is equal to B.
    { "ifnotequal", { "ifnotequal", FkissVersion::FKiSS3, false } },   // ifnotequal(A, B) - The ifnotequal begins a structured if-endif block. The conditional block is executed if A is not equal to B.
    { "ifgreaterthan", { "ifgreaterthan", FkissVersion::FKiSS3, false } },   // ifgreaterthan(A, B) - The ifgreaterthan begins a structured if-endif block. The conditional block is executed if A is greater than B.
    { "iflessthan", { "iflessthan", FkissVersion::FKiSS3, false } },   // iflessthan(A, B) - The iflessthan begins a structured if-endif block. The conditional block is executed if A is less than B.
    { "else", { "else", FkissVersion::FKiSS3, false } },   // else() - The else forms the middle of a structured if-else-endif block.
    { "elseifequal", { "elseifequal", FkissVersion::FKiSS4, false } },   // elseifequal(A, B) - The elseifequal forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is equal to B.
    { "elseifnotequal", { "elseifnotequal", FkissVersion::FKiSS4, false } },   // elseifnotequal(A, B) - The elseifnotequal forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is not equal to B.
    { "elseifgreaterthan", { "elseifgreaterthan", FkissVersion::FKiSS4, false } },   // elseifgreaterthan(A, B) - The elseifgreaterthan forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is greater than B.
    { "elseiflessthan", { "elseiflessthan", FkissVersion::FKiSS4, false } },   // elseiflessthan(A, B) - The elseiflessthan forms the middle of a structured if-elseif-endif block. The conditional block is executed if all previous conditional blocks were false and A is less than B.
    { "endif", { "endif", FkissVersion::FKiSS3, false } },   // endif() - The endif forms the end of a structured if-else-endif block.

    // ### Keyboard ###
    // These actions read the keyboard.
    { "letkey", { "letkey", FkissVersion::FKiSS4, false } },   // letkey(variable, keystring) - The letkey action sets the value of variable to the index of the first key in keystring which is pressed.
    { "letkeymap", { "letkeymap", FkissVersion::FKiSS4, false } },   // letkeymap(variable, keystring) - The letkeymap action sets each bit in variable if the corresponding key in keystring is pressed.

    // ### Mouse ###
    // These actions read the position of the mouse cursor.
    { "letmousex", { "letmousex", FkissVersion::FKiSS3, false } },   // letmousex(variable) - The letmousex action sets the value of variable to the current horizontal position of the mouse pointer.
    { "letmousey", { "letmousey", FkissVersion::FKiSS3, false } },   // letmousey(variable) - The letmousey action sets the value of variable to the current vertical position of the mouse pointer.

    // ### Environment ###
    // These actions affect the environment, set, or play field.
    { "changecol", { "changecol", FkissVersion::FKiSS1, false } },   // changecol(N) - The changecol action sets the palette for the current page to N.
    { "changeset", { "changeset", FkissVersion::FKiSS1, false } },   // changeset(N) - The changeset action changes to the set (page) specified by N.
    { "letpal", { "letpal", FkissVersion::FKiSS3, false } },   // letpal(variable) - The letpal action sets the value of variable to the current palette for the current set (page).
    { "letset", { "letset", FkissVersion::FKiSS3, false } },   // letset(variable) - The letset action sets the value of variable to the current set (page).

    // ### Viewport ###
    // These actions control the size of the window and view on that window.
    { "windowsize", { "windowsize", FkissVersion::FKiSS1, false } },   // windowsize(W, H) - The windowsize action sets the size of the viewport to <W x H>.
    { "viewport", { "viewport", FkissVersion::FKiSS1, false } },   // viewport(X, Y) - The viewport action sets the play field position of the top-left corner of the viewport to <X, Y>.

    // ### Miscellaneous ###
    // These actions fall in no particular category.
    { "nop", { "nop", FkissVersion::FKiSS1, false } },   // nop() - The nop action does nothing. It can be used as a placeholder in an event that may not have any actions in it.
    { "quit", { "quit", FkissVersion::FKiSS1b, false } },   // quit() - The quit action closes the KiSS doll.

    // ### Pool Values ###
    // These actions control access to value pools.
    { "valuepool", { "valuepool", FkissVersion::FKiSS4, false } },   // valuepool(pool) - The valuepool action sets the current value pool where data will be saved, loaded, or deleted.
    { "savevalue", { "savevalue", FkissVersion::FKiSS4, false } },   // savevalue(name, value) - The savevalue action saves a value to disk which can be loaded again later, even during future sessions.
    { "loadvalue", { "loadvalue", FkissVersion::FKiSS4, false } },   // loadvalue(variable, name) - The loadvalue action loads a value from disk which could have been saved earlier, even during previous sessions.
    { "deletevalue", { "deletevalue", FkissVersion::FKiSS4, false } },   // deletevalue(name) - The deletevalue action removes a named value from the current value pool.

    // ### Variables ###
    // These actions affect variables.
    { "let", { "let", FkissVersion::FKiSS3, false } }   // let(variable, A) - The let action sets the value of variable to the value of A.
};

Fkiss::Fkiss(QObject *parent)
    : QObject{parent}
{

}

void Fkiss::compileScriptLines(const QList<FkissLineInfo> &lineInfoList)
{
    QVector<Token> tokens = tokenize(lineInfoList);
    // qDebug() << "[Fkiss] parsed tokens count: " << tokens.length();
    // foreach (Token token, tokens) {
    //     qDebug() << "[Fkiss]" << token;
    // }
    QVector<std::shared_ptr<ASTCommand>> ast0 = parse0(tokens);
    // foreach (ASTCommand astNode, ast0) {
    //     qDebug() << "[Fkiss]" << astNode;
    // }
    pairAstCommands(ast0);
}

bool Fkiss::commandIsEvent(const QString &command)
{
    return Fkiss::KEYWORDS_EVENT.contains(command);
}

bool Fkiss::commandIsAction(const QString &command)
{
    return Fkiss::KEYWORDS_ACTION.contains(command);
}

QVector<Token> Fkiss::tokenize(const QList<FkissLineInfo> &lineInfoList) {
    QVector<Token> tokens;
    for (int r = 0; r < lineInfoList.length(); ++r) {  // row index (line index)
        auto lineInfo = lineInfoList[r];
        int oriLineNo = lineInfo.lineNoInCnf;
        QString line = removePrefixAndCommentOfLine(lineInfo.line);
        int c = 0;  // column index
        while (c < line.length()) {
            if (line[c].isSpace()) {
                int begC = c;
                c++;
                while (c < line.length() && line[c].isSpace()) {
                    c++;
                }
                tokens.append(Token(Token::Type::Space, line.mid(begC, c - begC), oriLineNo, begC));
            } else if (line[c].isLetter()) {
                int begC = c;
                while (c < line.length() && (line[c].isLetterOrNumber() || line[c] == '_')) {
                    c++;
                }
                tokens.append(Token(Token::Type::Identifier, line.mid(begC, c - begC), oriLineNo, begC));
            } else if (line[c] == '(') {
                tokens.append(Token(Token::Type::LParen, "(", oriLineNo, c));
                c++;
            } else if (line[c] == ')') {
                tokens.append(Token(Token::Type::RParen, ")", oriLineNo, c));
                c++;
            } else if (line[c] == ',') {
                tokens.append(Token(Token::Type::Comma, ",", oriLineNo, c));
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
                        tokens.append(Token(Token::Type::String, cleanStr, oriLineNo, begC));
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
                tokens.append(Token(Token::Type::ObjectID, line.mid(begC, c - begC), oriLineNo, begC));
            } else if (line[c].isDigit() || (c+1 < line.length() && line[c] == '-' && line[c+1].isDigit())) {
                int begC = c;
                c++;
                while (c < line.length() && line[c].isDigit()) {
                    c++;
                }
                tokens.append(Token(Token::Type::Integer, line.mid(begC, c - begC), oriLineNo, begC));
            } else {
                qCritical() << QStringLiteral("[ERROR][Fkiss::Tokenizer] Unexpected character [%1:%2]: %3").arg(lineInfo.lineNoInCnf).arg(c).arg(lineInfo.line);
                c++;
            }
        }
        tokens.append(Token(Token::Type::EndOfLine, "\n", oriLineNo, line.length()));
    }
    return tokens;
}

QVector<std::shared_ptr<ASTCommand>> Fkiss::parse0(const QVector<Token> &tokens)
{
    QVector<std::shared_ptr<ASTCommand>> astCmds;
    bool bodyStarted = false;
    for (int i = 0; i < tokens.length(); i++) {
        const Token &token = tokens[i];
        if (!bodyStarted) {
            if (token.type == Token::Type::Space) { continue; }
            if (token.type == Token::Type::EndOfLine) { continue; }
            if (token.type == Token::Type::Identifier && token.value == "EventHandler") {
                bodyStarted = true;
            } else {
                logSyntaxError(token, "Command starts before EventHandler, ignore.");
            }
            continue;
        }
        if (token.type == Token::Type::Space || token.type == Token::Type::EndOfLine) {
            continue;
        }
        // Parse a full command calling
        if (token.type == Token::Type::Identifier) {
            QString cmdName = token.value;
            const Token &lPar = tokens[++i];
            if (lPar.type != Token::Type::LParen) {
                logSyntaxError(lPar, "Command name must be followed by a left parenthesis. Abort.");
            }
            // Find out args list begin & end position.
            const int parenBeg = i;
            const int parenEnd = findNearestToken(tokens, Token::Type::RParen, parenBeg);
            if (parenEnd == -1) {
                logSyntaxError(lPar, "Cannot find the matched right parenthesis. Abort.");
                return astCmds;
            }
            i = parenEnd;   //
            QList<const Token*> argsTokensWithoutParensAndSpaces;
            for (int j = parenBeg + 1; j < parenEnd; j++) {
                const Token *tk = &tokens[j];
                if (tk->type == Token::Type::Space || tk->type == Token::Type::EndOfLine) {
                    continue;
                }
                argsTokensWithoutParensAndSpaces.append(tk);
            }
            QVector<Variant> cmdArgs;
            bool waitComma = false;
            for (int j = 0; j < argsTokensWithoutParensAndSpaces.length(); j++) {
                const Token *tk = argsTokensWithoutParensAndSpaces[j];
                if (tk->type == Token::Type::Comma) {
                    if (waitComma) {
                        waitComma = false;
                    } else {
                        logSyntaxError(*tk, "Comma must be a separator between two args.");
                    }
                } else  if (tk->type == Token::Type::Integer) {
                    cmdArgs.append(Variant::fromInt(tk->value));
                    waitComma = true;
                } else if (tk->type == Token::Type::String) {
                    cmdArgs.append(Variant::fromString(tk->value));
                    waitComma = true;
                } else if (tk->type == Token::Type::ObjectID) {
                    cmdArgs.append(Variant::fromObjectId(tk->value));
                    waitComma = true;
                } else if (tk->type == Token::Type::Identifier) {
                    logSyntaxError(*tk, "Currently FKiSS3 variable, or FKiSS4 named label, is not supported.");
                    waitComma = true;
                }
            }
            const auto &nextTk = tokens[i+1];
            if (nextTk.type != Token::Type::EndOfLine && nextTk.type != Token::Type::Space) {
                logSyntaxError(nextTk, "A command must be followed by a space or an EOL. Still runnable but this is a wrong code.");
            }
            astCmds.append(std::make_shared<ASTCommand>(ASTCommand{cmdName, cmdArgs}));
        }
    }
    return astCmds;
}

int Fkiss::findNearestToken(QVector<Token> tokens, Token::Type type, int fromIndex)
{
    for (int i = fromIndex+1; i < tokens.length(); i++) {
        const Token &tk = tokens[i];
        if (tk.type == type) {
            return i;
        }
    }
    return -1;
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

void Fkiss::pairAstCommands(QVector<std::shared_ptr<ASTCommand>> &commands)
{
    m_evpairs.clear();
    std::shared_ptr<ASTPairedCommands> curPair = std::make_shared<ASTPairedCommands>();  // create sentinel
    m_evpairs.append(curPair);   // add sentinel
    foreach (std::shared_ptr<ASTCommand> cmd, commands) {
        if (commandIsEvent(cmd->name)) {
            curPair = std::make_shared<ASTPairedCommands>();
            curPair->event = cmd;
            m_evpairs.append(curPair);
        } else if (commandIsAction(cmd->name)) {
            curPair->actions.append(cmd);
        } else {
            qWarning() << QStringLiteral("[WARNING][Fkiss] Unknown command \"%1\", skip.").arg(cmd->name);
        }
    }
    m_evpairs.removeFirst();  // remove sentinel
}

void Fkiss::logSyntaxError(const Token &token, QString msg)
{
    qDebug() << QStringLiteral("[FKiSS][SyntaxError] %1   [L%2, C%3] %4")
                .arg(msg)
                .arg(token.lineNo())
                .arg(token.columnNo())
                .arg(token.value);
}

QDebug operator<<(QDebug debug, const Token &token) {
    return debug.nospace() << QStringLiteral("Token { type: %1, value: %2, line: %3, col: %4 }")
                                        .arg(Token::stringifyType(token.type))
                                        .arg(token.value)
                                        .arg(token.lineNo())
                                        .arg(token.columnNo());
}
QDebug operator<<(QDebug debug, const Token::Type &tokenType) {
    QString str;
    Token::stringifyType(tokenType);
    return debug.nospace() << str;
}

QDebug operator<<(QDebug debug, const ASTCommand& astCmd) {
    auto dbg = debug.noquote().nospace() << "[ASTCommand] " << astCmd.name << "( ";
    foreach (auto arg, astCmd.args) {
        dbg << arg;
    }
    return dbg << ")";
};

QString Token::stringifyType(Type type)
{
    switch (type) {
    case Type::Identifier: return QStringLiteral("Identifier");
    case Type::Space: return QStringLiteral("Space");
    case Type::LParen: return QStringLiteral("LParen");
    case Type::RParen: return QStringLiteral("RParen");
    case Type::Comma: return QStringLiteral("Comma");
    case Type::String: return QStringLiteral("String");
    case Type::Integer: return QStringLiteral("Integer");
    case Type::ObjectID: return QStringLiteral("ObjectID");
    case Type::EndOfLine: return QStringLiteral("EndOfLine");
    }
}
