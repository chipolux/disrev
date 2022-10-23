#ifndef CONTAINER_H
#define CONTAINER_H

#include <QObject>

#include "entry.h"
#include "qtutils.h"

class Container : public QObject
{
    Q_OBJECT

    CM_PROP(QString, dir)
    CM_PROP(QString, path)
    CM_PROP(QList<QString>, resources)
    CM_PROP(QList<Entry *>, entries)
    Q_PROPERTY(QString indexPath READ indexPath CONSTANT)

  public:
    explicit Container(QObject *parent);
    QString indexPath() const { return u"%1\\%2"_qs.arg(dir, path); }
    QString resourcePath(const quint16 &flags) const;
};
QDebug operator<<(QDebug d, const Container *c);

#endif // CONTAINER_H
