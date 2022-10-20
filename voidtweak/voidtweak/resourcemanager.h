#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include <QPointer>
#include <QtQml>

#include "qtutils.h"

class Entry : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Backend only.")

    MBC_PROP(int, container)
    MBC_PROP(int, entry)
    MBC_PROP(qint64, indexPos)
    MBC_PROP(quint32, id)
    MBC_PROP(QString, type)
    MBC_PROP(QString, src)
    MBC_PROP(QString, dst)
    MBC_PROP(quint64, resourcePos)
    MBC_PROP(quint32, size)
    MBC_PROP(quint32, sizePacked)
    MBC_PROP(quint16, flags1)
    MBC_PROP(quint16, flags2)

  public:
    explicit Entry(const int &container, QObject *parent);
    explicit Entry(QPointer<Entry> other, QObject *parent);
    QString srcSuffix() const { return QFileInfo(src).suffix(); }
    QString dstSuffix() const { return QFileInfo(dst).suffix(); }
};
bool operator==(const Entry &left, const Entry &right);
QDebug operator<<(QDebug d, const Entry *e);

class Container : public QObject
{
    Q_OBJECT

    MBC_PROP(QString, dir)
    MBC_PROP(QString, path)
    MBC_PROP(QList<QString>, resources)
    MBC_PROP(QList<Entry *>, entries)
    Q_PROPERTY(QString pathAbsolute READ pathAbsolute CONSTANT)

  public:
    explicit Container(QObject *parent);
    QString pathAbsolute() const { return u"%1\\%2"_qs.arg(dir, path); }
    QString resourcePath(const quint16 &flags) const
    {
        // we always have 1 shared resource for Dishonored 2, but may be more
        // in other games or with older versions?
        QString result;
        const int index = flags & 0x8000 ? resources.count() - 1 : flags >> 2;
        if (resources.count() > index) {
            result = QDir(dir).absoluteFilePath(resources[index]);
        }
        return result;
    }
};
QDebug operator<<(QDebug d, const Container *c);

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
