#include "parsedcnf.h"


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
    m_fileName(fileName)
{

}
