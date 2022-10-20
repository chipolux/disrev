#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include <QPointer>
#include <QtQml>

class Entry;
class Container;

class ResourceManager : public QObject
{
    Q_OBJECT

  public:
    inline static QString D2Dir{u"C:\\Steam\\steamapps\\common\\Dishonored2\\base"_qs};

    explicit ResourceManager(QObject *parent = nullptr);

  signals:
    void errorOccured(QString error);
    void loadingFinished();
    void containersLoaded(int count);
    void entriesLoaded(int count);
    void searchResult(QPointer<Entry>);
    void extractResult(const QPointer<Entry> ref, QByteArray data);

  public slots:
    void loadIndexes();
    void search(const QString &query);
    void extract(const QPointer<Entry> ref);

  private:
    QList<Container *> m_containers;

    bool loadMasterIndex();
    bool loadChildIndexes();
};

#endif // RESOURCEMANAGER_H
