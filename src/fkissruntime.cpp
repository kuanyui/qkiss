#include "fkissruntime.h"
#include "gkissscene.h"
#include <QRandomGenerator>
#include "audioplayer.h"

using namespace Qt::Literals::StringLiterals;

FkissRuntime::FkissRuntime() :
    m_audioPlayer{new AudioPlayer{this}}
{
    qDebug() << "[fkissruntime] ctor()";
}

FkissRuntime::~FkissRuntime()
{
    qDebug() << "[fkissruntime] dtor()";
    stopTimersAndUnloadFkissScript();
}
void FkissRuntime::receiveUserEventFromGraphicsItem(FkissUserMovementEvent evtType, QVector<FkissVariant> vars)
{
    // All events are generated from GKissItem (instead of FKiSS script), so the items counts should not need to be checked here.
    switch (evtType) {
    case FkissUserMovementEvent::Press:    tryToRunActionsByEvent(FKK::E::PRESS, vars); qDebug() << "[DEBUG] User event " << FKK::E::PRESS << vars;  break;
    case FkissUserMovementEvent::Catch:    tryToRunActionsByEvent(FKK::E::CATCH, vars); qDebug() << "[DEBUG] User event " << FKK::E::CATCH << vars;  break;
    case FkissUserMovementEvent::Fixcatch: tryToRunActionsByEvent(FKK::E::FIXCATCH, vars); qDebug() << "[DEBUG] User event " << FKK::E::FIXCATCH << vars;  break;
    case FkissUserMovementEvent::Unfix:    tryToRunActionsByEvent(FKK::E::UNFIX, vars); qDebug() << "[DEBUG] User event " << FKK::E::UNFIX << vars;  break;
    case FkissUserMovementEvent::Collide:  tryToRunActionsByEvent(FKK::E::COLLIDE, vars); qDebug() << "[DEBUG] User event " << FKK::E::COLLIDE << vars;  break;
    case FkissUserMovementEvent::Apart:    tryToRunActionsByEvent(FKK::E::APART, vars); qDebug() << "[DEBUG] User event " << FKK::E::APART << vars;  break;
    case FkissUserMovementEvent::In:       tryToRunActionsByEvent(FKK::E::IN, vars); qDebug() << "[DEBUG] User event " << FKK::E::IN << vars;  break;
    case FkissUserMovementEvent::Stillin:  tryToRunActionsByEvent(FKK::E::STILLIN, vars); break;
    case FkissUserMovementEvent::Out:      tryToRunActionsByEvent(FKK::E::OUT, vars); qDebug() << "[DEBUG] User event " << FKK::E::OUT << vars;  break;
    case FkissUserMovementEvent::Stillout: tryToRunActionsByEvent(FKK::E::STILLOUT, vars);  break;
    case FkissUserMovementEvent::Release:  tryToRunActionsByEvent(FKK::E::RELEASE, vars); qDebug() << "[DEBUG] User event " << FKK::E::RELEASE << vars;  break;
    case FkissUserMovementEvent::Drop:     tryToRunActionsByEvent(FKK::E::DROP, vars); qDebug() << "[DEBUG] User event " << FKK::E::DROP << vars;  break;
    case FkissUserMovementEvent::Fixdrop:  tryToRunActionsByEvent(FKK::E::FIXDROP, vars); qDebug() << "[DEBUG] User event " << FKK::E::FIXDROP << vars;  break;
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
            if (!_item.has_value()) { qDebug() << "[DEBUG] FkissRuntime::setCurKissScene(), m_mapState has CEL '%1', but current cannot find this CEL in this scene..."_L1.arg(key.getCelName()); continue; }
            _item.value()->setVisible(visible);
        } else if (key.type() == FkissVariant::Type::Object) {
            std::optional<QVector<GKissItem *> *> _items = m_curKissScene->getItemsByObjId(key.getObjectId());
            if (!_items.has_value()) { qDebug() << "[DEBUG] FkissRuntime::setCurKissScene(), m_mapState has ObjectId '%1', but current cannot find this ObjectId in this scene..."_L1.arg(QString::number(key.getObjectId())); continue; }
            auto *items = _items.value();
            foreach (auto *item, *items) {
                item->setVisible(visible);
            }
        }
    }
}

void FkissRuntime::loadParsedFkissScript(QVector<std::shared_ptr<ASTPairedCommands> > parsedFkissScript)
{
    stopTimersAndUnloadFkissScript();
    m_evpairs = parsedFkissScript;
    // qDebug() << "loadParsedFkissScript begins here =========================>";
    // foreach (auto pair, parsedFkissScript) {
    //     qDebug() << pair->event->name << pair->event->args;
    // }
    // qDebug() << "loadParsedFkissScript ends here <=========================";
}

