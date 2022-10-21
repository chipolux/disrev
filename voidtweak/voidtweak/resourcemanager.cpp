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
        QFile f(c->indexPath());
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

void ResourceManager::extractEntry(const QPointer<Entry> ref)
{
    QByteArray output;
    if (extract(ref, output)) {
        emit extractResult(ref, output);
    }
}

void ResourceManager::exportEntry(const QPointer<Entry> ref, QUrl path)
{
    QByteArray output;
    if (!extract(ref, output))
        return;

    QFile f(path.toLocalFile());
    if (!f.open(QFile::WriteOnly)) {
        emit errorOccured(u"Failed to open file: %1"_qs.arg(f.fileName()));
        return;
    }
    f.write(output);
    f.close();
}

void ResourceManager::importEntry(const QPointer<Entry> ref, QUrl path)
{
    QByteArray data;
    QFile f(path.toLocalFile());
    if (f.open(QFile::ReadOnly)) {
        data = f.readAll();
        f.close();
    } else {
        emit errorOccured(u"Failed to open file: %1"_qs.arg(f.fileName()));
        return;
    }
    insert(ref, data);
}

const Container *ResourceManager::container(const QPointer<Entry> ref)
{
    Container *c = nullptr;
    if (m_containers.count() > ref->container) {
        c = m_containers[ref->container];
    } else {
        qWarning() << "Invalid container index:" << ref->container;
        emit errorOccured(u"Invalid container, try reloading indexes!"_qs);
        return c;
    }
    return c;
}

const Entry *ResourceManager::entry(const Container *c, const QPointer<Entry> ref)
{
    Entry *e = nullptr;
    if (c->entries.count() >= ref->entry) {
        e = c->entries[ref->entry];
    } else {
        qWarning() << "Invalid entry index:" << ref->entry;
        emit errorOccured(u"Invalid entry, try reloading indexes!"_qs);
        return e;
    }
    return e;
}

bool ResourceManager::extract(const QPointer<Entry> ref, QByteArray &output)
{
    const Container *c = container(ref);
    if (!c)
        return false;
    const Entry *e = entry(c, ref);
    if (!e)
        return false;
    qInfo() << "Starting extraction of" << e;
    QFile f(c->resourcePath(e->flags2));
    if (!f.open(QFile::ReadOnly)) {
        emit errorOccured(u"Failed to open resource file: %1"_qs.arg(f.fileName()));
        return false;
    }
    if (!f.seek(e->resourcePos)) {
        f.close();
        emit errorOccured(u"Failed to read resource file, resource beyond end of file!"_qs);
        return false;
    }
    QByteArray rawData = f.read(e->sizePacked);
    f.close();
    if (rawData.size() != e->sizePacked) {
        emit errorOccured(u"Failed to read resource file, resource file too small!"_qs);
        return false;
    }
    if (e->size != e->sizePacked) {
        if (!zutils::inflt(rawData, output)) {
            emit errorOccured(u"Failed to decompress asset, check for corrupt files!"_qs);
            return false;
        }
    } else {
        output = rawData;
    }
    qInfo() << "Finished extraction," << output.size() << "bytes!";
    return true;
}

bool ResourceManager::insert(const QPointer<Entry> ref, QByteArray &rawData)
{
    const Container *c = container(ref);
    if (!c)
        return false;
    const Entry *e = entry(c, ref);
    if (!e)
        return false;
    qInfo() << "Starting insertion of" << e;
    QByteArray data;
    if (e->size != e->sizePacked) {
        if (!zutils::deflt(rawData, data)) {
            emit errorOccured(u"Failed to compress asset!"_qs);
            return false;
        }
    } else {
        data = rawData;
    }
    if (data.size() > e->sizePacked) {
        emit errorOccured(u"Asset is too large, %1 > %2"_qs.arg(data.size()).arg(e->sizePacked));
        return false;
    } else if (data.size() < e->sizePacked) {
        data.append('\0' * (e->sizePacked - data.size()));
    }
    QFile f(c->resourcePath(e->flags2));
    if (!f.open(QFile::ReadWrite)) {
        emit errorOccured(u"Failed to open resource file: %1"_qs.arg(f.fileName()));
        return false;
    }
    if (!f.seek(e->resourcePos)) {
        f.close();
        emit errorOccured(u"Failed to write resource file, check for corrupt files!"_qs);
        return false;
    }
    f.write(data);
    f.close();
    qInfo() << "Finished insertion," << data.size() << "bytes!";
    return true;
}
