#include "wlgtool.h"

WLGTools::WLGTools()
{
m_keyList=QString("N,H,D,Diam").split(",");
}

bool WLGTools::setTool(int index, QMap<QString, QVariant> data)
{
  data.insert("index",index);
m_tools.insert(index,data);
return true;
}

WLGTool WLGTools::getTool(int ikey)
{
return m_tools.value(ikey);
}

WLGTool WLGTools::getToolAt(int index)
{
return getTool(getKeyAt(index));
}

WLGTool WLGTools::takeTool(int ikey)
{
return m_tools.take(ikey);
}

void WLGTools::setValue(int ikey, QString key, QVariant value)
{
auto Tool=getTool(ikey);
Tool.insert(key,value);
setTool(ikey,Tool);
}

QVariant WLGTools::getValue(int ikey, QString key, QVariant defvalue)
{
auto defTool=getTool(ikey);  //берем какой есть

defTool.insert(key,defvalue); //записываем

return m_tools.value(ikey,defTool).value(key,defvalue); //читаем
}

QVariant WLGTools::getValueAt(int ikey, QString key, QVariant defvalue)
{
int indexkey=getKeyAt(ikey);

if(indexkey>0){
 return getValue(indexkey,key,defvalue);
 }

return defvalue;
}

int WLGTools::getKeyAt(int index) const
{
return index<count() ? m_tools.keys().at(index) : -1;
}

int WLGTools::count() const
{
    return m_tools.count();
}

bool WLGTools::readFromFile(QString filename, QString split)
{
QStringList headersList;
QFile file(filename);

if(file.open(QIODevice::ReadOnly)){


headersList=static_cast<QString>(file.readLine()).simplified().split(split);

m_tools.clear();

while(!file.atEnd())    {
QStringList list=static_cast<QString>(file.readLine()).simplified().split(split);

WLGTool Tool;

if(list.size()==headersList.size())
    for(int i=0;i<headersList.size();i++){
     if(!list.at(i).isEmpty()){
       Tool.insert(headersList.at(i),list.at(i));
       }
    }

m_tools.insert(Tool.value("index").toInt(),Tool);
}

file.close();

return true;
}

return false;
}


bool WLGTools::writeToFile(QString filename, QString split)
{
QStringList headersList;

foreach(WLGTool Tool,m_tools)
{
foreach(QString key,Tool.keys())
  {
  if(headersList.indexOf(key)==-1)
      headersList.append(key);
  }
}

headersList.removeOne("index");
headersList.prepend("index");

QFile file(filename);
QTextStream stream(&file);

if(file.open(QIODevice::WriteOnly)){

stream<<headersList.join(split)<<"\r\n";

foreach(WLGTool Tool,m_tools)
{
QStringList values;

foreach(QString key,headersList)
  {
  QVariant value=Tool.value(key,"");
  bool ok;

  value.toString().replace(",",".").toDouble(&ok);

  if(ok)
    values<<value.toString().replace(",",".");
  else
    values<<Tool.value(key,"").toString();
  }

stream<<values.join(split)<<"\r\n";;
}

file.close();
return true;
}

return false;
}

