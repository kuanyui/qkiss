#include "fkissruntime.h"
#include "gkissscene.h"
#include <QRandomGenerator>
using namespace Qt::Literals::StringLiterals;

FkissRuntime::FkissRuntime()
{
    qDebug() << "[fkissruntime] ctor()";
}

FkissRuntime::~FkissRuntime()
{
    qDebug() << "[fkissruntime] dtor()";
    unloadFkissScript();
}

void FkissRuntime::receiveUserEventFromGraphicsItem(FkissUserMovementEvent evtType, QVector<FkissVariant> vars)
{
    // All events are generated from GKissItem (instead of FKiSS script), so the items counts should not need to be checked here.
    switch (evtType) {
    case FkissUserMovementEvent::Press:    tryToRunActionsByEvent(FKK::PRESS, vars); break;
    case FkissUserMovementEvent::Catch:    tryToRunActionsByEvent(FKK::CATCH, vars); break;
    case FkissUserMovementEvent::Fixcatch: tryToRunActionsByEvent(FKK::FIXCATCH, vars); break;
    case FkissUserMovementEvent::Unfix:    tryToRunActionsByEvent(FKK::UNFIX, vars); break;
    case FkissUserMovementEvent::Collide:  tryToRunActionsByEvent(FKK::COLLIDE, vars); break;
    case FkissUserMovementEvent::Apart:    tryToRunActionsByEvent(FKK::APART, vars); break;
    case FkissUserMovementEvent::In:       tryToRunActionsByEvent(FKK::IN, vars); break;
    case FkissUserMovementEvent::Stillin:  tryToRunActionsByEvent(FKK::STILLIN, vars); break;
    case FkissUserMovementEvent::Out:      tryToRunActionsByEvent(FKK::OUT, vars); break;
    case FkissUserMovementEvent::Stillout: tryToRunActionsByEvent(FKK::STILLOUT, vars); break;
    case FkissUserMovementEvent::Release:  tryToRunActionsByEvent(FKK::RELEASE, vars); break;
    case FkissUserMovementEvent::Drop:     tryToRunActionsByEvent(FKK::DROP, vars); break;
    case FkissUserMovementEvent::Fixdrop:  tryToRunActionsByEvent(FKK::FIXDROP, vars); break;
    }
    return;
}


void FkissRuntime::setCurKissScene(GKissScene *newCurKissScene)
{
    m_curKissScene = newCurKissScene;
    if (m_curKissScene == nullptr) { return; }
    QHashIterator<FkissVariant, bool> it(m_mapState);
    while (it.hasNext()) {
        it.next();
        const FkissVariant &key = it.key();
        bool visible = it.value();
        if (key.type() == FkissVariant::Type::String) {
            auto _item = m_curKissScene->getItemByCelName(key.getString());
            if (!_item.has_value()) { qDebug() << "WTF???"; continue; }
            _item.value()->setVisible(visible);
        } else if (key.type() == FkissVariant::Type::Object) {
            std::optional<QVector<GKissItem *> *> _items = m_curKissScene->getItemsByObjId(key.getObjectId());
            if (!_items.has_value()) { qDebug() << "WTF???"; continue; }
            auto *items = _items.value();
            foreach (auto *item, *items) {
                item->setVisible(visible);
            }
        }
    }
}

void FkissRuntime::loadParsedFkissScript(QVector<std::shared_ptr<ASTPairedCommands> > parsedFkissScript)
{
    unloadFkissScript();
    m_evpairs = parsedFkissScript;
    qDebug() << "load =========================";
    foreach (auto pair, parsedFkissScript) {
        qDebug() << pair->event->name << pair->event->args;
    }
    tryToRunActionsByEvent(FKK::INITIALIZE, {});
    tryToRunActionsByEvent(FKK::BEGIN, {});
}

void FkissRuntime::unloadFkissScript()
{
    qDebug() << "[fkissruntime] unloadFkissScript()";
    foreach (auto x, m_alarmTimerPool.values()) {
        x->qTimer->stop();
        // disconnect(x->qTimer.get(), nullptr, nullptr, nullptr);
        x->qTimer->disconnect();
    }
    m_alarmTimerPool.clear();
    m_evpairs.clear();
}

bool FkissRuntime::getMap(FkissVariant target)
{
    return m_mapState.value(target, true);
}

