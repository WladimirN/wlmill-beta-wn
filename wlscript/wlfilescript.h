#ifndef WLFILESCRIPT_H
#define WLFILESCRIPT_H

#include <QObject>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QMutex>
#include <QSettings>
#include <QScriptValue>
#include <QScriptEngine>
#include <QApplication>


class WLFileScript : public QObject
{
	Q_OBJECT

public:
    WLFileScript(QObject *parent);
    ~WLFileScript();
/*
Q_INVOKABLE QVariant loadValue(QString namefile,QString nameData,QVariant dataDef);
Q_INVOKABLE bool saveValue(QString namefile,QString nameData,QVariant data);
*/
Q_INVOKABLE QScriptValue loadValue(QString namefile,QString nameData,QScriptValue dataDef);
Q_INVOKABLE bool saveValue(QString namefile,QString nameData,QScriptValue data);

Q_INVOKABLE QString curPath(){return  QCoreApplication::applicationDirPath();}

Q_INVOKABLE bool createFile(QString namefile);
Q_INVOKABLE bool write(QString namefile,QString str);

Q_INVOKABLE QString listFiles(QString path,QString split=",");
Q_INVOKABLE QString listDirs(QString path,QString split=",");

private:
	QMutex mutex;
};

#endif // WLFILESCRIPT_H
