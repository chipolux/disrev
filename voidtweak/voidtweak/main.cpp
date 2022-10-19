#include <QDir>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "config.h"
#include "core.h"
#include "qtutils.h"

bool registerFonts()
{
    bool success = true;
    QString prefix;

    prefix = u":/%1/resources/Fira_Code/"_qs.arg(PROJECT_NAME);
    for (const QString &f : QDir(prefix).entryList({u"*.ttf"_qs})) {
        if (QFontDatabase::addApplicationFont(prefix + f) == -1)
            success = false;
    }

    return success;
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);

    QGuiApplication app(argc, argv);
    app.setApplicationName(APP_NAME);
    app.setOrganizationName(ORG_NAME);
    app.setOrganizationDomain(ORG_DOMAIN);
    app.setApplicationVersion(PROJECT_VER);
    app.setWindowIcon(QIcon(u":/%1/resources/icon.png"_qs.arg(PROJECT_NAME)));
    QQuickStyle::setStyle("Basic");
    qDebug() << APP_NAME << "version" << PROJECT_VER;

    qDebug() << "Registering custom fonts...";
    if (!registerFonts()) {
        qWarning() << "Failed to register custom fonts!";
    } else {
#ifdef Q_OS_WIN
        app.setFont({"Fira Code", 10, QFont::Normal});
#else
        app.setFont({"Fira Code", 13, QFont::Normal});
#endif
    }

    qDebug() << "Setting up backend...";
    Core core(&app);

    QQmlApplicationEngine engine(&app);
    QQmlContext *ctx = engine.rootContext();
    ctx->setContextProperty("core", &core);

    qDebug() << "Loading QML...";
    const QUrl url(u"qrc:/%1/qml/main.qml"_qs.arg(PROJECT_NAME));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url, &app](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                qCritical("Failed to load any QML objects.");
                app.quit();
            }
        },
        Qt::QueuedConnection);
    engine.load(url);

    qDebug() << "Starting application...";
    return app.exec();
}
