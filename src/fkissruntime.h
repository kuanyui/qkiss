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

#ifndef FKISSRUNTIME_H
#define FKISSRUNTIME_H

#include <QObject>
#include "fkiss.h"

class AudioPlayer;

class FkissRuntime : public QObject
{
public:
    FkissRuntime();
    ~FkissRuntime();
    void receiveUserEventFromGraphicsItem(FkissUserMovementEvent evtType, QVector<FkissVariant> vars);
    GKissScene *curKissScene() const;
    void setCurKissScene(GKissScene *newCurKissScene);
    /// Load parsed FKiSS scripts (AST). You need to call initializeAndBegin() later.
    void loadParsedFkissScript(QVector<std::shared_ptr<ASTPairedCommands>> parsedFkissScript);
    void stopTimersAndUnloadFkissScript();
    /// Trigger the events `initialize()` and `begin()`. You should set curKissScene() before calling this.
    void triggerInitializeAndBegin();
    /// Trigger the event `end()`. You should set curKissScene() before calling this.
    void triggerEnd();
    /// Trigger the event `set(setNo)`
    void triggerSet(uint8_t setNo);
    /// Trigger the event `col(paletteNo)`
    void triggerCol(uint8_t paletteNo);
private:
    /// for map(), unmap(), altmap().
    /// Return true if a target (cel or object) is mapped currently.
    /// This is stored in FKiSS runtime, doesn't actually reflect the state of scene.
    bool getMap(FkissVariant target);
    /// run action map()
    void runMap(FkissVariant target);
    void runUnmap(FkissVariant target);
    void runAltmap(FkissVariant target);

    void runMove(FkissVariant target, FkissVariant dx, FkissVariant dy);
    void runMoveto(FkissVariant target, FkissVariant x, FkissVariant y);
    void runMovebyx(FkissVariant target, FkissVariant relative, FkissVariant dx);
    void runMovebyy(FkissVariant target, FkissVariant relative, FkissVariant dy);
    void runTransparent(FkissVariant target, FkissVariant delta);

    void runIffixed(FkissVariant target, FkissVariant alarmID, FkissVariant delay);
    void runIfnotfixed(FkissVariant target, FkissVariant alarmID, FkissVariant delay);
    void runIfmapped(FkissVariant target, FkissVariant alarmID, FkissVariant delay);
    void runIfnotmapped(FkissVariant target, FkissVariant alarmID, FkissVariant delay);
    void runIfmoved(FkissVariant target, FkissVariant alarmID, FkissVariant delay);
    void runIfnotmoved(FkissVariant target, FkissVariant alarmID, FkissVariant delay);

    void runTimer(FkissVariant alarmID, FkissVariant delay);
    void runRandomtimer(FkissVariant alarmID, FkissVariant delay, FkissVariant range);
    void runSound(FkissVariant fileName);
    /// Tiny reminder: trigger this after view + scene + FKiSS runtime is ready.
    void runChangeset(FkissVariant pageNo);
    /// Tiny reminder: trigger this after view + scene + FKiSS runtime is ready.
    void runChangecol(FkissVariant paletteNoInKcf);
    /// Should be called by QTimer.
    void runActionsInAlarm(FkissVariant alarmID);
    std::shared_ptr<FkissAlarmTimer> getOrCreateAlarmTimer(FkissVariant alarmID, int delay);
    std::shared_ptr<ASTPairedCommands> getEventActionPair(const QString &eventName, const QList<FkissVariant> &args);
    bool tryToRunActionsByEvent(const QString &eventName, const QList<FkissVariant> &eventArgs);
    /// ATTENTION: may return nullptr.   // FIXME: Use std::optional?
    std::optional<std::shared_ptr<ASTPairedCommands>> findEvpair(const QString &eventName, const QList<FkissVariant> &args);
    QVector<std::shared_ptr<ASTPairedCommands>> m_evpairs;
    /// Currently active scene, all operations and manipulations will be applied on this scene.
    /// @borrowed
    GKissScene *m_curKissScene = nullptr;
    /// Key is alarmId. Runtime.
    QHash<FkissVariant, std::shared_ptr<FkissAlarmTimer>> m_alarmTimerPool;
    /// Store the visibility modified via FKiSS script (e.g. map(), unmap()). May be a cel or an obj.
    QHash<FkissVariant, bool> m_mapState;
    AudioPlayer *m_audioPlayer;
};

#endif // FKISSRUNTIME_H
