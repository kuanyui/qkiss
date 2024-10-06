#include "fkissruntime.h"
#include "gkissscene.h"
using namespace Qt::Literals::StringLiterals;

FkissRuntime::FkissRuntime()
{

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

bool FkissRuntime::getMap(FkissVariant target)
{
    return m_mapState.value(target, true);
}

GKissScene *FkissRuntime::curKissScene() const
{
    return m_curKissScene;
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
    m_evpairs = parsedFkissScript;
    tryToRunActionsByEvent(FKK::INITIALIZE, {});
    tryToRunActionsByEvent(FKK::BEGIN, {});
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
        auto actArgs = act->args;
        if (act->name == FKK::MAP) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid map() args count, skip."_L1; continue; }
            runMap(actArgs[0]);
        } else if (act->name == FKK::UNMAP) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid unmap() args count, skip."_L1; continue; }
            runUnmap(act->args[0]);
        } else if (act->name == FKK::ALTMAP) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid altmap() args count, skip."_L1; continue; }
            runAltmap(act->args[0]);
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
