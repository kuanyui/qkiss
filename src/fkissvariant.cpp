#include "fkissvariant.h"


FkissVariant::FkissVariant(int value) : m_type(Type::Int), m_intValue(value) {}

FkissVariant::FkissVariant(const QString &value) : m_type(Type::String), m_stringValue(value) {}

FkissVariant FkissVariant::fromString(const QString &value)
{
    return FkissVariant(value);
}

FkissVariant FkissVariant::fromInt(const QString &value)
{
    return FkissVariant(value.toInt());
}

FkissVariant FkissVariant::fromObjectId(const QString &value)
{
    if (value.startsWith('#')) {
        int id = value.mid(1).toInt();
        FkissVariant tmp = (id);
        tmp.m_type = Type::Object;
        return tmp;
    } else {
        int id = value.toInt();
        FkissVariant tmp = (id);
        tmp.m_type = Type::Object;
        return tmp;
    }
}


int FkissVariant::getInt() const
{
    return m_intValue;
}

QString FkissVariant::getString() const
{
    return m_stringValue;
}

int FkissVariant::getObjectId() const
{
    return m_intValue;
}

FkissVariant::Type FkissVariant::type() const { return m_type; }


QDebug operator<<(QDebug debug, const FkissVariant &arg) {
    switch (arg.type()) {
    case FkissVariant::Type::Int:
        debug.nospace() << arg.getInt();
        break;
    case FkissVariant::Type::String:
        debug.nospace() << arg.getString();
        break;
    case FkissVariant::Type::Object:
        debug.nospace() << "#" << arg.getInt();
        break;
    }
    return debug.space();
}
