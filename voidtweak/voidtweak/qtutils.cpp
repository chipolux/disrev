#include "qtutils.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QObject>

void messageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QByteArray time = QDateTime::currentDateTime().toString(Qt::ISODateWithMs).toLatin1();
    switch (type) {
    case QtDebugMsg:
#ifdef QT_DEBUG
        fprintf(stdout, "[%s] DBUG: %s\n", time.constData(), localMsg.constData());
        fflush(stdout);
#endif
        break;
    case QtInfoMsg:
        fprintf(stdout, "[%s] INFO: %s\n", time.constData(), localMsg.constData());
        fflush(stdout);
        break;
    case QtWarningMsg:
        if (!msg.contains(u"focus object"_qs)) { // ignore some noisy ios platform warnings
            fprintf(stderr, "[%s] WARN: %s\n", time.constData(), localMsg.constData());
        }
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[%s] CRIT: %s\n", time.constData(), localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "[%s] FATL: %s\n", time.constData(), localMsg.constData());
        QCoreApplication *app = QCoreApplication::instance();
        if (app != nullptr) {
            app->exit(-1);
        } else {
            abort();
        }
    }
}
