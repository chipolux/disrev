#include "decl.h"

#include <QDebug>
#include <QVariant>

namespace decl
{

EntityEntry::EntityEntry(const Entry &entry, QObject *parent)
    : QObject(parent)
    , m_key(entry.first)
    , m_value()
    , m_entries()
    , m_isKiscule(false)
{
    if (entry.second.canConvert<Scope>()) {
        if (entry.first == u"m_kiscule"_qs) {
            m_isKiscule = true;
        }
        const Scope scope = entry.second.value<Scope>();
        for (const auto &e : scope) {
            m_entries.append(new EntityEntry(e, this));
        }
    } else {
        m_value = entry.second.toString();
    }
}

void EntityEntry::toByteArray(QByteArray &stream, const int &depth) const
{
    const QString prefix(depth, '\t');
    if (!m_value.isEmpty()) {
        stream.append(u"%1%2 = %3;\n"_qs.arg(prefix, m_key, m_value).toUtf8());
    } else if (!m_entries.isEmpty()) {
        stream.append(u"%1%2 = {\n"_qs.arg(prefix, m_key).toUtf8());
        for (const auto e : m_entries) {
            e->toByteArray(stream, depth + 1);
        }
        stream.append(u"%1}\n"_qs.arg(prefix).toUtf8());
    } else {
        // NOTE: some entries in the game are scopes with no child entries
        //       this just places them back in...
        stream.append(u"%1%2 = {}\n"_qs.arg(prefix, m_key).toUtf8());
    }
}

QVariantMap EntityEntry::toMap() const
{
    QVariantMap data;
    if (isLeaf()) {
        qWarning() << "Tried to convert a leaf to a map:" << m_key << "=" << m_value;
        return data;
    }

    for (const auto entry : m_entries) {
        if (entry->isLeaf()) {
            data.insert(entry->key(), entry->value());
        } else {
            data.insert(entry->key(), entry->toMap());
        }
    }

    return data;
}

void EntityEntry::deleteEntry(decl::EntityEntry *entry)
{
    if (m_entries.contains(entry)) {
        entry->deleteLater();
        m_entries.removeAll(entry);
        emit entriesChanged(m_entries);
    }
}

void EntityEntry::addEntry()
{
    m_entries.append(new EntityEntry(Entry(), this));
    emit entriesChanged(m_entries);
}

Entity::Entity(const Scope &scope, QObject *parent)
    : QObject(parent)
    , m_entityId()
    , m_entityType()
    , m_definitionType()
    , m_entries()
{
    for (const auto &e : scope) {
        if (e.first == u"entityId"_qs) {
            m_entityId = e.second.toString();
        } else if (e.first == u"entityType"_qs) {
            m_entityType = e.second.toString();
        } else if (e.first == u"definitionType"_qs) {
            m_definitionType = e.second.toString();
        } else {
            m_entries.append(new EntityEntry(e, this));
        }
    }
}

bool Entity::isValid() const
{
    return !m_entityId.isEmpty() && !m_entityType.isEmpty() && !m_definitionType.isEmpty();
}

void Entity::toByteArray(QByteArray &stream) const
{
    stream.append(u"%1 {\n\t%2 %3 {\n"_qs.arg(m_definitionType, m_entityType, m_entityId).toUtf8());
    for (const auto e : m_entries) {
        e->toByteArray(stream, 2);
    }
    stream.append("\t}\n}\n");
}

void Entity::deleteEntry(decl::EntityEntry *entry)
{
    if (m_entries.contains(entry)) {
        entry->deleteLater();
        m_entries.removeAll(entry);
        emit entriesChanged(m_entries);
    }
}

void Entity::addEntry()
{
    m_entries.append(new EntityEntry(Entry(), this));
    emit entriesChanged(m_entries);
}

QString parse(const QByteArray &data, QList<Scope> &entities)
{
    qInfo() << "Started parsing entities...";
    if (data.first(9) != QByteArrayLiteral("Version 6")) {
        return u"Unknown header!"_qs;
    }
    QByteArray token;
    QList<QByteArray> tokens;
    QList<Scope> stack;
    bool inEscape = false;
    bool inString = false;
    for (int i = 9; i < data.size(); ++i) {
        const auto c = data[i];
        // shortcut escape sequences
        if (inEscape) {
            token.append(c);
            inEscape = false;
            continue;
        } else if (c == '\\') {
            // keep escape since we might write this back out unmodified
            token.append(c);
            inEscape = true;
            continue;
        }
        // shortcut characters within a string and string toggles
        if (inString || c == '"' || c == '\'') {
            token.append(c);
            if (c == '"' || c == '\'') {
                inString = !inString;
            }
            continue;
        }
        // detect end of token and insert it into token cache
        if (!token.isEmpty() && QByteArrayLiteral(" \t\r\n={};").contains(c)) {
            tokens.append(token);
            token.clear();
        }
        switch (c) {
        // skip these control characters
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case '=': {
            break;
        }
        // handle value end marker
        case ';': {
            if (stack.isEmpty()) {
                return u"Missing stack frame for key-value pair @ %1"_qs.arg(i);
            }
            if (tokens.isEmpty()) {
                return u"Missing tokens for key-value pair @ %1"_qs.arg(i);
            }
            // NOTE: some values have a key that is made up of 2+ tokens
            // we just combine them into 1 key string for now
            QByteArray value = tokens.takeLast();
            const int keyStart = stack.count() - 1;
            const int keyEnd = tokens.count() - 1;
            QByteArray key;
            for (int ki = keyEnd; ki >= keyStart; ki--) {
                key.prepend((ki == keyStart ? "" : " ") + tokens.takeAt(ki));
            }
            stack.last().append({QString(key), QString(value)});
            break;
        }
        // handle start of nested scope
        case '{': {
            if (stack.isEmpty() && tokens.count() == 3) {
                stack.append({
                    {u"entityId"_qs, QString(tokens.takeLast())},
                    {u"entityType"_qs, QString(tokens.takeLast())},
                    {u"definitionType"_qs, QString(tokens.takeLast())},
                });
            } else if (!stack.isEmpty()) {
                stack.append(Scope());
            }
            break;
        }
        // handle exiting nested scope
        case '}': {
            if (stack.count() == 1 && tokens.isEmpty()) {
                entities.append(stack.takeLast());
            } else if (stack.count() > 1 && !tokens.isEmpty()) {
                QVariant scope = QVariant::fromValue(stack.takeLast());
                stack.last().append({QString(tokens.takeLast()), scope});
            }
            break;
        }
        default: {
            token.append(c);
            break;
        }
        }
    }
    if (!stack.isEmpty() || !tokens.isEmpty()) {
        return u"Bad entities, leftover data!"_qs;
    }
    qInfo() << "Loaded entities:" << entities.count();
    return {};
}

void write(const QList<Entity *> &entities, QByteArray &stream)
{
    qInfo() << "Started writing entities...";
    stream.append("Version 6\n");
    for (const auto e : entities) {
        if (!e->isValid()) {
            continue;
        }
        e->toByteArray(stream);
    }
    qInfo() << "Wrote entities:" << stream.size();
}

} // namespace decl
