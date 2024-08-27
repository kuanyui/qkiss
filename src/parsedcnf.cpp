#include <QDebug>
#include <QRegularExpression>

#include "parsedcnf.h"
#include "encoding.h"
// Q_LOGGING_CATEGORY(CnfParser, "qtkiss.parser.cnf")

ParsedCnfCel::ParsedCnfCel(int32_t objId, int16_t fix, QString celFileName, int16_t kcfFileIndex, QList<uint8_t> setsIndexes):
    m_objId(objId),
    m_fix(fix),
    m_celFileName(celFileName),
    m_kcfFileIndex(kcfFileIndex),
    m_setsIndexes(setsIndexes)
{

}

ParsedCnfSet::ParsedCnfSet(uint8_t paletteNoInKcf):
    m_paletteNoInKcf(paletteNoInKcf)
{

}

void ParsedCnfSet::appendObjPos(uint32_t x, uint32_t y)
{
    m_objPos.append(QPoint(x, y));
}

void ParsedCnfSet::appendEmptyObjPos()
{
    m_objPos.append(std::nullopt);
}



ParsedCnf::ParsedCnf(QString fileName, QByteArray rawData, QtKISS::Encoding encode):
    m_errCount(0),
    m_fileName(fileName)
{
    m_fileContentText = Encoding::toUtf8(rawData, encode);
    QStringList lines = m_fileContentText.split("\n");
    parseLines(lines);
}

void ParsedCnf::parseLines(const QStringList &lines)
{
    u_int32_t lineNo {0};
    static QRegularExpression rePlayFieldSize(R"#(^((\d+) *?, *?(\d+))#");
    for (const QString &line: lines) {
        lineNo++;
        if (line.startsWith(";") && !line.startsWith(";@")) {
            m_readableComments.append(line);
        } else if (line.startsWith("(")) {
            QRegularExpressionMatch m = rePlayFieldSize.match(line);
            if (!m.hasMatch()) { continue; }
            m_playFieldSize.setWidth(m.captured(1).toInt());
            m_playFieldSize.setHeight(m.captured(2).toInt());
        } else if (line.startsWith("%")) {
            m_kcfFileNames.append(line.mid(1));
        } else if (line.startsWith("[")) {
            m_outsideColor = line.mid(1).toUInt();
        } else if (line.startsWith("#")) {
        } else if (line.startsWith("$")) {

        }
    }
}

