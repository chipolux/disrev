#include "core.h"

#include <algorithm>

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
    , m_entities()
    , m_entry(nullptr)
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
    connect(m_rm, &ResourceManager::statusChanged, this, &Core::rmStatusChanged);
    connect(m_rm, &ResourceManager::indexesLoaded, this, &Core::indexesLoaded);
    connect(m_rm, &ResourceManager::searchResult, this, &Core::searchResult);
    connect(m_rm, &ResourceManager::extractResult, this, &Core::extractResult);
    connect(m_rm, &ResourceManager::entitiesLoaded, this, &Core::entitiesLoaded);
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
    qInfo() << "Building full entities...";
    qDeleteAllLater(m_entities);
    m_entry = m_results.at(m_results.indexOf(ref));
    for (const auto &scope : entities) {
        m_entities.append(new decl::Entity(scope, this));
    }
    qInfo() << "Built entities:" << m_entities.count();
    emit entitiesChanged();
}

void Core::saveEntities()
{
    if (m_entry && !m_entities.isEmpty()) {
        QByteArray data;
        decl::write(m_entities, data);
        emit insertEntry(m_entry, data);
    }
}
