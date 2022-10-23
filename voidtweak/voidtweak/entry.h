#ifndef ENTRY_H
#define ENTRY_H

#include <QObject>
#include <QtQml>

#include "qtutils.h"

class Entry : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Backend only.")

    CM_PROP(int, container)
    CM_PROP(int, entry)
    CM_PROP(qint64, indexPos)
    CM_PROP(quint32, id)
    CM_PROP(QString, type)
    CM_PROP(QString, src)
    CM_PROP(QString, dst)
    CM_PROP(quint64, resourcePos)
    CM_PROP(quint32, size)
    CM_PROP(quint32, sizePacked)
    CM_PROP(quint16, flags1)
    CM_PROP(quint16, flags2)

    Q_PROPERTY(QString srcSuffix READ srcSuffix CONSTANT)
    Q_PROPERTY(QString dstSuffix READ dstSuffix CONSTANT)
    Q_PROPERTY(QString srcFileName READ srcFileName CONSTANT)
    Q_PROPERTY(QString dstFileName READ dstFileName CONSTANT)

  public:
    explicit Entry(const int &container, QObject *parent);
    explicit Entry(const QPointer<Entry> other, QObject *parent);
    QString srcSuffix() const { return QFileInfo(src).suffix(); }
    QString dstSuffix() const { return QFileInfo(dst).suffix(); }
    QString srcFileName() const { return QFileInfo(src).fileName(); }
    QString dstFileName() const { return QFileInfo(dst).fileName(); }
};
QDebug operator<<(QDebug d, const Entry *e);

#endif // ENTRY_H
