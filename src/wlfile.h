#ifndef WLFILE_H
#define WLFILE_H

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


class WLFile : public QObject
{
	Q_OBJECT

public:
    WLFile(QObject *parent);
    ~WLFile();

Q_INVOKABLE QVariant loadValue(QString namefile,QString nameData,QVariant dataDef);
Q_INVOKABLE bool saveValue(QString namefile,QString nameData,QVariant data);

Q_INVOKABLE QString curPath(){return  QCoreApplication::applicationDirPath();}

Q_INVOKABLE bool createFile(QString namefile);
Q_INVOKABLE bool write(QString namefile,QString str);
private:
	QMutex mutex;
};

#endif // WLFILE_H
