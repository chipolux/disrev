#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include <QPointer>
#include <QtQml>

#include "bwm.h"
#include "decl.h"

class Entry;
class Container;

class ResourceManager : public QObject
{
    Q_OBJECT

  public:
    explicit ResourceManager(QObject *parent = nullptr);

  signals:
    void statusChanged(bool busy, QString error);
    void indexesLoaded(int containerCount, int entryCount);
    void searchResult(const QPointer<Entry> entry);
    void extractResult(const QPointer<Entry> ref, QByteArray data);
    void entitiesLoaded(const QPointer<Entry> ref, QList<decl::Scope> entities);
    void bwmLoaded(const QPointer<Entry> ref, QList<bwm::PODObject> objects);

  public slots:
    void loadIndexes();
    void search(const QString &query);
    void extractEntry(const QPointer<Entry> ref);
    void insertEntry(const QPointer<Entry> ref, QByteArray data);
    void exportEntry(const QPointer<Entry> ref, QUrl path);
    void importEntry(const QPointer<Entry> ref, QUrl path);
    void loadEntities(const QPointer<Entry> ref);
    void exportAllEntries(QUrl path);
    void loadBwm(const QPointer<Entry> ref);
    void saveObject(const QPointer<Entry> ref, bwm::PODObject obj);
    void saveObjects(const QPointer<Entry> ref, QList<bwm::PODObject> objects);

  private:
    QList<Container *> m_containers;

    bool loadMasterIndex();
    bool loadChildIndexes();

    const Container *container(const QPointer<Entry> ref);
    const Entry *entry(const Container *c, const QPointer<Entry> ref);
    bool extract(const QPointer<Entry> ref, QByteArray &data);
    bool insert(const QPointer<Entry> ref, QByteArray &data);
};

#endif // RESOURCEMANAGER_H
