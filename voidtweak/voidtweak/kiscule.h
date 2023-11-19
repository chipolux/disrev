#ifndef KISCULE_H
#define KISCULE_H

#include <QObject>
#include <QtQml>

#include "qtutils.h"

namespace decl
{
class EntityEntry;
};

namespace kiscule
{

class Comment : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Backend only.")

  public:
    explicit Comment(const decl::EntityEntry *entry, QObject *parent);

  private:
    RO_PROP(int, id)
    RO_PROP(QRect, rect)
    RO_PROP(QString, outsideText)
    RO_PROP(QString, insideText)
};

class Node : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Backend only.")

  public:
    explicit Node(const decl::EntityEntry *entry, QObject *parent);

  private:
    RO_PROP(int, id)
    RO_PROP(QPoint, pos)
    RO_PROP(QString, name)
};

class Root : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Backend only.")

  public:
    explicit Root(const decl::EntityEntry *entry, QObject *parent);

  private:
    RO_PROP(int, version)
    RO_PROP(QList<Comment *>, comments)
    RO_PROP(QList<Node *>, nodes)
    RO_PROP(QPoint, lastPosition)
    RO_PROP(float, lastZoomLevel)
    RO_PROP(QRect, boundingBox)
    // RO_PROP(QList<QString>, nodes)
    // RO_PROP(QList<QString>, variables)
};

}; // namespace kiscule

#endif // KISCULE_H
