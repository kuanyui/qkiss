#include <QDebug>
#include <QRegularExpression>

#include "parsedcnf.h"
#include "encoding.h"
// Q_LOGGING_CATEGORY(CnfParser, "qtkiss.parser.cnf")

ParsedCnfCel::ParsedCnfCel(int32_t objId, int32_t fix, QString celFileName, int32_t kcfFileIndex, QList<int32_t> setsIndexes):
    m_objId(objId),
    m_fix(fix),
    m_celFileName(celFileName),
    m_kcfFileIndex(kcfFileIndex),
    m_setsIndexes()
{
    for (int32_t x: setsIndexes) {
        m_setsIndexes.append(x);
    }
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
    parseLines(m_fileContentText.split("\n"));
}

void ParsedCnf::error(const QString &msg)
{
    QString &line = m_lines[m_lineNo-1];
    qCritical() << "[ERROR]" << m_fileName << ":" << m_lineNo << ": " << msg << " ===>" << line;
}

void ParsedCnf::log(const QString &msg)
{
    QString &line = m_lines[m_lineNo-1];
    qCritical() << "[LOG]" << m_fileName << ":" << m_lineNo << ": " << msg << " ===>" << line;
}

void ParsedCnf::parseLines(const QStringList &lines)
{
    m_lines = lines;
    m_lineNo = 0;
    for (const QString &line: std::as_const(m_lines)) {
        m_lineNo++;
        if (line.startsWith(";") && !line.startsWith(";@")) {
            m_readableComments.append(line);
        } else if (line.startsWith("(")) {
            static QRegularExpression rePlayFieldSize(R"#(^\((\d+) *?, *?(\d+))#");
            QRegularExpressionMatch m = rePlayFieldSize.match(line);
            if (!m.hasMatch()) { error("Wrong playFieldSize format."); continue; }
            m_playFieldSize.setWidth(m.captured(1).toInt());
            m_playFieldSize.setHeight(m.captured(2).toInt());
        } else if (line.startsWith("%")) {
            m_kcfFileNames.append(line.mid(1));
        } else if (line.startsWith("[")) {
            static QRegularExpression reOutsideColor(R"#(^\[(\d+))#");
            QRegularExpressionMatch m = reOutsideColor.match(line);
            if (!m.hasMatch()) { error("Wrong outsideColor format."); continue; }
            m_outsideColor = line.mid(1).toUInt();
        } else if (line.startsWith("#")) {
            // static QRegularExpression reCel(R"#(^#(\d+)(?:[.](\d+))?\s+(\S+)\s+(?:[*](\d+))?\s+(?:[:]([\d\s]+))?)#");
            static QRegularExpression reCel(R"#(^#(\d+)(?:[.](\d+))?\s+(\S+)(?:\s+[*](\d+))?(?:\s+[:]([\d\s]+))?(?:;(.+))?)#");
            QRegularExpressionMatch m = reCel.match(line);
            if (!m.hasMatch()) { error("Wrong cel format."); continue; }
            int32_t objId = m.captured(1).toInt();
            int32_t fix = m.captured(2).toInt();
            QString celFileName = m.captured(3);
            int32_t kcfFileIndex = m.captured(4).toInt();
            static QRegularExpression reSpace("\\s+");
            QStringList setIndexesStrList = m.captured(5).trimmed().split(reSpace);
            QList<int32_t> setIndexes {};
            for (const QString &x: std::as_const(setIndexesStrList)) {
                setIndexes.append(x.toInt());
            }
            QString comment = m.captured(6).trimmed();
            log(QString("{ objId: %1, fix: %2, fileName: %3, kcfIdx: %4, sets: %5, comment: %6 }")
                .arg(objId)
                .arg(fix)
                .arg(celFileName)
                .arg(kcfFileIndex)
                .arg("[" + setIndexesStrList.join(",") + "]")
                .arg(comment)
                );
            ParsedCnfCel cnfCel {objId, fix, celFileName, kcfFileIndex, setIndexes};
            m_cels.append(cnfCel);
        } else if (line.startsWith("$")) {

        }
    }
    qDebug() << m_readableComments;
    m_lines.clear();
}
