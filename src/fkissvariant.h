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
        Int,      // 2147483647
        String,   // "Hello!"
        // Cel,      // "RIBBON.CEL"
        Object,   // #123
        // Group,    // !groupName   // Fkiss4 only, unused currently
    };
private:
    FkissVariant(int value);
    FkissVariant(const QString& value);
public:
    static FkissVariant fromString(const QString &value);
    static FkissVariant fromInt(const QString &value);
    static FkissVariant fromObjectId(const QString &value);
    int getInt() const;
    QString getString() const;
    int getObjectId() const;
    Type type() const;
    operator QString() const;
    bool operator==(const FkissVariant &other) const;
private:
    Type m_type;
    int m_intValue;
    QString m_stringValue;
};


QDebug operator<<(QDebug debug, const FkissVariant& arg);
size_t qHash(const FkissVariant& key, size_t seed = 0);


#endif // FKISSVARIANT_H