void FkissRuntime::runMap(FkissVariant target)
{
    m_mapState.insert(target, true);
    if (m_curKissScene) {
        m_curKissScene->setItemsVisibility(target, true);
    }
}

void FkissRuntime::runUnmap(FkissVariant target)
{
    m_mapState.insert(target, false);
    if (m_curKissScene) {
        m_curKissScene->setItemsVisibility(target, false);
    }
}

void FkissRuntime::runAltmap(FkissVariant target)
{
    bool oldVal = getMap(target);
    bool newVal = !oldVal;
    m_mapState.insert(target, newVal);
    if (m_curKissScene) {
        m_curKissScene->setItemsVisibility(target, newVal);
    }
}

void FkissRuntime::runMove(FkissVariant target, FkissVariant dx, FkissVariant dy)
{
    if (target.type() != FkissVariant::Type::Object) {
        qDebug() << "[fkissruntime][WARNING] currently, the target in move() must be an object ID. Skip."_L1;
        return;
    }
    if (dx.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][ERROR] the `dx` in move() must be an object ID. Skip."_L1;
        return;
    }
    if (dy.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][ERROR] the `dy` in move() must be an object ID. Skip."_L1;
        return;
    }
    m_curKissScene->moveObjectBy(target.getObjectId(), dx.getInt(), dy.getInt());
}

void FkissRuntime::runMoveto(FkissVariant target, FkissVariant x, FkissVariant y)
{
    if (target.type() != FkissVariant::Type::Object) {
        qDebug() << "[fkissruntime][WARNING] currently, the `target` in moveto() must be an object ID. Skip."_L1;
        return;
    }
    if (x.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][ERROR] the `x` in moveto() must be an object ID. Skip."_L1;
        return;
    }
    if (y.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][ERROR] the `y` in moveto() must be an object ID. Skip."_L1;
        return;
    }
    m_curKissScene->moveObjectTo(target.getObjectId(), x.getInt(), y.getInt());
}

void FkissRuntime::runTransparent(FkissVariant target, FkissVariant delta)
{
    if (!delta.isInt()) {
        qDebug() << "[fkissruntime][ERROR] the `translucency` in transparent() must be an object ID. Skip."_L1;
        return;
    }
    switch (target.type()) {
    case FkissVariant::Type::Object: m_curKissScene->executeTransparentOnObj(target.getObjectId(), delta.getInt()); return;
    case FkissVariant::Type::Cel: m_curKissScene->executeTransparentOnCel(target.getCelName(), delta.getInt()); return;
    case FkissVariant::Type::String: m_curKissScene->executeTransparentOnCel(target.getString(), delta.getInt()); return;
    default:
        qDebug() << "[fkissruntime][WARNING] currently, the `target` in transparent() must be an cel or object. Skip."_L1;
    }
}



void FkissRuntime::runTimer(FkissVariant alarmID, FkissVariant delay)
{
    if (alarmID.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][WARNING] currently, the alarmID in timer() must be an integer. Skip."_L1;
        return;
    }
    if (delay.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][ERROR] delay in timer() must be an interger. Skip."_L1;
        return;
    }
    getOrCreateAlarmTimer(alarmID, delay.getInt());
}
void FkissRuntime::runRandomtimer(FkissVariant alarmID, FkissVariant delay, FkissVariant range)
{
    if (alarmID.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][WARNING] currently, the alarmID in randomtimer() must be an integer. Skip."_L1;
        return;
    }
    if (delay.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][ERROR] delay in randomtimer() must be an interger. Skip."_L1;
        return;
    }
    if (range.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][ERROR] range in randomtimer() must be an interger. Skip."_L1;
        return;
    }
    int from = delay.getInt();
    int to = from + range.getInt();
    int realDelay = QRandomGenerator::global()->bounded(from, to);
    getOrCreateAlarmTimer(alarmID, realDelay);
}

void FkissRuntime::runActionsInAlarm(FkissVariant alarmID)
{
    // qDebug() << "[fkissruntime] runActionsInAlarm" << alarmID;
    tryToRunActionsByEvent(FKK::ALARM, { alarmID });
}

