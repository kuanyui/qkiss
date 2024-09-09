#ifndef CNFFILE_H
#define CNFFILE_H

#include <QString>
#include <QStringList>
#include <QSize>
#include <QPoint>
#include <optional>
#include "enums.h"

#include <QLoggingCategory>

// Q_DECLARE_LOGGING_CATEGORY(CnfParser)

/*
 * Docstrings are mainly copied from:
 *
 * - https://otakuworld.com/kiss/kisshelp/DIRECTKISSKISS_General_Specification.htm
 */

/**
 * Represent a referenced .CEL (a line starts with #) in a parsed .CNF file
 *
 * https://www2s.biglobe.ne.jp/~yav/kiss/kissgsj.html
 * https://otakuworld.com/kiss/kisshelp/DIRECTKISSKISS_General_Specification_CNF.htm
 *
 * - Syntax: #<object number>[.<fix>] <cel filename> [*<KCF number>] [:<set number>]
 * - Declares a cel, describing the object of which it is a member, which KCF it uses, on what sets it appears, and its initial fix value.
 *
 * > Note: The order in which cel declarations are listed determine the order in which they are drawn on screen. Cels listed first in the CNF are drawn on top of cels listed later in the CNF.
 * @example
 * ```
 * #2 data1.cel          ; Object 2 uses image data1.cel on all sets.
 * #2 data1_part.cel     ; Object 2 also uses image data1_part.cel on all sets.
 * #3.5 data2.cel :2 3 4 ; Object 3 with fixed value of 5 uses image data2.cel
 *                       ; on sets 2, 3, and 4.
 * #4.255 data3.cel      ; Object 4 cannot be moved and uses image data3.cel on
 *                       ; all sets.
 * #5 data4.cel *1 :6    ; Object 5 uses image data4.cel and KCF #1 on set 6.
 * ```
 */
class ParsedCnfCel
{
public:
    ParsedCnfCel(int32_t objId, int32_t fix, QString celFileName, int32_t kcfFileIndex, QList<int32_t> setsIndexes, QString comment);
    int32_t objId() const;
    int16_t fix() const;
    QString celFileName() const;
    int16_t kcfFileIndex() const;
    QList<uint8_t> setsIndexes() const;
    QString comment() const;

private:
    /*! Cels with the same object ID are members of the same object and will move together as a unit. */
    int32_t m_objId;
    /**
     * - How strongly this object is fixed in place.
     *   -- A value of 0 means the object is freely moveable.
     *   -- A value less than 100 means the object cannot be moved but the fix value will reduce by 1 each time it is clicked
    */
    int16_t m_fix;
    /*! Describes the filename of the cel including filename extension.  */
    QString m_celFileName;
    /**
     * - Indicates which KCF that should be used to draw this cel.
     * - Different instances of the same cel may use different KCFs.
     * - Different cels within the same object may use different KCFs.
     * - For color-based (32-bit) cels, this value is unused and can be omitted.
     * - For palette-based (4-bit, 8-bit) cels, if this value is omitted, KCF #0 (i.e., the first KCF specified) is assumed.
     */
    int16_t m_kcfFileIndex;
    /**
     * The cel is drawn only in the sets (pages) specified.
     * Multiple sets may be listed, separated by spaces.
     * These values may be 0-9 (because the typical limit is 10 sets).
     * If omitted, this cel is drawn in all sets.
     */
    QList<uint8_t> m_setsIndexes;
    QString m_comment;
};

using ParsedCnfSetObjPos = std::optional<QPoint>;

/**
 *  Represent a set (a line starts with $) defined in a parsed .CNF file
 *
 * - Syntax: $<palette> [<xpos,ypos>...]
 * - Palette to use followed by a list of positions for each object declared in the CNF file that has been declared to be in this set.
 * - Traditionally, there are a maximum of 10 sets and most KiSS viewers adhere to this limit.
 * - A long description may be wrapped to the next line with the following lines beginning with a space to indicate the line is continued.
 *
 * > Note: The sets declarations can be overwritten by the KISS save function.
 * @example
 * ```
 * $2 192,11 * 56,176 55,21 259,62 15,24 375,63
 * $3 43,115 154,62 372,108 253,156 * * * 165,207
 *  * 162,198 * 119,56 152,44 * * *       ; Continued from previous line
 *  16,355 394,362 108,355 * * * 125,261  ; Continued from previous line
 * $0 192,11 * 56,176 55,21 259,62 15,24 375,63
 * ```
 */
