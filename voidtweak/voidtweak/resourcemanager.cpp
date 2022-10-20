#include "resourcemanager.h"

#include <QDataStream>
#include <QDebug>
#include <QDir>

#include "container.h"
#include "entry.h"
#include "zutils.h"

ResourceManager::ResourceManager(QObject *parent)
    : QObject{parent}
{
}

void ResourceManager::loadIndexes()
{
    qInfo() << "Started loading...";

    // First load master index to determine containers and their resource files
    qDeleteAllLater(m_containers);
    if (!loadMasterIndex())
        return;
    emit containersLoaded(m_containers.count());

    // Then load the entries from the indexes of each container
    if (!loadChildIndexes())
        return;
    int entryCount = 0;
    for (const auto c : m_containers) {
        entryCount += c->entries.count();
    }
    emit entriesLoaded(entryCount);

    qInfo() << "Finished loading...";
    emit loadingFinished();
}

bool ResourceManager::loadMasterIndex()
{
    QDir dir(D2Dir);
    if (!dir.exists(u"master.index"_qs)) {
        qWarning() << "No master.index found!";
        emit errorOccured(u"No master.index found!"_qs);
        return false;
    }
    QFile f(dir.absoluteFilePath(u"master.index"_qs));
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open:" << f.fileName();
        emit errorOccured(u"Failed to open: %1"_qs.arg(f.fileName()));
        return false;
    }
    QDataStream in(&f);
    in.setByteOrder(QDataStream::BigEndian);

    quint32 magic;
    in >> magic;
    if (magic != 0x04534552) {
        qWarning() << "Invalid magic:" << magic;
        emit errorOccured(u"Invalid magic: %1"_qs.arg(magic));
        f.close();
        return false;
    }

    in.skipRawData(2); // unknown, always 0x0002?
    quint16 indexCount;
    in >> indexCount;
    for (quint32 i = 0; i < indexCount; i++) {
        in.setByteOrder(QDataStream::LittleEndian);
        Container *c = new Container(this);
        c->dir = dir.absolutePath();
        quint32 strLen;
        char *str = nullptr;
        in.readBytes(str, strLen);
        c->path = QString::fromUtf8(str, strLen);
        delete[] str;
        in.readBytes(str, strLen);
        c->resources.append(QString::fromUtf8(str, strLen));
        delete[] str;
        m_containers.append(c);
        in.setByteOrder(QDataStream::BigEndian);
    }

    quint32 resourceCount;
    in >> resourceCount;
    for (quint32 i = 0; i < resourceCount; i++) {
        in.setByteOrder(QDataStream::LittleEndian);
        quint32 strLen;
        char *str = nullptr;
        in.readBytes(str, strLen);
        in.setByteOrder(QDataStream::BigEndian);
        quint16 index;
        in >> index;
        m_containers[index]->resources.append(QString::fromUtf8(str, strLen));
        delete[] str;
    }

    quint32 sharedRscCount;
    in >> sharedRscCount;
    for (quint32 i = 0; i < sharedRscCount; i++) {
        quint32 sharedIndexCount;
        in >> sharedIndexCount;
        QList<quint16> indexes;
        for (quint32 k = 0; k < sharedIndexCount; k++) {
            quint16 idx;
            in >> idx;
            indexes << idx;
        }
        in.setByteOrder(QDataStream::LittleEndian);
        quint32 strLen;
        char *str = nullptr;
        in.readBytes(str, strLen);
        const QString rsc = QString::fromUtf8(str, strLen);
        delete[] str;
        for (auto c : indexes) {
            m_containers[c]->resources.append(rsc);
        }
        in.setByteOrder(QDataStream::BigEndian);
    }

    f.close();
    return true;
}

