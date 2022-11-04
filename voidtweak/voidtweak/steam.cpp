#include "steam.h"

#include <QDir>
#include <QSettings>

namespace steam
{

QString steamDir()
{
#ifdef Q_OS_MACOS
    return u"%s/Library/Application Support/Steam"_qs.arg(QDir::homePath());
#elif Q_OS_WINDOWS
    QSettigs reg(u"HKEY_LOCAL_MACHINE\\SOFTWARE\\Valve\\Steam"_qs, QSettings::Registry64Format);
    return reg.value(u"InstallPath"_qs);
#endif
    return {};
}

QString dis2Dir()
{
    QSettings settings;
    QString result = settings.value(u"steam/dis2dir"_qs).toString();
    if (result.isEmpty()) {
        result = u"%1/steamapps/common/Dishonored2"_qs.arg(steamDir());
    }
    return result;
}

} // namespace steam
