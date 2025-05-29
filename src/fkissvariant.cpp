#include "fkissvariant.h"


FkissVariant::FkissVariant(Type type, int value) : m_type(type), m_intValue(value) {}

FkissVariant::FkissVariant(Type type, const QString &value) : m_type(type), m_stringValue(value) {}

const char *FkissVariant::typeToString(Type type) {
    switch (type) {
    case Type::Int: return "Integer";
    case Type::String: return "String";
    case Type::Cel: return "Cel Name";
    case Type::Object: return "Object ID";
    }
    return "";
}

FkissVariant FkissVariant::fromString(const QString &value)
{
    return FkissVariant(Type::String, value);
}

FkissVariant FkissVariant::fromCelName(const QString &value)
{
    return FkissVariant(Type::Cel, value);
}

FkissVariant FkissVariant::fromInt(const QString &value)
{
    return FkissVariant(Type::Int, value.toInt());
}

FkissVariant FkissVariant::fromInt(const int &value)
{
    return FkissVariant(Type::Int, value);
}

FkissVariant FkissVariant::fromObjectId(const QString &value)
{
    if (value.startsWith('#')) {
        int id = value.mid(1).toInt();
        return FkissVariant { Type::Object, id };
    } else {
        int id = value.toInt();
        return FkissVariant { Type::Object, id };
    }
}

FkissVariant FkissVariant::fromObjectId(int value)
{
    return FkissVariant { Type::Object, value };
}

bool FkissVariant::isInt() const
{
    return m_type == Type::Int;
}

bool FkissVariant::isString() const
{
    return m_type == Type::String;
}

bool FkissVariant::isCelName() const
{
    return m_type == Type::Cel;
}

bool FkissVariant::isStringOrCelName() const
{
    return isString() || isCelName();
}

bool FkissVariant::isObjectId() const
{
    return m_type == Type::Object;
}

int FkissVariant::getInt() const
{
    return m_intValue;
}

QString FkissVariant::getString() const
{
    return m_stringValue;
}

QString FkissVariant::getCelName() const
{
    return m_stringValue;
}

int FkissVariant::getObjectId() const
{
    return m_intValue;
}

FkissVariant::Type FkissVariant::type() const { return m_type; }

bool FkissVariant::strictEq(const FkissVariant &other) const
{
    if (m_type != other.type()) { return false; }
    switch (m_type) {
    case Type::Int: return getInt() == other.getInt();
    case Type::Object: return getObjectId() == other.getObjectId();
    case Type::String: return getString() == other.getString();
    case Type::Cel: return getString() == other.getString();
    default: return false;
    }
}

bool FkissVariant::operator==(const FkissVariant &other) const
{
    switch (m_type) {
    case Type::Int:
        return m_type == other.type() && getInt() == other.getInt();
    case Type::Object:
        return m_type == other.type() && getObjectId() == other.getObjectId();
    case Type::String:
    case Type::Cel:
        // FIXME: Some CNF use different cases in CEL file declaraction against FKiSS Script... e.g. `#0 BODY.CEL` and `map("body.cel")`
        return (other.type() == Type::String || other.type() == Type::Cel) &&
                getString() == other.getString();
    default: return false;
    }
}
bool FkissVariant::operator!=(const FkissVariant &other) const
{
    return !(*this == other);
}


QDebug operator<<(QDebug debug, const FkissVariant &arg) {
    switch (arg.type()) {
    case FkissVariant::Type::Int:
        debug.nospace() << arg.getInt();
        break;
    case FkissVariant::Type::String:
        debug.nospace() << arg.getString();
        break;
    case FkissVariant::Type::Cel:
        debug.nospace() << arg.getCelName();
        break;
    case FkissVariant::Type::Object:
        debug.nospace() << "#" << arg.getInt();
        break;
    }
    return debug.space();
}

size_t qHash(const FkissVariant &key, size_t seed)
{
    switch (key.type()) {
    case FkissVariant::Type::String: return qHash(key.getString(), seed);
    case FkissVariant::Type::Cel: return qHash(key.getCelName(), seed);
    case FkissVariant::Type::Int: return qHash(key.getInt(), seed);
    case FkissVariant::Type::Object: return qHash(key.getObjectId(), seed);
    default: return qHash(0, seed);
    }
}
