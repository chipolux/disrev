#ifndef DECL_H
#define DECL_H

#include <QObject>
#include <QtQml>

#include "qtutils.h"

namespace decl
{

// these are used for passing around and serializing entities when a full QObject
// version is not a good fit (between threads, etc.)
using Entry = QPair<QString, QVariant>;
using Scope = QList<Entry>;

class EntityEntry : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Backend only.")

    Q_PROPERTY(QObject *scope READ parent CONSTANT)

  public:
    explicit EntityEntry(const Entry &entry, QObject *parent);
    void toByteArray(QByteArray &stream, const int &depth = 0) const;

  public slots:
    void deleteEntry(decl::EntityEntry *entry);

  private:
    RW_PROP(QString, key, setKey)
    RW_PROP(QString, value, setValue)
    RW_PROP(QList<EntityEntry *>, entries, setEntries)
    RW_PROP(bool, isKiscule, setIsKiscule)
};

class Entity : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Backend only.")

  public:
    explicit Entity(const Scope &scope, QObject *parent);
    bool isValid() const;
    void toByteArray(QByteArray &stream) const;

  public slots:
    void deleteEntry(decl::EntityEntry *entry);

  private:
    RW_PROP(QString, entityId, setEntityId)
    RW_PROP(QString, entityType, setEntityType)
    RW_PROP(QString, definitionType, setDefinitionType)
    RW_PROP(QList<EntityEntry *>, entries, setEntries)
};

// basically all .decl files are a sequence of scopes, this is catered to .entities
QString parse(const QByteArray &data, QList<Scope> &entities);

// format a list of entities to be written to a .entities file
void write(const QList<Entity *> &entities, QByteArray &data);

}; // namespace decl

#endif // DECL_H
