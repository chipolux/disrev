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

    MBC_PROP(int, container)
    MBC_PROP(int, entry)
    MBC_PROP(qint64, indexPos)
    MBC_PROP(quint32, id)
    MBC_PROP(QString, type)
    MBC_PROP(QString, src)
    MBC_PROP(QString, dst)
    MBC_PROP(quint64, resourcePos)
    MBC_PROP(quint32, size)
    MBC_PROP(quint32, sizePacked)
    MBC_PROP(quint16, flags1)
    MBC_PROP(quint16, flags2)

  public:
    explicit Entry(const int &container, QObject *parent);
    explicit Entry(QPointer<Entry> other, QObject *parent);
    QString srcSuffix() const { return QFileInfo(src).suffix(); }
    QString dstSuffix() const { return QFileInfo(dst).suffix(); }
};
QDebug operator<<(QDebug d, const Entry *e);

#endif // ENTRY_H
