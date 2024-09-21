#include <QDebug>
#include <QRegularExpression>

#include "parsedcnf.h"
#include "encoding.h"
// Q_LOGGING_CATEGORY(CnfParser, "qtkiss.parser.cnf")
using namespace Qt::Literals::StringLiterals;

ParsedCnfCel::ParsedCnfCel(int32_t objId, int32_t fix, QString celFileName, int32_t kcfFileIndex, QList<int32_t> setsIndexes, QString comment):
    m_objId(objId),
    m_fix(fix),
    m_celFileName(celFileName),
    m_kcfFileIndex(kcfFileIndex),
    m_setsIndexes(),
    m_comment(comment)
{
    for (int32_t x: setsIndexes) {
        m_setsIndexes.append(x);
    }
}

int32_t ParsedCnfCel::objId() const
{
    return m_objId;
}

int16_t ParsedCnfCel::fix() const
{
    return m_fix;
}

QString ParsedCnfCel::celFileName() const
{
    return m_celFileName;
}

int16_t ParsedCnfCel::kcfFileIndex() const
{
    return m_kcfFileIndex;
}

QList<uint8_t> ParsedCnfCel::setsIndexes() const
{
    return m_setsIndexes;
}

QString ParsedCnfCel::comment() const
{
    return m_comment;
}

ParsedCnfSet::ParsedCnfSet(int32_t paletteNoInKcf):
    m_paletteNoInKcf(paletteNoInKcf)
{

}

void ParsedCnfSet::appendObjPos(int32_t x, int32_t y)
{
    m_objPosList.append(std::optional(QPoint(x, y)));

}

void ParsedCnfSet::appendEmptyObjPos()
{
    m_objPosList.append(std::optional(QPoint(0, 0)));
}

int32_t ParsedCnfSet::paletteNoInKcf() const
{
    return m_paletteNoInKcf;
}

const QList<ParsedCnfSetObjPos> &ParsedCnfSet::getObjPosList() const
{
    return m_objPosList;
}

const ParsedCnfSetObjPos ParsedCnfSet::getObjPosByObjId(int32_t objId) const
{
    if (objId >= m_objPosList.length()) {
        return std::nullopt;
    }
    return m_objPosList[objId];
}



ParsedCnf::ParsedCnf(QString fileName, QByteArray rawData, QtKISS::Encoding encode):
    m_errCount(0),
    m_fileName(fileName),
    m_fkiss(new Fkiss())
{
    m_fileContentText = Encoding::toUtf8(rawData, encode);
    static QRegularExpression reNewline(R"#((\r\n|\r|\n))#");
    parseLines(m_fileContentText.split(reNewline));
}

ParsedCnf::~ParsedCnf()
{
    qDeleteAll(m_cels);
    qDeleteAll(m_sets);
    delete m_fkiss;
}

void ParsedCnf::error(const QString &msg)
{
    QString &line = m_lines[m_lineIdx];
    qCritical() << "[ERROR]" << m_fileName << ":" << m_lineIdx + 1 << ": " << msg << " ===>" << line;
}

void ParsedCnf::log(const QString &msg)
{
    QString &line = m_lines[m_lineIdx];
    qCritical() << "[LOG]" << m_fileName << ":" << m_lineIdx + 1 << ": " << msg << " ===>" << line;
}

