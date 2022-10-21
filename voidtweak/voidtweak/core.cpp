#include "core.h"

Core::Core(QObject *parent)
    : QObject{parent}
    , m_error()
    , m_loading(false)
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
    connect(m_rm, &ResourceManager::errorOccured, this, &Core::setError);
    connect(m_rm, &ResourceManager::loadingFinished, this, [&]() { setLoading(false); });
    connect(m_rm, &ResourceManager::containersLoaded, this, &Core::setContainerCount);
    connect(m_rm, &ResourceManager::entriesLoaded, this, &Core::setEntryCount);
    connect(m_rm, &ResourceManager::searchResult, this, &Core::searchResult);
    connect(m_rm, &ResourceManager::extractResult, this, &Core::extractResult);
    m_rmThread->start();

    m_searchResultDebounce->setInterval(500);
    m_searchResultDebounce->setSingleShot(true);
    connect(m_searchResultDebounce, &QTimer::timeout, this, &Core::resultsChanged);

    QTimer::singleShot(100, this, &Core::loadIndexes);
}

Core::~Core()
{
    m_rmThread->quit();
    m_rmThread->wait();
}
