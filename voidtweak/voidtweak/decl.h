#ifndef DECL_H
#define DECL_H

#include <QList>
#include <QPair>
#include <QString>
#include <QVariant>

namespace decl
{

// the order of values within a scope seems important, so we preserve them
// as Key-Value pairs (Entry) within a list (Scope), the value in an Entry can
// be (and often is) a Scope
using Entry = QPair<QString, QVariant>;
using Scope = QList<Entry>;

struct Entity {
    QString entityId;
    QString entityType;
    QString definitionType;
    Scope data;

    explicit Entity(const Scope &scope)
        : data(scope)
    {
        for (const auto &e : scope) {
            if (e.first == u"entityId"_qs) {
                entityId = e.second.toString();
            } else if (e.first == u"entityType"_qs) {
                entityType = e.second.toString();
            } else if (e.first == u"definitionType"_qs) {
                definitionType = e.second.toString();
            }
        }
    }
    bool isValid() const
    {
        return !entityId.isEmpty() && !entityType.isEmpty() && !definitionType.isEmpty();
    }
};

// basically all .decl files are a sequence of scopes, this is catered to .entities
QString parse(const QByteArray &data, QList<Scope> &entities);

}; // namespace decl

#endif // DECL_H
