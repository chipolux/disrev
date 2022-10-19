#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QtQml>

#include "qtutils.h"

class ResourceManager;

class SearchResult : public QObject
{
    Q_OBJECT

  public:
    explicit SearchResult(const quint32 &id, const QString &src, const QString &dst,
                          const quint32 &size, QObject *parent)
        : QObject(parent)
        , m_id(id)
        , m_src(src)
        , m_dst(dst)
        , m_size(size)
    {
    }

  private:
    RO_PROP(quint32, id)
    RO_PROP(QString, src)
    RO_PROP(QString, dst)
    RO_PROP(quint32, size)
};

class Core : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Backend only.")

  public:
    explicit Core(QObject *parent = nullptr);
    ~Core();

  signals:
    void startLoadingIndexes();
    void startSearch(const QString &query);

  public slots:
    void loadIndexes()
    {
        if (!m_loading) {
            clear();
            setLoading(true);
            emit startLoadingIndexes();
        }
    }
    void search(const QString &query)
    {
        if (!m_loading) {
            clear();
            setLoading(true);
            emit startSearch(query);
        }
    }
    void clear()
    {
        if (!m_loading) {
            setError({});
            setResultCount(0);
            qDeleteAllLater(m_results);
            emit resultsChanged(m_results);
        }
    }

  private slots:
    void searchResult(quint32 id, QString src, QString dst, quint32 size)
    {
        setResultCount(m_resultCount + 1);
        if (m_results.count() < 200) {
            m_results.append(new SearchResult(id, src, dst, size, this));
            emit resultsChanged(m_results);
        } else {
            setError(u"Too many results, only displaying first 200."_qs);
        }
    }

  private:
    RW_PROP(QString, error, setError);
    RW_PROP(bool, loading, setLoading);

    RW_PROP(int, containerCount, setContainerCount);
    RW_PROP(int, entryCount, setEntryCount);
    RW_PROP(int, resultCount, setResultCount);
    RO_PROP(QList<SearchResult *>, results);

    QPointer<ResourceManager> m_rm;
    QThread *m_rmThread;
};

#endif // CORE_H
