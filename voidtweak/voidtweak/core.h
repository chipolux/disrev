#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QtQml>

#include "qtutils.h"
#include "resourcemanager.h"

class Core : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Backend only.")

    Q_PROPERTY(int resultCount READ resultCount NOTIFY resultsChanged)
    Q_PROPERTY(QList<Entry *> results READ results NOTIFY resultsChanged)

  public:
    explicit Core(QObject *parent = nullptr);
    ~Core();
    const int &resultCount() const { return m_resultCount; }
    const QList<Entry *> &results() const { return m_results; }

  signals:
    void startLoadingIndexes();
    void startSearch(const QString &query);
    void resultsChanged();
    void startExtraction(Entry *entry);

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
            m_resultCount = 0;
            qDeleteAllLater(m_results);
            emit resultsChanged();
        }
    }

  private slots:
    void searchResult(QPointer<Entry> entry)
    {
        m_resultCount++;
        // we let the ResourceManager keep ownership of the Container and Entry
        // dataset, but since it lives in another thread we make a copy of the
        // Entry's we wish to show in the UI
        m_results.append(new Entry(entry, this));
        m_searchResultDebounce->start();
    }
    void extractResult(const QPointer<Entry>, QByteArray) {}

  private:
    RW_PROP(QString, error, setError);
    RW_PROP(bool, loading, setLoading);

    RW_PROP(int, containerCount, setContainerCount);
    RW_PROP(int, entryCount, setEntryCount);
    int m_resultCount;
    QList<Entry *> m_results;

    QPointer<ResourceManager> m_rm;
    QThread *m_rmThread;
    QTimer *m_searchResultDebounce;
};

#endif // CORE_H
