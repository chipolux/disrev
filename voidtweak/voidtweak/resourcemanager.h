#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>

struct Entry {
    qint64 indexPos;
    quint32 id;
    QString type;
    QString src;
    QString dst;
    quint64 resourcePos;
    quint32 size;
    quint32 sizePacked;
    quint16 flags1;
    quint16 flags2;
};
QDebug operator<<(QDebug d, const Entry &e);

struct Container {
    QString dir;
    QString path;
    QList<QString> resources;
    QList<Entry> entries;

    QString pathAbsolute() const { return u"%1\\%2"_qs.arg(dir, path); }
};
QDebug operator<<(QDebug d, const Container &c);

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
    void searchResult(quint32 id, QString src, QString dst, quint32 size);

  public slots:
    void loadIndexes();
    void search(const QString &query);

  private:
    QList<Container> m_containers;

    bool loadMasterIndex();
    bool loadChildIndexes();
};

#endif // RESOURCEMANAGER_H