void FkissRuntime::stopTimersAndUnloadFkissScript()
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

void FkissRuntime::triggerInitializeAndBegin()
{
    tryToRunActionsByEvent(FKK::E::INITIALIZE, {});
    tryToRunActionsByEvent(FKK::E::BEGIN, {});
    tryToRunActionsByEvent(FKK::E::VERSION, { FkissVariant::fromInt(static_cast<int>(FkissVersion::FKiSS1)) });
    tryToRunActionsByEvent(FKK::E::VERSION, { FkissVariant::fromInt(static_cast<int>(FkissVersion::FKiSS1b)) });
    tryToRunActionsByEvent(FKK::E::VERSION, { FkissVariant::fromInt(static_cast<int>(FkissVersion::FKiSS2)) });
    tryToRunActionsByEvent(FKK::E::VERSION, { FkissVariant::fromInt(static_cast<int>(FkissVersion::FKiSS2_1)) });
}

void FkissRuntime::triggerEnd()
{
    tryToRunActionsByEvent(FKK::E::END, {});
}

void FkissRuntime::triggerSet(uint8_t setNo)
{
    tryToRunActionsByEvent(FKK::E::SET, { FkissVariant::fromInt(setNo) });
}

void FkissRuntime::triggerCol(uint8_t paletteNo)
{
    tryToRunActionsByEvent(FKK::E::COL, { FkissVariant::fromInt(paletteNo) });
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
    if (m_curKissScene == nullptr) { return; }
    if (target.type() != FkissVariant::Type::Object) {
        qDebug() << "[fkissruntime][WARNING] currently, the target in move() must be an object ID. Skip."_L1;
        return;
    }
    if (dx.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][ERROR] the `dx` in move() must be an integer. Skip."_L1;
        return;
    }
    if (dy.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][ERROR] the `dy` in move() must be an integer. Skip."_L1;
        return;
    }
    m_curKissScene->moveObject(target.getObjectId(), dx.getInt(), dy.getInt());
}

void FkissRuntime::runMoveto(FkissVariant target, FkissVariant x, FkissVariant y)
{
    if (m_curKissScene == nullptr) { return; }
    if (target.type() != FkissVariant::Type::Object) {
        qDebug() << "[fkissruntime][WARNING] currently, the `target` in moveto() must be an object ID. Skip."_L1;
        return;
    }
    if (x.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][ERROR] the `x` in moveto() must be an integer. Skip."_L1;
        return;
    }
    if (y.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][ERROR] the `y` in moveto() must be an integer. Skip."_L1;
        return;
    }
    m_curKissScene->moveObjectTo(target.getObjectId(), x.getInt(), y.getInt());
}

void FkissRuntime::runMovebyx(FkissVariant target, FkissVariant relative, FkissVariant dx)
{
    if (target.type() != FkissVariant::Type::Object) {
        qDebug() << "[fkissruntime][WARNING] currently, the `target` in movebyx() must be an object ID. Skip."_L1;
        return;
    }
    if (relative.type() != FkissVariant::Type::Object) {
        qDebug() << "[fkissruntime][WARNING] currently, the `target` in movebyx() must be an object ID. Skip."_L1;
        return;
    }
    if (dx.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][ERROR] the `x` in movebyx() must be an integer. Skip."_L1;
        return;
    }
    m_curKissScene->moveObjectByX(target.getObjectId(), relative.getObjectId(), dx.getInt());
}

void FkissRuntime::runMovebyy(FkissVariant target, FkissVariant relative, FkissVariant dy)
{
    if (target.type() != FkissVariant::Type::Object) {
        qDebug() << "[fkissruntime][WARNING] currently, the `target` in movebyx() must be an object ID. Skip."_L1;
        return;
    }
    if (relative.type() != FkissVariant::Type::Object) {
        qDebug() << "[fkissruntime][WARNING] currently, the `target` in movebyx() must be an object ID. Skip."_L1;
        return;
    }
    if (dy.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][ERROR] the `y` in movebyy() must be an integer. Skip."_L1;
        return;
    }
    m_curKissScene->moveObjectByY(target.getObjectId(), relative.getObjectId(), dy.getInt());
}

