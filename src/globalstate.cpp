#include "globalstate.h"
#include <QSettings>

namespace CfgFileKey {
    // constexpr auto MainWindow_Geometry = "MainWindow/Geometry";
    // constexpr auto MainWindow_State = "MainWindow/State";
    constexpr auto Global_CaseSensitivity = "Global/CaseSensitivity";
}

GlobalState::GlobalState(QObject *parent)
    : QObject{parent}
{

}

void GlobalState::loadAll()
{
    QSettings set;
    // set.value(CfgKey::Global_CaseSensitivity, )
}

void GlobalState::saveAll()
{

}

QKiSS::Case GlobalState::caseSensitivity() const
{
    return m_caseSensitivity;
}

void GlobalState::setCaseSensitivity(QKiSS::Case newCaseSensitivity)
{
    m_caseSensitivity = newCaseSensitivity;
    QSettings set;
    set.setValue(CfgFileKey::Global_CaseSensitivity, static_cast<int>(newCaseSensitivity));
}
