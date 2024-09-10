#include "fkissvariant.h"


FkissVariant::FkissVariant(int value) : m_type(Type::Int), m_intValue(value) {}

FkissVariant::FkissVariant(const QString &value) : m_type(Type::String), m_stringValue(value) {}

int FkissVariant::getInt() const
{
    return m_intValue;
}

QString FkissVariant::getString() const
{
    return m_stringValue;
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
    }
    return debug.space();
}
