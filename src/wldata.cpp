#include "wldata.h"

WLData::WLData()
{
}

bool WLData::setData(int index, WLEData data)
{
data.insert("index",index);
m_data.insert(index,data);
return true;
}

WLEData WLData::getData(int ikey)
{
return m_data.value(ikey);
}

WLEData WLData::getDataAt(int index)
{
return getData(getKeyAt(index));
}

WLEData WLData::takeData(int ikey)
{
return m_data.take(ikey);
}

void WLData::setValue(int ikey, QString key, QVariant value)
{
auto Data=getData(ikey);
Data.insert(key,value);
setData(ikey,Data);
}

QVariant WLData::getValue(int ikey, QString key, QVariant defvalue)
{
auto defData=getData(ikey);  //берем какой есть

defData.insert(key,defvalue); //записываем

return m_data.value(ikey,defData).value(key,defvalue); //читаем
}

QVariant WLData::getValueAt(int ikey, QString key, QVariant defvalue)
{
int indexkey=getKeyAt(ikey);

if(indexkey>0){
 return getValue(indexkey,key,defvalue);
 }

return defvalue;
}

int WLData::getKeyAt(int index) const
{
if(0<=index&&index<count()) {
 return  m_data.keys().at(index);
 }else {
 return -1;
 }
}

int WLData::count() const
{
return m_data.count();
}

bool WLData::readFromFile(QString filename, QString split)
{
QStringList headersList;
QFile file(filename);

if(file.open(QIODevice::ReadOnly)){

headersList=static_cast<QString>(QTextCodec::codecForName("Windows-1251")->toUnicode(file.readLine())).simplified().split(split);

m_data.clear();

while(!file.atEnd())    {

QStringList list=static_cast<QString>(QTextCodec::codecForName("Windows-1251")->toUnicode(file.readLine())).simplified().split(split);

WLEData Data;

if(list.size()==headersList.size())
    for(int i=0;i<headersList.size();i++){
     if(!list.at(i).isEmpty()){
       Data.insert(headersList.at(i),list.at(i));
       }
    }

m_data.insert(Data.value("index",m_data.count()).toInt(),Data);
}

file.close();

return true;
}

return false;
}


bool WLData::writeToFile(QString filename, QString split)
{
QStringList headersList=m_headers;

foreach(WLEData Data,m_data)
{
foreach(QString key,Data.keys())
  {
  if(headersList.indexOf(key)==-1)
      headersList.append(key);
  }
}

headersList.prepend("index");
headersList.removeDuplicates();

QFile file(filename);
QTextStream stream(&file);

stream.setCodec(QTextCodec::codecForName("Windows-1251"));

if(file.open(QIODevice::WriteOnly)){

stream<<headersList.join(split)<<"\r\n";

foreach(WLEData Data,m_data)
{
QStringList values;

foreach(QString key,headersList)
  {
  QVariant value=Data.value(key,"");
  bool ok;

  value.toString().replace(",",".").toDouble(&ok);

  if(ok)
    values<<value.toString().replace(",",".");
  else
    values<<Data.value(key,"").toString();
  }

stream<<values.join(split)<<"\r\n";;
}

file.close();
return true;
}

return false;
}

void WLData::setHeaders(QStringList headers)
{
m_headers=headers;
}

void WLData::clear()
{
m_data.clear();
}