std::shared_ptr<FkissAlarmTimer> FkissRuntime::getOrCreateAlarmTimer(FkissVariant alarmID, int delay)
{
    if (m_alarmTimerPool.contains(alarmID)) {
        std::shared_ptr<FkissAlarmTimer> tmp = m_alarmTimerPool.value(alarmID);
        tmp->qTimer->stop();
        tmp->qTimer->start(delay);
        return tmp;
    } else {
        std::shared_ptr<FkissAlarmTimer> tmp = std::make_shared<FkissAlarmTimer>();
        m_alarmTimerPool.insert(alarmID, tmp);
        tmp->alarmId = alarmID.getInt();
        tmp->qTimer = std::make_unique<QTimer>();
        tmp->qTimer->setSingleShot(true);
        tmp->qTimer->setInterval(delay);
        connect(tmp->qTimer.get(), &QTimer::timeout, [=]() {
            runActionsInAlarm(alarmID);
        });
        tmp->qTimer->start();
        return tmp;
    }
}

GKissScene *FkissRuntime::curKissScene() const
{
    return m_curKissScene;
}


std::shared_ptr<ASTPairedCommands> FkissRuntime::getEventActionPair(const QString &eventName, const QList<FkissVariant> &args)
{
    foreach (std::shared_ptr<ASTPairedCommands> pair, m_evpairs) {
        if (pair->event->name == eventName) {
            if (pair->event->args == args) {
                return pair;
            }
        }
    }
    return nullptr;
}

bool FkissRuntime::tryToRunActionsByEvent(const QString &eventName, const QList<FkissVariant> &eventArgs)
{
    qDebug() << "[Fkiss] tryToRunActionsByEvent" << eventName << eventArgs;
    auto _evpair = findEvpair(eventName, eventArgs);
    if (!_evpair.has_value()) { return false; }
    auto evpair = _evpair.value();
    foreach (auto act, evpair->actions) {
        qDebug() << "[Fkiss]   tryToRunActionsByEvent(): execute action" << act->name << act->args;
        auto actName = act->name;
        auto actArgs = act->args;
        if (actName == FKK::MAP) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid map() args count, skip."_L1; continue; }
            runMap(actArgs[0]);
            continue;
        }
        if (actName == FKK::UNMAP) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid unmap() args count, skip."_L1; continue; }
            runUnmap(actArgs[0]);
            continue;
        }
        if (actName == FKK::ALTMAP) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid altmap() args count, skip."_L1; continue; }
            runAltmap(actArgs[0]);
            continue;
        }
        if (actName == FKK::MOVE) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid move() args count, skip."_L1; continue; }
            runMove(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }        if (actName == FKK::MOVETO) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid moveto() args count, skip."_L1; continue; }
            runMoveto(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }
        if (actName == FKK::TRANSPARENT) {
            if (actArgs.length() != 2) { qDebug() << "[Fkiss] invalid transparent(target, N) args count, skip."_L1; continue; }
            runTransparent(actArgs[0], actArgs[1]);
            continue;
        }
        if (actName == FKK::TIMER) {
            if (actArgs.length() != 2) { qDebug() << "[Fkiss] invalid timer(alarmID, delayMs) args count, skip."_L1; continue; }
            runTimer(actArgs[0], actArgs[1]);
            continue;
        }
        if (actName == FKK::RANDOMTIMER) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid randomtimer(alarmID, delay, range) args count, skip."_L1; continue; }
            runRandomtimer(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }
    }
    return true;
}

std::optional<std::shared_ptr<ASTPairedCommands> > FkissRuntime::findEvpair(const QString &eventName, const QList<FkissVariant> &args)
{
    static QStringList EVENTS_WITH_ORDER_INSENSITIVE_ARGS {
        FKK::COLLIDE,
        FKK::APART,
        FKK::IN,
        FKK::STILLIN,
        FKK::OUT,
        FKK::STILLOUT
    };
    bool isOrderInsensitiveArgs = EVENTS_WITH_ORDER_INSENSITIVE_ARGS.contains(eventName);
    // TODO: some events' args are order-insensitive. e.g. collide(), in()
    foreach (auto evpair, m_evpairs) {
        if (evpair->event->name != eventName) { continue; }
        if (evpair->event->args == args) {
            return evpair;
        }
        // TODO: optimize this shit, please.
        if (isOrderInsensitiveArgs && evpair->event->args.length() == args.length()) {
            QSet<FkissVariant> s1 { evpair->event->args.begin(), evpair->event->args.end() };
            QSet<FkissVariant> s2 { args.begin(), args.end() };
            if (s1 == s2) {
                return evpair;
            }
        }
    }
    return std::nullopt;
}


