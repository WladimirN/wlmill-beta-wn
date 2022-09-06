#include "wlfile.h"

WLFile::WLFile(QObject *parent)
	: QObject(parent)
{

}

WLFile::~WLFile()
{

}

bool WLFile::createFile(QString namefile)
{
QFile File(namefile);
if(File.open(QIODevice::WriteOnly))
 {
 File.close();
 return true;
 }
return false;
}

bool WLFile::write(QString namefile,QString str)
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

bool WLFile::saveValue(QString namefile,QString nameData,QVariant data)
{
QMutexLocker locker(&mutex);

QVariant val;

QSettings save(namefile, QSettings::IniFormat);

save.setIniCodec("UTF-8");

save.setValue(nameData,data);

return save.isWritable();
}

QVariant WLFile::loadValue(QString namefile,QString nameData,QVariant dataDef)
{
QMutexLocker locker(&mutex);

QVariant val;

QSettings load(namefile, QSettings::IniFormat);

load.setIniCodec("UTF-8");

return load.value(nameData,dataDef);
}

