#include "core.h"

#include <QProcess>
#include <algorithm>

#include "steam.h"

Core::Core(QObject *parent)
    : QObject{parent}
    , m_error()
    , m_busy(false)
    , m_containerCount(0)
    , m_entryCount(0)
    , m_sortOrder(SortNone)
    , m_resultCount(0)
    , m_results()
    , m_rm(new ResourceManager)
    , m_rmThread(new QThread(this))
    , m_searchResultDebounce(new QTimer(this))
    , m_entry(nullptr)
    , m_entities()
    , m_script(nullptr)
{
    m_rm->moveToThread(m_rmThread);
    connect(m_rmThread, &QThread::finished, m_rm, &QObject::deleteLater);
    connect(this, &Core::startLoadingIndexes, m_rm, &ResourceManager::loadIndexes);
    connect(this, &Core::startSearch, m_rm, &ResourceManager::search);
    connect(this, &Core::extractEntry, m_rm, &ResourceManager::extractEntry);
    connect(this, &Core::insertEntry, m_rm, &ResourceManager::insertEntry);
    connect(this, &Core::exportEntry, m_rm, &ResourceManager::exportEntry);
    connect(this, &Core::importEntry, m_rm, &ResourceManager::importEntry);
    connect(this, &Core::loadEntities, m_rm, &ResourceManager::loadEntities);
    connect(this, &Core::exportAllEntries, m_rm, &ResourceManager::exportAllEntries);
    connect(this, &Core::loadBwm, m_rm, &ResourceManager::loadBwm);
    connect(this, &Core::startSavingObject, m_rm, &ResourceManager::saveObject);
    connect(this, &Core::startSavingObjects, m_rm, &ResourceManager::saveObjects);
    connect(m_rm, &ResourceManager::statusChanged, this, &Core::rmStatusChanged);
    connect(m_rm, &ResourceManager::indexesLoaded, this, &Core::indexesLoaded);
    connect(m_rm, &ResourceManager::searchResult, this, &Core::searchResult);
    connect(m_rm, &ResourceManager::extractResult, this, &Core::extractResult);
    connect(m_rm, &ResourceManager::entitiesLoaded, this, &Core::entitiesLoaded);
    connect(m_rm, &ResourceManager::bwmLoaded, this, &Core::bwmLoaded);
    m_rmThread->start();

    m_searchResultDebounce->setInterval(100);
    m_searchResultDebounce->setSingleShot(true);
    connect(m_searchResultDebounce, &QTimer::timeout, this, &Core::resultsChanged);

    QTimer::singleShot(100, this, &Core::loadIndexes);
}

Core::~Core()
{
    m_rmThread->quit();
    m_rmThread->wait();
}

const QString Core::sortOrderName() const
{
    QString name;
    switch (m_sortOrder) {
    case SortNone:
    case SortMax:
        name = u"None"_qs;
        break;
    case SortSizeAscending:
        name = u"Size ▲"_qs;
        break;
    case SortSizeDescending:
        name = u"Size ▼"_qs;
        break;
    case SortTypeAscending:
        name = u"Type ▲"_qs;
        break;
    case SortTypeDescending:
        name = u"Type ▼"_qs;
        break;
    case SortSrcAscending:
        name = u"Src ▲"_qs;
        break;
    case SortSrcDescending:
        name = u"Src ▼"_qs;
        break;
    case SortDstAscending:
        name = u"Dst ▲"_qs;
        break;
    case SortDstDescending:
        name = u"Dst ▼"_qs;
        break;
    }
    return name;
}

void Core::launchGame()
{
    const auto exePath = steam::dis2Exe();
    if (exePath.isEmpty()) {
        return;
    }

    QProcess::execute(exePath, {u"+com_showLoadingScreen 0"_qs});
}

void Core::sortResults(const Core::SortOrder &order)
{
    if (order != m_sortOrder) {
        m_sortOrder = order == SortMax ? SortNone : order;
        switch (m_sortOrder) {
        case SortNone:
        case SortMax:
            break;
        case SortSizeAscending: {
            std::sort(m_results.begin(), m_results.end(),
                      [](const Entry *a, const Entry *b) { return a->size < b->size; });
            emit resultsChanged();
            break;
        }
        case SortSizeDescending: {
            std::sort(m_results.begin(), m_results.end(),
                      [](const Entry *a, const Entry *b) { return a->size > b->size; });
            emit resultsChanged();
            break;
        }
        case SortTypeAscending: {
            std::sort(m_results.begin(), m_results.end(),
                      [](const Entry *a, const Entry *b) { return a->type < b->type; });
            emit resultsChanged();
            break;
        }
        case SortTypeDescending: {
            std::sort(m_results.begin(), m_results.end(),
                      [](const Entry *a, const Entry *b) { return a->type > b->type; });
            emit resultsChanged();
            break;
        }
        case SortSrcAscending: {
            std::sort(m_results.begin(), m_results.end(),
                      [](const Entry *a, const Entry *b) { return a->src < b->src; });
            emit resultsChanged();
            break;
        }
        case SortSrcDescending: {
            std::sort(m_results.begin(), m_results.end(),
                      [](const Entry *a, const Entry *b) { return a->src > b->src; });
            emit resultsChanged();
            break;
        }
        case SortDstAscending: {
            std::sort(m_results.begin(), m_results.end(),
                      [](const Entry *a, const Entry *b) { return a->dst < b->dst; });
            emit resultsChanged();
            break;
        }
        case SortDstDescending: {
            std::sort(m_results.begin(), m_results.end(),
                      [](const Entry *a, const Entry *b) { return a->dst > b->dst; });
            emit resultsChanged();
            break;
        }
        }
        emit sortOrderChanged();
    }
}