class ParsedCnfSet
{
public:
    ParsedCnfSet(int32_t paletteNoInKcf);
    void appendObjPos(int32_t x, int32_t y);
    void appendEmptyObjPos();
    int32_t paletteNoInKcf() const;
    const QList<ParsedCnfSetObjPos> &getObjPosList() const;
    /** if returns nullopt, it means don't specify the object position. Just leave it as (0,0) */
    const ParsedCnfSetObjPos getObjPosByObjId(int32_t objId) const;
private:
    /**
     * - Initial palette for the set. This is the number of a palette within a KCF.
     * - The range is based on the KCF with the greatest number of palettes. The palette number starts at 0 and goes up to one less than the greatest number of palettes.
     * - For example, if the greatest number of palettes is 8, then the range for a palette number is 0 through 7. If the greatest number of palettes is 20, then the range is 0 through 19.
     */
    int32_t m_paletteNoInKcf;
    /**
     * - Position of object relative to the top-left corner of the play field.
     * - Positions are listed by order of object ID number.
     * - No spaces may appear between xpos, ypos, and the comma.
     * - An aserisk ('*') means no position data is given for this object. If the object exists, a position of 0,0 is assumed.
     */
    QList<ParsedCnfSetObjPos> m_objPosList;
};


/**
 * Represent a parsed .CNF file.
 */
class ParsedCnf
{
public:
    /** fileName is the relative file name in archive file. */
    ParsedCnf(QString fileName, QByteArray rawData, QtKISS::Encoding encode);
    ~ParsedCnf();
    QSize playFieldSize() const;
    QList<ParsedCnfCel *> cels() const;
    QList<ParsedCnfSet *> sets() const;
    /**
     * KCF file index is the index of a KCF in a CNF declaration (KCF files used by this CNF).
    */
    QString getKcfNameByKcfIndex(uint8_t kcfFileIndex);
    QStringList kcfFileNames() const;
    /** KCF files amount used in this CNF */
    size_t kcfFilesAmount() const;
    QString fileName() const;
    uint32_t outsideBorderColor() const;

    QList<int32_t> sortedObjIdList() const;

private:
    void log(const QString &line);
    void warn(const QString &line);
    void error(const QString &msg);
private:
    void parseLines(const QStringList &lines);
    void startToParseSetLines();
    uint32_t m_errCount;
    /** Temporary storage when parsing. */
    QStringList m_lines;
    /** currently parsing line */
    u_int32_t m_lineIdx;

    // Basic
    QString m_fileName;
    /** UTF8 */
    QString m_fileContentText;
    // Parsed
    QStringList m_readableComments;
    /**
     * - Syntax: (<horizontal size>,<vertical size>
     * - Defines the size of the play field. If omitted, "(448,320" is assumed for KISS v2.18 compatibility.
     */
    QSize m_playFieldSize;
    /**
     * - Syntax: %<palette filename>
     * - Declares a palette file. Palette files are numbered as 0, 1, 2... by order listed.
     * - All colors in the palette file #0 are used. Color #0 in palette file #0 is used as the background color.
     *   > Color #0 in subsequent palette files is unused.
     *
     * The total number of colors must be 256 or less across all palettes in all KCF palette files in viewers without Enhanced Palette Support.
     */
    QStringList m_kcfFileNames;
    /**
     * - Syntax: [<border color index>
     * - This color is the index of a palette entry in palette #0 of the first KCF. The screen area outside of the play field is filled with this color.
     *
     *       Example:
     *       [12     ; Set border color to color at index 12 in palette #0.
     */
    u_int32_t m_outsideColor = 0;  // if not specified, set to 0.
    QList<ParsedCnfCel *> m_cels;
    QList<ParsedCnfSet *> m_sets;
    /** ObjectID may be not continuous, and CNF's set (starts with $) is sorted by the value of ObjectID. */
    QList<int32_t> m_sortedObjIdList;
};


#endif // CNFFILE_H
