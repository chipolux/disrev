#ifndef STEAM_H
#define STEAM_H

#include <QString>

#define DIS2_APPID u"403640"_qs

namespace steam
{

QString steamDir();

QString d2Dir();

QVariantMap loadLibraryFolders();

QString parse(const QByteArray &data, QVariantMap &output);

} // namespace steam

#endif // STEAM_H
