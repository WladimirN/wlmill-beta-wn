#include "wlfilescript.h"

WLFileScript::WLFileScript(QObject *parent)
	: QObject(parent)
{

}

WLFileScript::~WLFileScript()
{

}

bool WLFileScript::createFile(QString namefile)
{
QFile File(namefile);
if(File.open(QIODevice::WriteOnly))
 {
 File.close();
 return true;
 }
return false;
}

bool WLFileScript::write(QString namefile,QString str)
{
QFile File(namefile);
if(File.open(QIODevice::Append))
 {
 File.write(str.toLocal8Bit());
 File.close();
 return true;
 }
return false;
}

bool WLFileScript::saveValue(QString namefile,QString nameData,QScriptValue data)
{
QMutexLocker locker(&mutex);

QVariant val;

QSettings save(namefile, QSettings::IniFormat);

save.setIniCodec("UTF-8");

save.setValue(nameData,data.toVariant());

return true;
}

QScriptValue WLFileScript::loadValue(QString namefile,QString nameData,QScriptValue dataDef)
{
QMutexLocker locker(&mutex);

QVariant val;

QSettings load(namefile, QSettings::IniFormat);

load.setIniCodec("UTF-8");

if(dataDef.isNumber())
    return load.value(nameData,dataDef.toVariant()).toDouble();
else
    return load.value(nameData,dataDef.toVariant()).toString();
}
/*
bool WLFileScript::saveValue(QString namefile,QString nameData,QVariant data)
{
QMutexLocker locker(&mutex);

QVariant val;

QSettings save(namefile, QSettings::IniFormat);

save.setIniCodec("UTF-8");

save.setValue(nameData,data);

return save.isWritable();
}

QVariant WLFileScript::loadValue(QString namefile,QString nameData,QVariant dataDef)
{
QMutexLocker locker(&mutex);

QVariant val;

QSettings load(namefile, QSettings::IniFormat);

load.setIniCodec("UTF-8");

return load.value(nameData,dataDef);
}
*/
