#include "wldatatablemodel.h"
#include <QColor>
#include <QFont>

WLDataTableModel::WLDataTableModel(WLData *data,QObject *parent):QAbstractTableModel (parent)
{
m_data=data;
m_defheaders=QStringList()<<"index,all";
m_headers=m_defheaders;
}

void WLDataTableModel::setHeaders(QString strheaders)
{
if(strheaders.isEmpty()){
  m_headers=m_defheaders;
  }
  else{
  m_headers=strheaders.simplified().remove(" ").split(",",QString::SkipEmptyParts);
  }

QStringList head=m_headers;

head.removeOne("all");

m_data->setHeaders(head);

emit layoutChanged();
}

QStringList WLDataTableModel::getHeaders()
{
    return m_headers;
}

void WLDataTableModel::setScaleFont(double scale)
{
if(scale>0)
  {
  m_scaleFont=scale;
  emit layoutChanged();
  }
}

double WLDataTableModel::getScaleFont() const
{
return m_scaleFont;
}

int WLDataTableModel::rowCount(const QModelIndex &parent) const
{
return m_data->count();
}

int WLDataTableModel::columnCount(const QModelIndex &parent) const
{
return m_headers.size();
}

QVariant WLDataTableModel::data(const QModelIndex &index, int role) const
{
if (!index.isValid()) return QVariant();

if (role == Qt::DisplayRole || role == Qt::EditRole) {    
  if(m_headers[index.column()]!="all")  {
     QVariant var=m_data->getValueAt(index.row(),m_headers[index.column()],"");
     double d;
     bool ok;
     d=var.toDouble(&ok);

     if(ok&&m_headers[index.column()]!="index")
        return QString::number(d,'f',3);
     else
        return m_data->getValueAt(index.row(),m_headers[index.column()],"").toString();
     }
     else {
     QStringList ret;
     WLEData edata=m_data->getDataAt(index.row());

     foreach(QString key,edata.keys())
       {
       if(m_headers.indexOf(key)==-1)
           ret+=key+":\""+edata.value(key).toString()+"\"";
       }

     return ret.join(" ");

     }
}

if (role == Qt::FontRole){
   QFont font;

   if(index.column()<(m_headers.size()-1))
      font.setPointSizeF(font.pointSize()*m_scaleFont);

   bool ok;

   data(index,Qt::EditRole).toDouble(&ok);
   font.setBold(ok);

   return font;
   }

if (role == Qt::BackgroundRole){
   if(m_data->getValueAt(index.row(),m_headers[index.column()],"").toString().isEmpty()
    &&index.column()<(m_headers.size()-1))
       return QColor(Qt::lightGray);
   }

if (role == Qt::TextAlignmentRole) {
    return Qt::AlignCenter;
}

return QVariant();
}

bool WLDataTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
if(!index.isValid()) return false;

if(role==Qt::EditRole){

int ikey = m_data->getKeyAt(index.row());

WLEData edata=m_data->getData(ikey);

bool   ok;
double dvalue=value.toString().replace(",",".").toDouble(&ok);

if(ok)
    edata.insert(m_headers[index.column()],dvalue);
else
    edata.insert(m_headers[index.column()],value);

m_data->setData(ikey,edata);
}

return true;
}

QVariant WLDataTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
if(role==Qt::DisplayRole) {

if(orientation==Qt::Vertical) { 
 }
 else {
 if(m_headers.at(section)!="index")
 return m_headers.at(section);
 }

return QVariant();
}

return QAbstractItemModel::headerData(section,orientation,role);
}

Qt::ItemFlags WLDataTableModel::flags(const QModelIndex &index) const
{
if (!index.isValid()) QAbstractTableModel::flags(index);

return QAbstractTableModel::flags(index) | ( m_headers.at(index.column())!="index"
                                           &&m_headers.at(index.column())!="all"   ? Qt::ItemIsEditable : Qt::NoItemFlags);

}


