#include "fkiss.h"
#include "fkissparser.h"
#include "fkissruntime.h"
#include <QRegularExpression>
#include <QHashIterator>
using namespace Qt::Literals::StringLiterals;

Fkiss::Fkiss(QObject *parent)
    : QObject{parent},
      m_parser{new FkissParser()},
      m_runtime(new FkissRuntime())
{

}

Fkiss::~Fkiss()
{
    delete m_parser;
    delete m_runtime;
}

void Fkiss::loadFkissScript(QList<FkissLineInfo> fkissScriptLines)
{
    QVector<std::shared_ptr<ASTPairedCommands>> parsedScript = m_parser->parseScriptLines(fkissScriptLines);
    m_runtime->loadParsedFkissScript(parsedScript);
}

FkissRuntime *Fkiss::runtime()
{
    return m_runtime;
}

FkissParser *Fkiss::parser()
{
    return m_parser;
}



QDebug operator<<(QDebug debug, const ASTCommand& astCmd) {
    auto dbg = debug.noquote().nospace() << "[ASTCommand] " << astCmd.name << "( ";
    foreach (auto arg, astCmd.args) {
        dbg << arg;
    }
    return dbg << ")";
};