bool ResourceManager::loadChildIndexes()
{
    for (int ci = 0; ci < m_containers.count(); ci++) {
        Container *c = m_containers[ci];
        QFile f(c->pathAbsolute());
        if (!f.open(QFile::ReadOnly)) {
            qWarning() << "Failed to open:" << f.fileName();
            emit errorOccured(u"Failed to open: %1"_qs.arg(f.fileName()));
            return false;
        }
        QDataStream in(&f);
        in.setByteOrder(QDataStream::BigEndian);

        quint32 magic;
        in >> magic;
        if (magic != 0x05534552) {
            qWarning() << "Invalid magic:" << magic;
            emit errorOccured(u"Invalid magic: %1"_qs.arg(magic));
            f.close();
            return false;
        }
        in.skipRawData(28); // unknown, seems to be padding

        quint32 entryCount;
        int entriesAdded = 0;
        in >> entryCount;
        for (quint32 ei = 0; ei < entryCount; ei++) {
            Entry *e = new Entry(ci, c);
            e->indexPos = f.pos();
            in >> e->id;
            in.setByteOrder(QDataStream::LittleEndian);
            quint32 strLen;
            char *str = nullptr;
            in.readBytes(str, strLen);
            e->type = QString::fromUtf8(str, strLen);
            delete[] str;
            in.readBytes(str, strLen);
            e->src = QString::fromUtf8(str, strLen);
            delete[] str;
            in.readBytes(str, strLen);
            e->dst = QString::fromUtf8(str, strLen);
            delete[] str;
            in.setByteOrder(QDataStream::BigEndian);
            in >> e->resourcePos;
            in >> e->size;
            in >> e->sizePacked;
            in.skipRawData(6); // unknown, always null?
            in >> e->flags1;
            in >> e->flags2;
            // skip empty entries (deleted?)
            if (e->size == 0 || e->sizePacked == 0)
                continue;
            c->entries.append(e);
            e->entry = entriesAdded++;
        }

        qDebug() << "Kept" << entriesAdded << "of" << entryCount << "entries in" << c->path;
        f.close();
    }
    return true;
}

void ResourceManager::search(const QString &query)
{
    qInfo() << "Searching for:" << query;
    for (const auto c : m_containers) {
        for (const auto e : c->entries) {
            if (e->src.contains(query) || e->dst.contains(query)) {
                emit searchResult(e);
            }
        }
    }
    qInfo() << "Finished searching...";
    emit loadingFinished();
}

void ResourceManager::extract(const QPointer<Entry> ref)
{
    Container *c = nullptr;
    if (m_containers.count() > ref->container) {
        c = m_containers[ref->container];
    } else {
        qWarning() << "Invalid container index:" << ref->container;
        emit errorOccured(u"Invalid container, try reloading indexes!"_qs);
        return;
    }
    Entry *e = nullptr;
    if (c->entries.count() >= ref->entry) {
        e = c->entries[ref->entry];
    } else {
        qWarning() << "Invalid entry index:" << ref->entry;
        emit errorOccured(u"Invalid entry, try reloading indexes!"_qs);
        return;
    }
    qInfo() << "Starting extraction of" << ref;
    QFile f(c->resourcePath(e->flags2));
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open:" << f.fileName();
        emit errorOccured(u"Failed to open resource file: %1"_qs.arg(f.fileName()));
        return;
    }
    if (!f.seek(e->resourcePos)) {
        f.close();
        qWarning() << "Failed to seek to resource position...";
        emit errorOccured(u"Failed to read resource file, check for corrupt files!"_qs);
        return;
    }
    QByteArray rawData = f.read(e->sizePacked);
    f.close();
    if (rawData.size() != e->sizePacked) {
        qWarning() << "Failed to read all data from resource file...";
        emit errorOccured(u"Failed to read resource file, check for corrupt files!"_qs);
        return;
    }
    QByteArray result;
    if (e->size != e->sizePacked) {
        if (!zutils::inflt(rawData, result)) {
            qWarning() << "Failed to decompress data...";
            emit errorOccured(u"Failed to decompress asset, check for corrupt files!"_qs);
            return;
        }
    } else {
        result = rawData;
    }
    qInfo() << "Finished extraction," << result.size() << "bytes!";
    emit extractResult(ref, result);
}
