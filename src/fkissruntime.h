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
    void loadParsedFkissScript(QVector<std::shared_ptr<ASTPairedCommands>> parsedFkissScript);
    void unloadFkissScript();
private:
    /// run map(), unmap(), altmap()
    void runMap(FkissVariant target);
    void runUnmap(FkissVariant target);
    void runAltmap(FkissVariant target);
    void runMove(FkissVariant target, FkissVariant dx, FkissVariant dy);
    void runMoveto(FkissVariant target, FkissVariant x, FkissVariant y);
    /// for map(), unmap(), altmap()
    bool getMap(FkissVariant target);
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
