#include "core.h"

#include "resourcemanager.h"

Core::Core(QObject *parent)
    : QObject{parent}
    , m_error()
    , m_loading(false)
    , m_containerCount(0)
    , m_entryCount(0)
    , m_rm(new ResourceManager)
    , m_rmThread(new QThread(this))
{
    m_rm->moveToThread(m_rmThread);
    connect(m_rmThread, &QThread::finished, m_rm, &QObject::deleteLater);
    connect(this, &Core::startLoadingIndexes, m_rm, &ResourceManager::loadIndexes);
    connect(this, &Core::startSearch, m_rm, &ResourceManager::search);
    connect(m_rm, &ResourceManager::errorOccured, this, &Core::setError);
    connect(m_rm, &ResourceManager::loadingFinished, this, [&]() { setLoading(false); });
    connect(m_rm, &ResourceManager::containersLoaded, this, &Core::setContainerCount);
    connect(m_rm, &ResourceManager::entriesLoaded, this, &Core::setEntryCount);
    connect(m_rm, &ResourceManager::searchResult, this, &Core::searchResult);
    m_rmThread->start();

    QTimer::singleShot(100, this, &Core::loadIndexes);
}

Core::~Core()
{
    m_rmThread->quit();
    m_rmThread->wait();
}
