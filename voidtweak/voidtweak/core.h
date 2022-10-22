#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QtQml>

#include "entry.h"
#include "qtutils.h"
#include "resourcemanager.h"

class Core : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Backend only.")

    Q_PROPERTY(SortOrder sortOrder READ sortOrder NOTIFY sortOrderChanged)
    Q_PROPERTY(QString sortOrderName READ sortOrderName NOTIFY sortOrderChanged)
    Q_PROPERTY(int resultCount READ resultCount NOTIFY resultsChanged)
    Q_PROPERTY(QList<Entry *> results READ results NOTIFY resultsChanged)

  public:
    enum SortOrder {
        SortNone = 0,
        SortSizeAscending,
        SortSizeDescending,
        SortTypeAscending,
        SortTypeDescending,
        SortSrcAscending,
        SortSrcDescending,
        SortDstAscending,
        SortDstDescending,
        SortMax,
    };
    Q_ENUM(SortOrder)

    explicit Core(QObject *parent = nullptr);
    ~Core();
    const SortOrder &sortOrder() const { return m_sortOrder; }
    const QString sortOrderName() const;
    const int &resultCount() const { return m_resultCount; }
    const QList<Entry *> &results() const { return m_results; }

  signals:
    void startLoadingIndexes();
    void startSearch(const QString &query);
    void extractEntry(Entry *entry);
    void exportEntry(Entry *entry, QUrl path);
    void importEntry(Entry *entry, QUrl path);
    void loadEntities(Entry *entry);
    void sortOrderChanged();
    void resultsChanged();

  public slots:
    void sortResults(const Core::SortOrder &order);
    void loadIndexes();
    void search(const QString &query);
    void clear();

  private slots:
    void rmStatusChanged(bool busy, QString error);
    void indexesLoaded(int containerCount, int entryCount);
    void searchResult(QPointer<Entry> entry);
    void extractResult(const QPointer<Entry>, QByteArray);

  private:
    RW_PROP(QString, error, setError)
    RW_PROP(bool, busy, setBusy)

    RW_PROP(int, containerCount, setContainerCount)
    RW_PROP(int, entryCount, setEntryCount)
    SortOrder m_sortOrder;
    int m_resultCount;
    QList<Entry *> m_results;

    QPointer<ResourceManager> m_rm;
    QThread *m_rmThread;
    QTimer *m_searchResultDebounce;
};

#endif // CORE_H
