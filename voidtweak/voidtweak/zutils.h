#ifndef ZUTILS_H
#define ZUTILS_H

#include <QByteArray>

namespace zutils
{

// odd names to avoid collisions with zlib and qt/zlib globals
QByteArray deflt(QByteArray &input);
QByteArray inflt(QByteArray &input);

} // namespace zutils

#endif // ZUTILS_H
