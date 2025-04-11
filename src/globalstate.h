#ifndef GLOBALSTATE_H
#define GLOBALSTATE_H

#include <QObject>
#include <QWidget>
#include "archivefile.h"

class GlobalState : public QObject
{
    Q_OBJECT
public:
    static GlobalState& instance() {
        static GlobalState instance;
        return instance;
    }
    void loadAll();
    void saveAll();
    QKiSS::Case caseSensitivity() const;
    void setCaseSensitivity(QKiSS::Case newCaseSensitivity);

signals:

private:
    /** Avoid initialized from outside. */
    GlobalState(QObject *parent = nullptr);
    Q_DISABLE_COPY(GlobalState)

    QKiSS::Case m_caseSensitivity = QKiSS::Case::Insensitive;
    // /** 0 ~ 9 */
    // uint8_t m_cnfSetNo;
    // /** usually it's 9. */
    // uint8_t m_cnfSetNoMax;
    // /** paletteNo in a KCF file */
    // uint8_t m_cnfPletteNo;
};

#endif // GLOBALSTATE_H
