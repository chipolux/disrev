#ifndef ZUTILS_H
#define ZUTILS_H

#include <QByteArray>

namespace zutils
{

// odd names to avoid collisions with zlib and qt/zlib globals
bool deflt(QByteArray &input, QByteArray &output);
bool inflt(QByteArray &input, QByteArray &output);

} // namespace zutils

#endif // ZUTILS_H
