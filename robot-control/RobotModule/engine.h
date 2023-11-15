#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QQmlEngine>

class Engine : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit Engine(QObject *parent = nullptr);

signals:

};

#endif // ENGINE_H