void FkissRuntime::runTransparent(FkissVariant target, FkissVariant delta)
{
    if (m_curKissScene == nullptr) { return; }
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

// qDebug() << "[fkissruntime][WARNING] currently, the `" #paramName "` in command `" #fkissCmd "()` must be a(n) " << FkissVariant::typeToString(expectedType) << ". Skip.";

#define RT_TYPE_CHECK(fkissCmd, paramName, expectedType) \
    if ((paramName).type() != expectedType) { \
        qDebug() << "[fkissruntime][WARNING] currently, the `" #paramName "` in command `" #fkissCmd "()` must be a " << FkissVariant::typeToString(expectedType) << ". Skip." << paramName; \
        return; \
    }

void FkissRuntime::runIffixed(FkissVariant target, FkissVariant alarmID, FkissVariant delay)
{
    // if (target.type() != FkissVariant::Type::Cel) {
    //     qDebug() << "[fkissruntime][WARNING] currently, the `target` in iffixed() must be an object ID. Skip."_L1;
    //     return;
    // }
    // if (alarmId.type() != FkissVariant::Type::Object) {
    //     qDebug() << "[fkissruntime][WARNING] currently, the `alarmId` in iffixed() must be an object ID. Skip."_L1;
    //     return;
    // }
    // if (delay.type() != FkissVariant::Type::Int) {
    //     qDebug() << "[fkissruntime][ERROR] the `delay` in iffixed() must be an integer. Skip."_L1;
    //     return;
    // }
    if (m_curKissScene == nullptr) { return; }
    RT_TYPE_CHECK("iffixed", target, FkissVariant::Type::Cel)
    RT_TYPE_CHECK("iffixed", alarmID, FkissVariant::Type::Int)
    RT_TYPE_CHECK("iffixed", delay, FkissVariant::Type::Int)
    QString targetCelName = target.getCelName();
    auto _celItem = m_curKissScene->getItemByCelName(targetCelName);
    if (!_celItem.has_value()) { return; }
    auto celItem = _celItem.value();
    if (celItem->fix() > 0) {
        getOrCreateAlarmTimer(alarmID, delay.getInt());
    }
}

void FkissRuntime::runIfnotfixed(FkissVariant target, FkissVariant alarmID, FkissVariant delay)
{
    if (m_curKissScene == nullptr) { return; }
    RT_TYPE_CHECK("iffixed", target, FkissVariant::Type::Cel)
    RT_TYPE_CHECK("iffixed", alarmID, FkissVariant::Type::Int)
    RT_TYPE_CHECK("iffixed", delay, FkissVariant::Type::Int)
    QString targetCelName = target.getCelName();
    auto _celItem = m_curKissScene->getItemByCelName(targetCelName);
    if (!_celItem.has_value()) { return; }
    auto celItem = _celItem.value();
    if (celItem->fix() == 0) {
        getOrCreateAlarmTimer(alarmID, delay.getInt());
    }
}

void FkissRuntime::runIfmapped(FkissVariant target, FkissVariant alarmID, FkissVariant delay)
{
    if (m_curKissScene == nullptr) { return; }
    RT_TYPE_CHECK("ifmapped", target, FkissVariant::Type::Cel)
    RT_TYPE_CHECK("ifmapped", alarmID, FkissVariant::Type::Int)
    RT_TYPE_CHECK("ifmapped", delay, FkissVariant::Type::Int)
    QString targetCelName = target.getCelName();
    auto _celItem = m_curKissScene->getItemByCelName(targetCelName);
    if (!_celItem.has_value()) { return; }
    auto celItem = _celItem.value();
    if (celItem->isVisible()) {
        getOrCreateAlarmTimer(alarmID, delay.getInt());
    }
}

void FkissRuntime::runIfnotmapped(FkissVariant target, FkissVariant alarmID, FkissVariant delay)
{
    if (m_curKissScene == nullptr) { return; }
    RT_TYPE_CHECK("ifnotmapped", target, FkissVariant::Type::Cel)
    RT_TYPE_CHECK("ifnotmapped", alarmID, FkissVariant::Type::Int)
    RT_TYPE_CHECK("ifnotmapped", delay, FkissVariant::Type::Int)
    QString targetCelName = target.getCelName();
    auto _celItem = m_curKissScene->getItemByCelName(targetCelName);
    if (!_celItem.has_value()) { return; }
    auto celItem = _celItem.value();
    if (!celItem->isVisible()) {
        getOrCreateAlarmTimer(alarmID, delay.getInt());
    }
}

void FkissRuntime::runIfmoved(FkissVariant target, FkissVariant alarmID, FkissVariant delay)
{
    if (m_curKissScene == nullptr) { return; }
    RT_TYPE_CHECK("ifmoved", target, FkissVariant::Type::Cel)
    RT_TYPE_CHECK("ifmoved", alarmID, FkissVariant::Type::Int)
    RT_TYPE_CHECK("ifmoved", delay, FkissVariant::Type::Int)
    QString targetCelName = target.getCelName();
    auto _celItem = m_curKissScene->getItemByCelName(targetCelName);
    if (!_celItem.has_value()) { return; }
    auto celItem = _celItem.value();
    if (celItem->pos() != celItem->initialPos()) {
        getOrCreateAlarmTimer(alarmID, delay.getInt());
    }
}

void FkissRuntime::runIfnotmoved(FkissVariant target, FkissVariant alarmID, FkissVariant delay)
{
    if (m_curKissScene == nullptr) { return; }
    RT_TYPE_CHECK("ifmoved", target, FkissVariant::Type::Cel)
    RT_TYPE_CHECK("ifmoved", alarmID, FkissVariant::Type::Int)
    RT_TYPE_CHECK("ifmoved", delay, FkissVariant::Type::Int)
    QString targetCelName = target.getCelName();
    auto _celItem = m_curKissScene->getItemByCelName(targetCelName);
    if (!_celItem.has_value()) { return; }
    auto celItem = _celItem.value();
    if (celItem->pos() == celItem->initialPos()) {
        getOrCreateAlarmTimer(alarmID, delay.getInt());
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

void FkissRuntime::runSound(FkissVariant fileName)
{
    if (m_curKissScene == nullptr) { return; }
    auto archiveFile = m_curKissScene->archiveFile();
    if (!archiveFile.has_value()) { return; }
    auto audioFileData = archiveFile.value()->getOtherFile(fileName.getString());
    if (!audioFileData.has_value()) { return; }
    m_audioPlayer->playWavFromMemory(audioFileData.value());
}

void FkissRuntime::runChangeset(FkissVariant pageNo)
{
    if (m_curKissScene == nullptr) { return; }
    if (pageNo.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][WARNING] currently, the pageNo in changeset(pageNo) must be an integer. Skip."_L1;
        return;
    }
    int setNo = pageNo.getInt();
    auto archiveFile = m_curKissScene->archiveFile();
    if (archiveFile.has_value()) {
        archiveFile.value()->setCnfSetNoFromFkiss(setNo);
    }
}

void FkissRuntime::runChangecol(FkissVariant paletteNoInKcf)
{
    if (m_curKissScene == nullptr) { return; }
    if (paletteNoInKcf.type() != FkissVariant::Type::Int) {
        qDebug() << "[fkissruntime][WARNING] currently, the paletteNoInKcf in changecol(paletteNoInKcf) must be an integer. Skip."_L1;
        return;
    }
    int paletteNoInKcfInt = paletteNoInKcf.getInt();
    auto archiveFile = m_curKissScene->archiveFile();
    if (archiveFile.has_value()) {
        archiveFile.value()->setPaletteNoFromFkiss(paletteNoInKcfInt);
    }
}

void FkissRuntime::runActionsInAlarm(FkissVariant alarmID)
{
    // qDebug() << "[fkissruntime] runActionsInAlarm" << alarmID;
    tryToRunActionsByEvent(FKK::E::ALARM, { alarmID });
}

std::shared_ptr<FkissAlarmTimer> FkissRuntime::getOrCreateAlarmTimer(FkissVariant alarmID, int delay)
{
    if (m_alarmTimerPool.contains(alarmID)) {
        std::shared_ptr<FkissAlarmTimer> tmp = m_alarmTimerPool.value(alarmID);
        tmp->qTimer->stop();
        if (delay == 0) {   // delay = 0 means cancel the timer.
            return tmp;
        }
        tmp->qTimer->start(delay);
        return tmp;
    } else {
        std::shared_ptr<FkissAlarmTimer> tmp = std::make_shared<FkissAlarmTimer>();
        m_alarmTimerPool.insert(alarmID, tmp);
        tmp->alarmId = alarmID.getInt();
        tmp->qTimer = std::make_unique<QTimer>();
        tmp->qTimer->setSingleShot(true);
        tmp->qTimer->setInterval(delay);
        connect(tmp->qTimer.get(), &QTimer::timeout, [=, this]() {
            runActionsInAlarm(alarmID);
        });
        if (delay == 0) {
            return tmp;
        }
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
    bool printDebug = false;
    if (eventName == FKK::E::STILLOUT) {  // Stillout() generate too many logs
        printDebug = false;
    }
    if (printDebug) {
        qDebug() << "[DEBUG][Fkiss] tryToRunActionsByEvent" << eventName << eventArgs;
    }
    auto _evpair = findEvpair(eventName, eventArgs);
    if (!_evpair.has_value()) { return false; }
    auto evpair = _evpair.value();
    foreach (auto act, evpair->actions) {
        if (printDebug) {
            qDebug() << "[DEBUG][Fkiss]   tryToRunActionsByEvent(): execute action" << act->name << act->args;
        }
        auto actName = act->name;
        auto actArgs = act->args;
        if (actName == FKK::A::MAP) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid map() args count, skip."_L1; continue; }
            runMap(actArgs[0]);
            continue;
        }
        if (actName == FKK::A::UNMAP) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid unmap() args count, skip."_L1; continue; }
            runUnmap(actArgs[0]);
            continue;
        }
        if (actName == FKK::A::ALTMAP) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid altmap() args count, skip."_L1; continue; }
            runAltmap(actArgs[0]);
            continue;
        }
        if (actName == FKK::A::MOVE) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid move() args count, skip."_L1; continue; }
            runMove(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }
        if (actName == FKK::A::MOVETO) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid moveto() args count, skip."_L1; continue; }
            runMoveto(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }
        if (actName == FKK::A::MOVEBYX) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid movebyx() args count, skip."_L1; continue; }
            runMovebyx(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }
        if (actName == FKK::A::MOVEBYY) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid movebyy() args count, skip."_L1; continue; }
            runMovebyy(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }
        if (actName == FKK::A::IFFIXED) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid iffixed() args count, skip."_L1; continue; }
            runIffixed(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }
        if (actName == FKK::A::IFNOTFIXED) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid ifnotfixed() args count, skip."_L1; continue; }
            runIfnotfixed(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }
        if (actName == FKK::A::IFMAPPED) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid ifmapped() args count, skip."_L1; continue; }
            runIfmapped(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }
        if (actName == FKK::A::IFNOTMAPPED) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid ifnotmapped() args count, skip."_L1; continue; }
            runIfnotmapped(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }
        if (actName == FKK::A::IFMOVED) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid ifmoved() args count, skip."_L1; continue; }
            runIfmoved(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }
        if (actName == FKK::A::IFNOTMOVED) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid ifnotmoved() args count, skip."_L1; continue; }
            runIfnotmoved(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }
        if (actName == FKK::A::TRANSPARENT) {
            if (actArgs.length() != 2) { qDebug() << "[Fkiss] invalid transparent(target, N) args count, skip."_L1; continue; }
            runTransparent(actArgs[0], actArgs[1]);
            continue;
        }
        if (actName == FKK::A::TIMER) {
            if (actArgs.length() != 2) { qDebug() << "[Fkiss] invalid timer(alarmID, delayMs) args count, skip."_L1; continue; }
            runTimer(actArgs[0], actArgs[1]);
            continue;
        }
        if (actName == FKK::A::RANDOMTIMER) {
            if (actArgs.length() != 3) { qDebug() << "[Fkiss] invalid randomtimer(alarmID, delay, range) args count, skip."_L1; continue; }
            runRandomtimer(actArgs[0], actArgs[1], actArgs[2]);
            continue;
        }
        if (actName == FKK::A::SOUND) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid sound(fileName) args count, skip."_L1; continue; }
            runSound(actArgs[0]);
            continue;
        }
        if (actName == FKK::A::CHANGESET) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid changeset(pageNo) args count, skip."_L1; continue; }
            runChangeset(actArgs[0]);
            continue;
        }
        if (actName == FKK::A::CHANGECOL) {
            if (actArgs.length() != 1) { qDebug() << "[Fkiss] invalid changecol(paletteNo) args count, skip."_L1; continue; }
            runChangecol(actArgs[0]);
            continue;
        }
        qDebug() << "[Fkiss][WARN] unsupported action `%1`, skip."_L1.arg(actName);
    }
    return true;
}

std::optional<std::shared_ptr<ASTPairedCommands> > FkissRuntime::findEvpair(const QString &eventName, const QList<FkissVariant> &args)
{
    static QStringList EVENTS_WITH_ORDER_INSENSITIVE_ARGS {
        FKK::E::COLLIDE,
        FKK::E::APART,
        FKK::E::IN,
        FKK::E::STILLIN,
        FKK::E::OUT,
        FKK::E::STILLOUT
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


