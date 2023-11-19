#ifndef STEAM_H
#define STEAM_H

#include <QString>

#define DIS2_APPID u"403640"_qs
#define DOTO_APPID u"614570"_qs

namespace steam
{

QString steamDir();

QString appDir(const QString &appId);

QString dis2Dir();

QString dis2Exe();

QString dotoDir();

QVariantMap loadLibraryFolders();

QVariantMap load(const QString &path);

QString parse(const QByteArray &data, QVariantMap &output);

} // namespace steam

#endif // STEAM_H
