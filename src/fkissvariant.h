#ifndef FKISSVARIANT_H
#define FKISSVARIANT_H
#include <QString>
#include <QDebug>
#include <QHash>

/** Used by FKiSS script */
class FkissVariant
{
public:
    enum class Type {
        Int = 1,      // 2147483647
        String = 2,   // "Hello!"
        Cel = 4,      // "RIBBON.CEL"
        Object = 8,   // #123
        // Group,    // !groupName   // Fkiss4 only, unused currently
    };
private:
    FkissVariant(Type type, int value);
    FkissVariant(Type type, const QString& value);
public:
    static FkissVariant fromString(const QString &value);
    static FkissVariant fromCelName(const QString &value);
    static FkissVariant fromInt(const QString &value);
    static FkissVariant fromInt(const int &value);
    static FkissVariant fromObjectId(const QString &value);
    static FkissVariant fromObjectId(int value);
    bool isInt() const;
    bool isString() const;
    bool isCelName() const;
    /// Workaround for convenience.
    bool isStringOrCelName() const;
    bool isObjectId() const;
    int getInt() const;
    QString getString() const;
    QString getCelName() const;
    int getObjectId() const;
    Type type() const;
    operator QString() const;
    bool strictEq(const FkissVariant &other) const;
    /// Make FkissVariant::operator== treat Cel and String as same type, for convenience.
    /// Because currently, for String and Cel, when parsing FKiSS script, I doesn't do extra type checking for each event/action and cast to corresponding type,
    /// but when interpreting in FKiSS runtime, a type checking is done.
    /// TODO: do type checking everywhere for consistency?
    bool operator==(const FkissVariant &other) const;
    bool operator!=(const FkissVariant &other) const;
private:
    Type m_type;
    int m_intValue;
    QString m_stringValue;
};


QDebug operator<<(QDebug debug, const FkissVariant& arg);
size_t qHash(const FkissVariant& key, size_t seed = 0);


#endif // FKISSVARIANT_H

