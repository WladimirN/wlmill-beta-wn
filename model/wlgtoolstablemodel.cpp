#include "wlgtoolstablemodel.h"
#include <QColor>
#include <QFont>

WLGToolsTableModel::WLGToolsTableModel(WLGCode *_GCode,QObject *parent):QAbstractTableModel (parent)
{
GCode=_GCode;

defheaders=QStringList()<<"index"<<"D"<<"H"<<"all";
headers=defheaders;

connect(GCode,&WLGCode::changedTool,[=](int number){
emit dataChanged(index(number-1,0),index(number-1,2));
});

connect(GCode,&WLGCode::changedTools,[=](){
emit layoutChanged();
});
}

void WLGToolsTableModel::setHeaders(QStringList _headers)
{
if(_headers.isEmpty())
  headers=defheaders;
else
  {
  headers=_headers;

 if(headers.indexOf("all")==-1)
          headers<<"all";
  }

emit layoutChanged();
}

QStringList WLGToolsTableModel::getHeaders()
{
    return headers;
}

void WLGToolsTableModel::setScaleFont(double scale)
{
if(scale>0)
  {
  m_scaleFont=scale;
  emit layoutChanged();
  }
}

double WLGToolsTableModel::getScaleFont() const
{
return m_scaleFont;
}

int WLGToolsTableModel::rowCount(const QModelIndex &parent) const
{
return GCode->data()->Tools.count();
}

int WLGToolsTableModel::columnCount(const QModelIndex &parent) const
{
return headers.size();
}

QVariant WLGToolsTableModel::data(const QModelIndex &index, int role) const
{
if (!index.isValid()) return QVariant();

if (role == Qt::DisplayRole || role == Qt::EditRole) {    
  if(headers[index.column()]!="all")  {
     return GCode->getTools()->getValueAt(index.row(),headers[index.column()],"").toString();
     }
     else {
     QStringList ret;
     WLGTool Tool=GCode->getTools()->getToolAt(index.row());

     foreach(QString key,Tool.keys())
       {
       if(headers.indexOf(key)==-1)
           ret+=key+"=\""+Tool.value(key).toString()+"\"";
       }

     return ret.join(" ");

     }
}

if (role == Qt::FontRole){
   QFont font;

   if(index.column()<(headers.size()-1))
      font.setPointSizeF(font.pointSize()*m_scaleFont);

   bool ok;

   data(index,Qt::EditRole).toDouble(&ok);
   font.setBold(ok);

   return font;
   }


if (role == Qt::BackgroundRole){
   if(GCode->data()->Tools.getValueAt(index.row(),headers[index.column()],"").toString().isEmpty()
    &&index.column()<(headers.size()-1))
       return QColor(Qt::lightGray);
   }

if (role == Qt::BackgroundRole
  &&headers[index.column()]=="index"
  &&GCode->getT()==GCode->getTools()->getValueAt(index.row(),"index","-1").toInt()){
   return QColor(250,100,100);
   }

if (role == Qt::BackgroundRole
  && (headers[index.column()]=="Xg"
    ||headers[index.column()]=="Yg"
    ||headers[index.column()]=="Zg")
  &&GCode->getOfstTool()==GCode->getTools()->getValueAt(index.row(),"index","-1").toInt()){
   return QColor(100,200,100);
   }

if (role == Qt::TextAlignmentRole) {
    return Qt::AlignCenter;
}

return QVariant();
}

bool WLGToolsTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
if(!index.isValid()) return false;

if(role==Qt::EditRole){

int ikey = GCode->getTools()->getKeyAt(index.row());

WLGTool Tool=GCode->getTool(ikey);

bool   ok;
double dvalue=value.toString().replace(",",".").toDouble(&ok);

if(ok)
    Tool.insert(headers[index.column()],dvalue);
else
    Tool.insert(headers[index.column()],value);

GCode->setTool(ikey,Tool);;
}

return true;
}

QVariant WLGToolsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
if(role==Qt::DisplayRole) {
if(orientation==Qt::Vertical)
 {
 return QVariant();
 }
 else return headers.at(section);
}

return QAbstractItemModel::headerData(section,orientation,role);
}

Qt::ItemFlags WLGToolsTableModel::flags(const QModelIndex &index) const
{
if (!index.isValid()) return NULL;

return QAbstractTableModel::flags(index) | (0 < index.column() &&  index.column() < headers.size()-1 ? Qt::ItemIsEditable : Qt::NoItemFlags);
}


