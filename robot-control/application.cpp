#include "application.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <QDebug>

Q_LOGGING_CATEGORY(dcApplication, "Application")

static int s_shutdownCounter = 0;

static const char *const normal = "\033[0m";
static const char *const warning = "\e[33m";
static const char *const error = "\e[31m";

static void consoleLogHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    switch (type) {
    case QtInfoMsg:
        if (context.category == QStringLiteral("default")) {
            fprintf(stdout, "I | %s\n", message.toUtf8().data());
        } else {
            fprintf(stdout, "I | %s: %s\n", context.category, message.toUtf8().data());
        }
        break;
    case QtDebugMsg:
        if (context.category == QStringLiteral("default")) {
            fprintf(stdout, "D | %s\n", message.toUtf8().data());
        } else {
            fprintf(stdout, "D | %s: %s\n", context.category, message.toUtf8().data());
        }
        break;
    case QtWarningMsg:
        fprintf(stdout, "%s%s: %s%s\n", warning, context.category, message.toUtf8().data(), normal);
        break;
    case QtCriticalMsg:
        fprintf(stdout, "%s%s: %s%s\n", error, context.category, message.toUtf8().data(), normal);
        break;
    case QtFatalMsg:
        fprintf(stdout, "%s%s: %s%s\n", error, context.category, message.toUtf8().data(), normal);
        break;
    }
    fflush(stdout);
}


static void catchUnixSignals(const std::vector<int>& quitSignals, const std::vector<int>& ignoreSignals = std::vector<int>())
{
    auto handler = [](int sig) ->void {

        // forecefully exit() if repeated signals come in.
        if (s_shutdownCounter > 0) {
            if (s_shutdownCounter < 4) {
                qCCritical(dcApplication()) << "Shutdown in progress." << (4 - s_shutdownCounter) << "more times to abort.";
                s_shutdownCounter++;
                return;
            }
            exit(EXIT_FAILURE);
            return;
        }

        switch (sig) {
        case SIGQUIT:
            qCDebug(dcApplication) << "Cought SIGQUIT signal...";
            break;
        case SIGINT:
            qCDebug(dcApplication) << "Cought SIGINT signal...";
            break;
        case SIGTERM:
            qCDebug(dcApplication) << "Cought SIGTERM signal...";
            break;
        case SIGHUP:
            qCDebug(dcApplication) << "Cought SIGHUP signal...";
            break;
        case SIGKILL:
            qCDebug(dcApplication) << "Cought SIGKILL signal...";
            break;
        case SIGSEGV:
            qCDebug(dcApplication) << "Cought SIGSEGV quit signal...";
            break;
        case SIGFPE:
            qCDebug(dcApplication) << "Cought SIGFPE quit signal...";
            break;
        default:
            qCDebug(dcApplication) << "Cought signal" << sig;
            break;
        }

        qCInfo(dcApplication) << "----------------------------------------------";
        qCInfo(dcApplication) << "Shutting down" << PROJECT_NAME;

        s_shutdownCounter++;
        Application::quit();
    };

    // all these signals will be ignored.
    for (int sig : ignoreSignals)
        signal(sig, SIG_IGN);

    for (int sig : quitSignals)
        signal(sig, handler);
}



Application::Application(int &argc, char **argv) :
    QGuiApplication(argc, argv)
{
    qInstallMessageHandler(consoleLogHandler);

    // Catching SIGSEGV messes too much with various tools...
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP, SIGKILL, SIGFPE});
}
