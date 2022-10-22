#include "core.h"

Core::Core(QObject *parent)
    : QObject{parent}
    , m_error()
    , m_busy(false)
    , m_containerCount(0)
    , m_entryCount(0)
    , m_rm(new ResourceManager)
    , m_rmThread(new QThread(this))
    , m_searchResultDebounce(new QTimer(this))
{
    m_rm->moveToThread(m_rmThread);
    connect(m_rmThread, &QThread::finished, m_rm, &QObject::deleteLater);
    connect(this, &Core::startLoadingIndexes, m_rm, &ResourceManager::loadIndexes);
    connect(this, &Core::startSearch, m_rm, &ResourceManager::search);
    connect(this, &Core::extractEntry, m_rm, &ResourceManager::extractEntry);
    connect(this, &Core::exportEntry, m_rm, &ResourceManager::exportEntry);
    connect(this, &Core::importEntry, m_rm, &ResourceManager::importEntry);
    connect(this, &Core::loadEntities, m_rm, &ResourceManager::loadEntities);
    connect(m_rm, &ResourceManager::statusChanged, this, &Core::rmStatusChanged);
    connect(m_rm, &ResourceManager::indexesLoaded, this, &Core::indexesLoaded);
    connect(m_rm, &ResourceManager::searchResult, this, &Core::searchResult);
    connect(m_rm, &ResourceManager::extractResult, this, &Core::extractResult);
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
        m_resultCount = 0;
        qDeleteAllLater(m_results);
        emit resultsChanged();
    }
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

void Core::searchResult(QPointer<Entry> entry)
{
    m_resultCount++;
    // we let the ResourceManager keep ownership of the Container and Entry
    // dataset, but since it lives in another thread we make a copy of the
    // Entry's we wish to show in the UI
    m_results.append(new Entry(entry, this));
    m_searchResultDebounce->start();
}

void Core::extractResult(const QPointer<Entry>, QByteArray) {}
