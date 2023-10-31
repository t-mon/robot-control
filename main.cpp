#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

#include "engine.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);



    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/robot-arm/Main.qml"_qs);
    QQuickStyle::setStyle("Universal");
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
