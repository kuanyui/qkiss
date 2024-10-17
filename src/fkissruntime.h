#ifndef FKISSRUNTIME_H
#define FKISSRUNTIME_H

#include <QObject>
#include "fkiss.h"

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
private:
    /// for map(), unmap(), altmap()
    bool getMap(FkissVariant target);
    /// run action map()
    void runMap(FkissVariant target);
    void runUnmap(FkissVariant target);
    void runAltmap(FkissVariant target);
    void runMove(FkissVariant target, FkissVariant dx, FkissVariant dy);
    void runMoveto(FkissVariant target, FkissVariant x, FkissVariant y);
    void runTransparent(FkissVariant target, FkissVariant delta);
    void runTimer(FkissVariant alarmID, FkissVariant delay);
    void runRandomtimer(FkissVariant alarmID, FkissVariant delay, FkissVariant range);
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
};

#endif // FKISSRUNTIME_H