void Core::loadIndexes()
{
    if (!m_busy) {
        clear();
        emit startLoadingIndexes();
    }
}

void Core::search(const QString &query)
{
    if (!m_busy) {
        clear();
        emit startSearch(query);
    }
}

void Core::clear()
{
    if (!m_busy) {
        m_sortOrder = SortNone;
        m_resultCount = 0;
        qDeleteAllLater(m_results);
        emit resultsChanged();
        emit sortOrderChanged();
        clearEntities();
        clearObjects();
        clearScript();
    }
}

void Core::clearEntities()
{
    qDeleteAllLater(m_entities);
    m_entry = nullptr;
    emit entitiesChanged();
}

void Core::rmStatusChanged(bool busy, QString error)
{
    setBusy(busy);
    setError(error);
}

void Core::indexesLoaded(int containerCount, int entryCount)
{
    setContainerCount(containerCount);
    setEntryCount(entryCount);
}

void Core::searchResult(const QPointer<Entry> entry)
{
    ++m_resultCount;
    // we let the ResourceManager keep ownership of the Container and Entry
    // dataset, but since it lives in another thread we make a copy of the
    // Entry's we wish to show in the UI
    m_results.append(new Entry(entry, this));
    m_searchResultDebounce->start();
}

void Core::extractResult(const QPointer<Entry>, QByteArray) {}

void Core::entitiesLoaded(const QPointer<Entry> ref, QList<decl::Scope> entities)
{
    if (m_results.contains(ref)) {
        qDebug() << "Building full entities...";
        qDeleteAllLater(m_entities);
        m_entry = m_results.at(m_results.indexOf(ref));
        for (const auto &scope : entities) {
            m_entities.append(new decl::Entity(scope, this));
        }
        qInfo() << "Built" << m_entities.count() << "for" << ref;
        emit entitiesChanged();
    } else {
        qWarning() << "No matching entry in results:" << ref;
    }
}

void Core::bwmLoaded(const QPointer<Entry> ref, QList<bwm::PODObject> objects)
{
    if (m_results.contains(ref)) {
        qDebug() << "Building full objects...";
        qDeleteAllLater(m_objects);
        m_entry = m_results.at(m_results.indexOf(ref));
        for (const auto &o : objects) {
            m_objects.append(new bwm::Object(o, this));
        }
        qInfo() << "Built" << m_objects.count() << "objects for" << ref;
        emit objectsChanged();
    } else {
        qWarning() << "No matching entry in results:" << ref;
    }
}

void Core::saveEntities()
{
    if (m_entry && !m_entities.isEmpty()) {
        QByteArray data;
        decl::write(m_entities, data);
        emit insertEntry(m_entry, data);
    }
}

void Core::deleteEntities(const QList<decl::Entity *> &entities)
{
    bool changed = false;
    for (const auto e : entities) {
        if (m_entities.contains(e)) {
            e->deleteLater();
            m_entities.removeAll(e);
            changed = true;
        }
    }
    if (changed) {
        emit entitiesChanged();
    }
}

void Core::clearObjects()
{
    qDeleteAllLater(m_objects);
    m_entry = nullptr;
    emit objectsChanged();
}

void Core::saveObject(bwm::Object *obj)
{
    if (obj) {
        emit startSavingObject(m_entry, obj->pod());
    }
}

void Core::saveObjects()
{
    if (!m_objects.isEmpty()) {
        QList<bwm::PODObject> objects;
        for (const auto obj : m_objects) {
            objects.append(obj->pod());
        }
        emit startSavingObjects(m_entry, objects);
    }
}

void Core::loadScript(const decl::EntityEntry *entry)
{
    if (m_script) {
        m_script->deleteLater();
    }
    m_script = new kiscule::Root(entry, this);
    emit scriptChanged();
}

void Core::clearScript()
{
    if (m_script) {
        m_script->deleteLater();
        m_script = nullptr;
        emit scriptChanged();
    }
}
