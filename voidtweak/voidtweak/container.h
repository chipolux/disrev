#ifndef CONTAINER_H
#define CONTAINER_H

#include <QObject>

#include "entry.h"
#include "qtutils.h"

class Container : public QObject
{
    Q_OBJECT

    MBC_PROP(QString, dir)
    MBC_PROP(QString, path)
    MBC_PROP(QList<QString>, resources)
    MBC_PROP(QList<Entry *>, entries)
    Q_PROPERTY(QString pathAbsolute READ pathAbsolute CONSTANT)

  public:
    explicit Container(QObject *parent);
    QString pathAbsolute() const { return u"%1\\%2"_qs.arg(dir, path); }
    QString resourcePath(const quint16 &flags) const;
};
QDebug operator<<(QDebug d, const Container *c);

#endif // CONTAINER_H
