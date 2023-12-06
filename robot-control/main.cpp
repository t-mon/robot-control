#include <QQuickStyle>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "application.h"

int main(int argc, char *argv[])
{
    Application application(argc, argv);
    application.setApplicationName(PROJECT_NAME);
    application.setOrganizationName(PROJECT_NAME);
    application.setApplicationVersion(VERSION_STRING);

    int debugLevel = 1;

    // Command line parser
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription(QString("\nTool to control a DIY robot arm.\n\n"
                                             "Copyright %1 2023 Simon Stürz <stuerz.simon@gmail.com>\n\n").arg(QChar(0xA9)));

    QCommandLineOption debugOption(QStringList() << "d" << "debug", QString("Print debug information (0-3). Default is %1. Set 0 to disable all outputs.").arg(debugLevel), "level");
    parser.addOption(debugOption);

    parser.process(application);

    // Verify debug prints
    if (parser.isSet(debugOption)) {
        bool debugLevelOk;
        debugLevel = parser.value(debugOption).toInt(&debugLevelOk);
        if (!debugLevelOk || debugLevel < 0 || debugLevel > 3) {
            qWarning() << "The debug level value is not valid. Please use a level between 0 and 3.";
            exit(EXIT_FAILURE);
        }
    }

    QStringList debugRulesList;
    debugRulesList.append("*.debug=false");
    if (debugLevel == 0) {
        debugRulesList.append("*.info=false");
        debugRulesList.append("*.debug=false");
        debugRulesList.append("*.warning=false");
    }

    if (debugLevel >= 1) {
        debugRulesList.append("default.debug=true");
        debugRulesList.append("*.warning=true");
        debugRulesList.append("Engine.info=true");
        debugRulesList.append("RobotController.info=true");
        debugRulesList.append("RobotSerialInterface.info=true");
    }

    if (debugLevel >= 2) {
        debugRulesList.append("Engine.debug=true");
        debugRulesList.append("RobotController.debug=true");
        debugRulesList.append("RobotSerialInterface.debug=true");
        debugRulesList.append("RobotSerialInterfaceTraffic.debug=false");
    }

    if (debugLevel >= 3) {
        debugRulesList.append("*.info=true");
        debugRulesList.append("RobotSerialInterfaceTraffic.info=true");
        debugRulesList.append("RobotSerialInterfaceTraffic.debug=true");
    }

    QLoggingCategory::setFilterRules(debugRulesList.join('\n'));

    QQuickStyle::setStyle("Universal");

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qt/qml/RobotControl/ui/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &application, []() { QCoreApplication::exit(EXIT_FAILURE); },
        Qt::QueuedConnection);
    engine.load(url);



    return application.exec();
}
