#ifndef APPLICATION_H
#define APPLICATION_H

#include <QLoggingCategory>
#include <QSocketNotifier>
#include <QGuiApplication>

Q_DECLARE_LOGGING_CATEGORY(dcApplication)

class Application : public QGuiApplication
{
public:
    Application(int &argc, char **argv);
};

#endif // APPLICATION_H
