#include "entry.h"

Entry::Entry(const int &container, QObject *parent)
    : QObject(parent)
    , container(container)
{
}

Entry::Entry(const QPointer<Entry> other, QObject *parent)
    : QObject(parent)
    , container(other->container)
    , entry(other->entry)
    , indexPos(other->indexPos)
    , id(other->id)
    , type(other->type)
    , src(other->src)
    , dst(other->dst)
    , resourcePos(other->resourcePos)
    , size(other->size)
    , sizePacked(other->sizePacked)
    , flags1(other->flags1)
    , flags2(other->flags1)
{
}

QDebug operator<<(QDebug d, const Entry *e)
{
    d.nospace() << "Entry(" << e->id << ", " << e->src << ", " << e->dst << ")";
    return d;
}
