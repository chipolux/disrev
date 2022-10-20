#include "container.h"

#include <QDir>

Container::Container(QObject *parent)
    : QObject(parent)
{
}

QDebug operator<<(QDebug d, const Container *c)
{
    d.nospace() << "Container(" << c->path << ", " << c->resources.count() << " resources)";
    return d;
}

QString Container::resourcePath(const quint16 &flags) const
{
    // we always have 1 shared resource for Dishonored 2, but may be more
    // in other games or with older versions?
    QString result;
    const int index = flags & 0x8000 ? resources.count() - 1 : flags >> 2;
    if (resources.count() > index) {
        result = QDir(dir).absoluteFilePath(resources[index]);
    }
    return result;
}
