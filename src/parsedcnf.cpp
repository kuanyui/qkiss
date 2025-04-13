#include <QDebug>
#include <QRegularExpression>

#include "parsedcnf.h"
#include "encoding.h"
// Q_LOGGING_CATEGORY(CnfParser, "qkiss.parser.cnf")
using namespace Qt::Literals::StringLiterals;

ParsedCnfCel::ParsedCnfCel(int32_t objId, int32_t fix, QString celFileName, int32_t kcfFileIndex, QList<int32_t> setsIndexes, QString comment):
    m_objId(objId),
    m_fix(fix),
    m_celFileName(celFileName),
    m_kcfFileIndex(kcfFileIndex),
    m_setsIndexes()
{
    setComment(comment);
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

void ParsedCnfCel::setComment(QString rawComment)
{
    qDebug() << "SET COMMENT: " << rawComment;
    static QRegularExpression re(R"#(%t(\d+))#");
    QRegularExpressionMatch m = re.match(rawComment);
    if (m.hasMatch()) {
        int val = m.captured(1).toInt();
        if (val > 256) {
            val = 256;
        } else if (val < 0) {
            val = 0;
        }
        m_translucency = val;
        rawComment = rawComment.replace(re, ""_L1);
    }
    m_comment = removeQuotesAndNewlines(rawComment);
}

QString ParsedCnfCel::removeQuotesAndNewlines(QString &comment)
{
    comment = comment.trimmed();
    if (comment.startsWith("\"") && comment.endsWith("\"")) {
        comment = comment.slice(1, comment.length() - 2);
    }

    static QRegularExpression reNewline (R"#(\\n)#");
    comment.replace(reNewline, "\n");

    static QRegularExpression reQuote (R"#(\\")#");
    comment.replace(reQuote, "\"");

    static QRegularExpression reUseless (R"#(\\t")#");
    comment.replace(reUseless, "");

    return comment;
}

int16_t ParsedCnfCel::translucency() const
{
    return m_translucency;
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



ParsedCnf::ParsedCnf(QString fileName, QByteArray rawData, QKiSS::Encoding encode):
    m_errCount(0),
    m_fileName(fileName),
    m_fkiss(new Fkiss())
{
    qDebug().noquote() << "[Log] Parse CNF file \"%1\""_L1.arg(fileName);
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

void ParsedCnf::error(const QString &msg, int lineIdx)
{
    if (lineIdx == -1) {
        lineIdx = m_lineIdx;
    }
    QString &line = m_lines[lineIdx];
    qCritical() << "[ERROR]" << m_fileName << ":" << lineIdx + 1 << ": " << msg << " ===>" << line;
}

QString ParsedCnf::removeComment(const QString &line)
{
    return line.split(';').first();
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
        if (line.startsWith(":@"_L1) || line.startsWith(";@"_L1)) {    // Some lunetic files has typo, colon `:` instead of semicolon `;`...
           continue;
        } else if (line.startsWith(";"_L1) && !line.startsWith(";@"_L1)) {
            m_readableComments.append(line);
        } else if (line.startsWith("("_L1)) {
            static QRegularExpression rePlayFieldSize(R"#(^\((\d+) *?[,.*] *?(\d+))#");   // Actually, the standard spec is `(x,y`, but some CNF wrongly wrote as `(x.y` even `(x*y`...`
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
            // static QRegularExpression reCel(R"#(^#(\d+)(?:[.](\d+))?\s+(\S+)(?:\s+[*]\s*?(\d+))?(?:\s+[:]\s*?([\d\s]+))?(?:;(.+))?)#");

            // More tolerance for some common mistakes:
            //
            // - No spaces before `:`             #0.999 body011.cel:0 1 2 3 4 5 6 7 8 9; 身体
            // - No spacce before `:`             #63           BERET.CEL     *1:            6
            // - A `.` without fix number         #40.      Mess00A_.cel *2  :                  9 ;
            // - full-width `;` or spaces         #5	BOU1.CEL　     ；（制服の帽子　表）
            //   (NOTE: it seems that `\s` doesn't include full-width space `　`)
            //
            //                                     obj        fix         file               kcfIdx         setIdxArr             comment
            //                                      v
            static QRegularExpression reCel(R"#(^#(\d+)(?:[.](\d*))?\s+([^\s\t　*:]+)(?:\s*[*]\s*?(\d+))?(?:\s*[:]\s*?([\d\s]+))?(?:[;；](.*))?)#");
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
    qDebug() << "Readable comments in this CNF:\n" << m_readableComments;
    for (m_lineIdx = 0; m_lineIdx < m_lines.length(); m_lineIdx++) {
        QString &line = m_lines[m_lineIdx];
        // qDebug() << "===============" << line;
        if (line.startsWith(":@"_L1)) {    // Some lunetic files has typo, colon `:` instead of semicolon `;`...
           fkissScriptLines.append({m_lineIdx + 1, line});
           error("Wrong fkiss prefix `:@`, but for tolerance of legacy accepts it =>  "_L1 + line);
        } else if (line.startsWith(";@"_L1)) {
            fkissScriptLines.append({m_lineIdx + 1, line});
        }
    }
    m_fkiss->parseAndCacheFkissScript(fkissScriptLines);
    m_lines.clear();
}

void ParsedCnf::startToParseSetLines()
{
    QString line = removeComment(m_lines[m_lineIdx].trimmed());
    qDebug() << "SET START ======" << line;
    QStringList segs = line.split(" "_L1);
    int32_t paletteNoInPaletteFile = segs[0].mid(1).toInt();
    ParsedCnfSet *set = new ParsedCnfSet { paletteNoInPaletteFile };
    static QRegularExpression rePos(R"#(^([0-9-]+),([0-9-]+)$)#");   // some CNF contains x,y whose value < 0
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
        line = removeComment(m_lines[m_lineIdx + 1]);
        qDebug() << "(SET) ======" << line;
        if (line.trimmed().isEmpty()) {
            m_lineIdx++;
            continue;
        }
        if (!line.startsWith(" "_L1)) { break; }
        segs = line.trimmed().split(" "_L1);
        for (uint32_t i = 0; i < segs.length(); i++) {
            QString &seg = segs[i];
            if (seg == "*"_L1) {
                set->appendEmptyObjPos();
            } else if (QRegularExpressionMatch m = rePos.match(seg); m.hasMatch()) {
                set->appendObjPos(m.captured(1).toInt(), m.captured(2).toInt());
            } else {
                error("Wrong objPos format", m_lineIdx + 1);
            }
        }
        m_lineIdx++;
    }
    qDebug() << "(SET ENDS)";
    m_sets.append(set);  // QList::append() will copy this obj... So append here.
}

QString ParsedCnf::fileContentText() const
{
    return m_fileContentText;
}

QStringList ParsedCnf::readableComments() const
{
    return m_readableComments;
}

const QList<int32_t> &ParsedCnf::sortedObjIdList() const
{
    return m_sortedObjIdList;
}

uint32_t ParsedCnf::outsideBorderColor() const
{
    return m_outsideColor;
}

Fkiss *ParsedCnf::fkiss()
{
    return m_fkiss;
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
