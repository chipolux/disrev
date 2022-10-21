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
    emit statusChanged(true, {});
    qInfo() << "Started loading...";
    qDeleteAllLater(m_containers);
    if (!loadMasterIndex())
        return;
    if (!loadChildIndexes())
        return;
    int entryCount = 0;
    for (const auto c : m_containers) {
        entryCount += c->entries.count();
    }
    qInfo() << "Finished loading...";
    emit indexesLoaded(m_containers.count(), entryCount);
    emit statusChanged(false, {});
}

bool ResourceManager::loadMasterIndex()
{
    QDir dir(D2Dir);
    if (!dir.exists(u"master.index"_qs)) {
        qWarning() << "No master.index found!";
        emit statusChanged(false, u"No master.index found!"_qs);
        return false;
    }
    QFile f(dir.absoluteFilePath(u"master.index"_qs));
    if (!f.open(QFile::ReadOnly)) {
        emit statusChanged(false, u"Failed to open: %1"_qs.arg(f.fileName()));
        return false;
    }
    QDataStream in(&f);
    in.setByteOrder(QDataStream::BigEndian);

    quint32 magic;
    in >> magic;
    if (magic != 0x04534552) {
        f.close();
        emit statusChanged(false, u"Invalid magic: %1"_qs.arg(magic));
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
            emit statusChanged(false, u"Failed to open: %1"_qs.arg(f.fileName()));
            return false;
        }
        QDataStream in(&f);
        in.setByteOrder(QDataStream::BigEndian);

        quint32 magic;
        in >> magic;
        if (magic != 0x05534552) {
            f.close();
            emit statusChanged(false, u"Invalid magic: %1"_qs.arg(magic));
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
    emit statusChanged(true, {});
    qInfo() << "Searching for:" << query;
    for (const auto c : m_containers) {
        for (const auto e : c->entries) {
            if (e->src.contains(query) || e->dst.contains(query)) {
                emit searchResult(e);
            }
        }
    }
    qInfo() << "Finished searching...";
    emit statusChanged(false, {});
}

void ResourceManager::extractEntry(const QPointer<Entry> ref)
{
    emit statusChanged(true, {});
    QByteArray output;
    if (!extract(ref, output))
        return;
    emit extractResult(ref, output);
    emit statusChanged(false, {});
}

void ResourceManager::exportEntry(const QPointer<Entry> ref, QUrl path)
{
    emit statusChanged(true, {});
    QByteArray output;
    if (!extract(ref, output))
        return;
    QFile f(path.toLocalFile());
    if (!f.open(QFile::WriteOnly)) {
        emit statusChanged(false, u"Failed to open file: %1"_qs.arg(f.fileName()));
        return;
    }
    f.write(output);
    f.close();
    emit statusChanged(false, {});
}

void ResourceManager::importEntry(const QPointer<Entry> ref, QUrl path)
{
    emit statusChanged(true, {});
    QByteArray data;
    QFile f(path.toLocalFile());
    if (f.open(QFile::ReadOnly)) {
        data = f.readAll();
        f.close();
    } else {
        emit statusChanged(false, u"Failed to open file: %1"_qs.arg(f.fileName()));
        return;
    }
    insert(ref, data);
    emit statusChanged(false, {});
}

void ResourceManager::loadEntities(const QPointer<Entry> ref)
{
    emit statusChanged(true, {});
    QByteArray data;
    if (!extract(ref, data))
        return;
    if (!processEntities(data))
        return;
    emit statusChanged(false, {});
}

const Container *ResourceManager::container(const QPointer<Entry> ref)
{
    Container *c = nullptr;
    if (m_containers.count() > ref->container) {
        c = m_containers[ref->container];
    } else {
        emit statusChanged(false, u"Invalid container, try reloading indexes!"_qs);
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
        emit statusChanged(false, u"Invalid entry, try reloading indexes!"_qs);
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
        emit statusChanged(false, u"Failed to open resource file: %1"_qs.arg(f.fileName()));
        return false;
    }
    if (!f.seek(e->resourcePos)) {
        f.close();
        emit statusChanged(false, u"Failed to read resource file, resource beyond end of file!"_qs);
        return false;
    }
    QByteArray rawData = f.read(e->sizePacked);
    f.close();
    if (rawData.size() != e->sizePacked) {
        emit statusChanged(false, u"Failed to read resource file, resource file too small!"_qs);
        return false;
    }
    if (e->size != e->sizePacked) {
        if (!zutils::inflt(rawData, output)) {
            emit statusChanged(false, u"Failed to decompress asset, check for corrupt files!"_qs);
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
            emit statusChanged(false, u"Failed to compress asset!"_qs);
            return false;
        }
    } else {
        data = rawData;
    }
    if (data.size() > e->sizePacked) {
        emit statusChanged(false,
                           u"Asset is too large, %1 > %2"_qs.arg(data.size()).arg(e->sizePacked));
        return false;
    } else if (data.size() < e->sizePacked) {
        data.append('\0' * (e->sizePacked - data.size()));
    }
    QFile f(c->resourcePath(e->flags2));
    if (!f.open(QFile::ReadWrite)) {
        emit statusChanged(false, u"Failed to open resource file: %1"_qs.arg(f.fileName()));
        return false;
    }
    if (!f.seek(e->resourcePos)) {
        f.close();
        emit statusChanged(false, u"Failed to write resource file, check for corrupt files!"_qs);
        return false;
    }
    f.write(data);
    f.close();
    qInfo() << "Finished insertion," << data.size() << "bytes!";
    return true;
}

bool ResourceManager::processEntities(const QByteArray &data)
{
    qInfo() << "Started loading entities...";
    if (data.first(9) != QByteArrayLiteral("Version 6")) {
        emit statusChanged(false, u"Unknown header!"_qs);
        return false;
    }
    QByteArray token;
    QList<QByteArray> tokens;
    QList<QVariantMap> stack;
    QVariantMap entities;
    int entityCount = 0;
    bool inEscape = false;
    bool inString = false;
    for (int i = 9; i < data.size(); i++) {
        const auto c = data[i];
        // shortcut escape sequences
        if (inEscape) {
            token.append(c);
            inEscape = false;
            continue;
        } else if (c == '\\') {
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
        switch (c) {
        // handle token separators
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case '=': {
            if (!token.isEmpty()) {
                tokens.append(token);
                token.clear();
            }
            break;
        }
        // handle value end marker
        case ';': {
            if (stack.isEmpty()) {
                emit statusChanged(false, u"Missing stack frame for key-value pair @ %1"_qs.arg(i));
                return false;
            }
            if (tokens.isEmpty()) {
                emit statusChanged(false, u"Missing tokens for key-value pair @ %1"_qs.arg(i));
                return false;
            }
            // NOTE: some values have a key that is made up of 2+ tokens
            // we just combine them into 1 key string for now
            const int keyStart = stack.count() - 1;
            const int keyEnd = tokens.count() - 1;
            QByteArray key;
            for (int ki = keyEnd; ki >= keyStart; ki--) {
                key.prepend((ki == keyStart ? "" : " ") + tokens.takeAt(ki));
            }
            stack.last().insert(QString(key), QString(token));
            token.clear();
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
                stack.append(QVariantMap());
            }
            break;
        }
        // handle exiting nested scope
        case '}': {
            if (stack.count() == 1 && tokens.isEmpty()) {
                QVariantMap frame = stack.takeLast();
                entities.insert(frame.value(u"entityId"_qs).toString(), frame);
                entityCount++;
            } else if (stack.count() > 1 && !tokens.isEmpty()) {
                QVariantMap frame = stack.takeLast();
                stack.last().insert(QString(tokens.takeLast()), frame);
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
        emit statusChanged(false, u"Bad entities, leftover data!"_qs);
        return false;
    }
    if (entityCount != entities.count()) {
        emit statusChanged(false, u"Bad entities, duplicate ids!"_qs);
        return false;
    }

    qInfo() << "Loaded entities:" << entities.count();
    return true;
}
