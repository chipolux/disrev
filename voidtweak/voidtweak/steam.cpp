#include "steam.h"

#include <QDir>
#include <QSettings>

namespace steam
{

QString steamDir()
{
    QString result;
#ifdef Q_OS_MACOS
    result = u"%s/Library/Application Support/Steam"_qs.arg(QDir::homePath());
#endif
#ifdef Q_OS_WINDOWS
    QSettings reg(u"HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Valve\\Steam"_qs,
                  QSettings::Registry64Format);
    result = reg.value(u"InstallPath"_qs).toString();
#endif
    return result;
}

QString d2Dir()
{
    QSettings settings;
    QString result = settings.value(u"steam/dis2dir"_qs).toString();
    if (result.isEmpty()) {
        const QVariantMap libraryFolders = loadLibraryFolders();
        for (const auto &folder : libraryFolders) {
            const auto &path = folder.toMap().value(u"path"_qs).toString();
            const auto &apps = folder.toMap().value(u"apps"_qs);
            if (apps.toMap().contains(DIS2_APPID)) {
                result =
                    QDir::toNativeSeparators(u"%1/steamapps/common/Dishonored2/base"_qs.arg(path));
                break;
            }
        }
    }
    return result;
}

QVariantMap loadLibraryFolders()
{
    QVariantMap result;
    QFile f(QDir::toNativeSeparators(u"%1/config/libraryfolders.vdf"_qs.arg(steamDir())));
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open libraryfolders.vdf:" << f.fileName();
        return result;
    }
    const QString error = parse(f.readAll(), result);
    if (!error.isEmpty()) {
        qWarning() << "Failed to parse libraryfolders.vdf:" << error;
    }
    return result.value(u"libraryfolders"_qs).toMap();
}

QString parse(const QByteArray &data, QVariantMap &output)
{
    QByteArray token;
    QList<QVariantMap> stack;
    QList<QByteArray> tokens;
    bool inEscape = false;
    bool inToken = false;
    bool haveKey = false;
    for (qsizetype i = 0; i < data.size(); ++i) {
        const auto c = data[i];
        if (inEscape) {
            token.append(c);
            inEscape = false;
            continue;
        } else if (c == '\\') {
            inEscape = true;
            continue;
        }
        if (inToken && c != '"') {
            token.append(c);
            continue;
        }
        switch (c) {
        // handle start/end of token (key or value)
        case '"': {
            if (inToken && haveKey) {
                if (stack.isEmpty()) {
                    return u"No stack frames to insert kv pair at %1"_qs.arg(i);
                }
                stack.last().insert(tokens.takeLast(), token);
                token.clear();
                haveKey = false;
            } else if (inToken) {
                tokens.append(token);
                token.clear();
                haveKey = true;
            }
            inToken = !inToken;
            break;
        }
        case '{': {
            if (!haveKey) {
                return u"Missing key for scope starting at %1"_qs.arg(i);
            }
            stack.append(QVariantMap());
            haveKey = false;
            break;
        }
        case '}': {
            if (tokens.isEmpty()) {
                return u"Missing key for scope ending at %1"_qs.arg(i);
            }
            if (stack.isEmpty()) {
                return u"Missing stack frame for scope ending at %1"_qs.arg(i);
            }
            if (stack.count() > 1) {
                QVariantMap frame = stack.takeLast();
                stack.last().insert(tokens.takeLast(), frame);
            } else {
                output.insert(tokens.takeLast(), stack.takeLast());
            }
            break;
        }
        default: {
            break;
        }
        }
    }
    if (!tokens.isEmpty()) {
        return u"Leftover tokens at EOF: %1"_qs.arg(tokens.count());
    }
    if (!stack.isEmpty()) {
        return u"Leftover stack at EOF: %1"_qs.arg(stack.count());
    }
    qDebug() << "Parsed steam kv:" << output;
    return {};
}

} // namespace steam