void ParsedCnf::parseLines(const QStringList &lines)
{
    m_lines = lines;
    m_lineIdx = 0;
    m_sortedObjIdList.clear();
    /** With ;@ prefix */
    QList<FkissLineInfo> fkissScriptLines;
    // for (auto l : lines) {
    //     qDebug() << "=========" << l;
    // }
    for (m_lineIdx = 0; m_lineIdx < m_lines.length(); m_lineIdx++) {
        QString &line = m_lines[m_lineIdx];
        // qDebug() << "===============" << line;
        if (line.startsWith(";"_L1)) {
            if (line.startsWith(";@"_L1)) {
                fkissScriptLines.append({m_lineIdx + 1, line});
            } else {
                m_readableComments.append(line);
            }
        } else if (line.startsWith("("_L1)) {
            static QRegularExpression rePlayFieldSize(R"#(^\((\d+) *?, *?(\d+))#");
            QRegularExpressionMatch m = rePlayFieldSize.match(line);
            if (!m.hasMatch()) { error("Wrong playFieldSize format."); continue; }
            m_playFieldSize.setWidth(m.captured(1).toInt());
            m_playFieldSize.setHeight(m.captured(2).toInt());
        } else if (line.startsWith("%"_L1)) {
            static QRegularExpression reKcf(R"#(^%(\S+)\s*?(?:;(.+))?)#");
            QRegularExpressionMatch m = reKcf.match(line);
            if (!m.hasMatch()) { error("Wrong KCF file name format."); continue; }
            qDebug() << "CNF requires KCF " << m.captured(1).trimmed();
            m_kcfFileNames.append(m.captured(1).trimmed());
        } else if (line.startsWith("[")) {
            static QRegularExpression reOutsideColor(R"#(^\[(\d+))#");
            QRegularExpressionMatch m = reOutsideColor.match(line);
            if (!m.hasMatch()) { error("Wrong outsideColor format."); continue; }
            m_outsideColor = line.mid(1).toUInt();
        } else if (line.startsWith("#"_L1)) {
            // static QRegularExpression reCel(R"#(^#(\d+)(?:[.](\d+))?\s+(\S+)\s+(?:[*](\d+))?\s+(?:[:]([\d\s]+))?)#");
            static QRegularExpression reCel(R"#(^#(\d+)(?:[.](\d+))?\s+(\S+)(?:\s+[*]\s*?(\d+))?(?:\s+[:]\s*?([\d\s]+))?(?:;(.+))?)#");
            QRegularExpressionMatch m = reCel.match(line);
            if (!m.hasMatch()) { error("Wrong cel format."); continue; }
            int32_t objId = m.captured(1).toInt();
            if (!m_sortedObjIdList.contains(objId)) {
                m_sortedObjIdList.append(objId);
            }
            int32_t fix = m.captured(2).isEmpty() ? 0 : m.captured(2).toInt();
            QString celFileName = m.captured(3);
            int32_t kcfFileIndex =  m.captured(4).isEmpty() ? 0 :  m.captured(4).toInt();
            static QRegularExpression reSpace("\\s+");
            QStringList setIndexesStrList;
            if (!m.captured(5).isEmpty()) {
                setIndexesStrList = m.captured(5).trimmed().split(reSpace);
            }
            QList<int32_t> setIndexes {};
            for (const QString &x: std::as_const(setIndexesStrList)) {
                setIndexes.append(x.toInt());
            }

            QString rawComment = m.captured(6).trimmed();
            // qDebug() << QString("[LOG][ParsedCnf] { objId: %1, fix: %2, fileName: %3, kcfIdx: %4, sets: [%5], comment: %6 }")
            //     .arg(objId)
            //     .arg(fix)
            //     .arg(celFileName)
            //     .arg(kcfFileIndex)
            //     .arg(setIndexesStrList.join(","))
            //     .arg(comment);
            ParsedCnfCel *cnfCel = new ParsedCnfCel {objId, fix, celFileName, kcfFileIndex, setIndexes, rawComment};
            qDebug() << "[DEBUG][ParsedCnf]" << *cnfCel;
            m_cels.append(cnfCel);
        } else if (line.startsWith("$"_L1)) {
            static QRegularExpression reSetNo(R"#(^[$]\d+\s)#");
            if (!reSetNo.match(line).hasMatch()) { error("Wrong set line format."); continue; }
            startToParseSetLines();
            continue;
        }
    }
    std::sort(m_sortedObjIdList.begin(), m_sortedObjIdList.end(), std::less<int32_t>());
    qDebug() << m_sortedObjIdList;
    for (int i = 0; i< m_sets.length(); i++) {
        ParsedCnfSet *set = m_sets[i];
        // qDebug() << "===================================================================";
        qDebug() << QStringLiteral("[LOG][ParsedCnf] Set %1: Palette no: %2, Objects amounts: %3")
                    .arg(i)
                    .arg(set->paletteNoInKcf())
                    .arg(set->getObjPosList().length());
        // qDebug() << "===================================================================";
        // for (auto pos : set.getObjPosList()) {
        //     if (pos.has_value()) {
        //         qDebug() << "    " << pos.value();
        //     } else {
        //         qDebug() << "    *";
        //     }
        // }
    }
    qDebug() << m_readableComments;
    m_fkiss->compileScriptLines(fkissScriptLines);
    m_lines.clear();
}

