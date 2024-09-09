#ifndef FKISS_H
#define FKISS_H

#include <QObject>
#include <QList>
#include <QHash>
#include "parsedcnf.h"

struct FkissEvent {
    QString name;
    QStringList args;
};

struct FkissAction {
    QString name;
    QStringList args;
};

class Fkiss : public QObject
{
    Q_OBJECT
public:
    explicit Fkiss(QObject *parent = nullptr);
    void parseLines(const QList<FkissScriptLine> &lines);

signals:

private:
    QVector<FkissEvent*> m_events;
    QVector<FkissAction*> m_actions;

};

#endif // FKISS_H
