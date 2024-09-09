#include "fkiss.h"
#include <QRegularExpression>


Fkiss::Fkiss(QObject *parent)
    : QObject{parent}
{

}

void Fkiss::parseLines(const QList<FkissScriptLine> &lines)
{
    foreach (const FkissScriptLine info, lines) {
        const QString &rawLine = info.line.trimmed();
        static QRegularExpression re(R"#(;@\s*(.+))#");
        auto m = re.match(rawLine);
        if (!m.hasMatch()) {
            qCritical() << QStringLiteral("[ERROR][Fkiss] Impossible. Skip line %1 : %2").arg(info.lineNoInCnf).arg(info.line);
            continue;
        }
    }
}