void ParsedCnf::startToParseSetLines()
{
    QString line = m_lines[m_lineIdx].trimmed();
    qDebug() << "SET START ======" << line;
    QStringList segs = line.split(" "_L1);
    int32_t paletteNoInPaletteFile = segs[0].mid(1).toInt();
    ParsedCnfSet *set = new ParsedCnfSet { paletteNoInPaletteFile };
    static QRegularExpression rePos(R"#(^(\d+),(\d+)$)#");
    for (uint32_t i = 1; i < segs.length(); i++) {
        QString &seg = segs[i];
        if (seg == "*"_L1) {
            set->appendEmptyObjPos();
        } else if (QRegularExpressionMatch m = rePos.match(seg); m.hasMatch()) {
            set->appendObjPos(m.captured(1).toInt(), m.captured(2).toInt());
        } else {
            error("Wrong objPos format");
        }
    }
    while (m_lineIdx + 1 < m_lines.length()) {
        line = m_lines[m_lineIdx + 1];
        qDebug() << "======" << line;
        if (!line.startsWith(" "_L1)) { break; }
        segs = line.trimmed().split(" "_L1);
        for (uint32_t i = 0; i < segs.length(); i++) {
            QString &seg = segs[i];
            if (seg == "*"_L1) {
                set->appendEmptyObjPos();
            } else if (QRegularExpressionMatch m = rePos.match(seg); m.hasMatch()) {
                set->appendObjPos(m.captured(1).toInt(), m.captured(2).toInt());
            } else {
                error("Wrong objPos format");
            }
        }
        m_lineIdx++;
    }
    m_sets.append(set);  // QList::append() will copy this obj... So append here.
}

const QList<int32_t> &ParsedCnf::sortedObjIdList() const
{
    return m_sortedObjIdList;
}

uint32_t ParsedCnf::outsideBorderColor() const
{
    return m_outsideColor;
}

QString ParsedCnf::fileName() const
{
    return m_fileName;
}

QStringList ParsedCnf::kcfFileNames() const
{
    return m_kcfFileNames;
}

size_t ParsedCnf::kcfFilesAmount() const
{
    return m_kcfFileNames.size();
}

QList<ParsedCnfSet *> ParsedCnf::sets() const
{
    return m_sets;
}

QList<ParsedCnfCel *> ParsedCnf::cels() const
{
    return m_cels;
}

QSize ParsedCnf::playFieldSize() const
{
    return m_playFieldSize;
}

QDebug operator<<(QDebug debug, const ParsedCnfCel &cnfCel) {
    QStringList fmtSetIndexes;
    foreach (auto x , cnfCel.setsIndexes()) {
        fmtSetIndexes.append(QString::number(x));
    }
    return debug.noquote().nospace() << QString("ParsedCnfCel { objId: %1,    fix: %2,    fileName: %3,     kcfIdx: %4,     sets: [ %5 ],     comment: %6 }")
                                        .arg(cnfCel.objId())
                                        .arg(cnfCel.fix())
                                        .arg(cnfCel.celFileName())
                                        .arg(cnfCel.kcfFileIndex())
                                        .arg(fmtSetIndexes.join(", "))
                                        .arg(cnfCel.comment());
}
