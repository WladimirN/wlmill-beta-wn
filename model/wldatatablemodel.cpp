#include "wldatatablemodel.h"
#include <QColor>
#include <QFont>

WLDataTableModel::WLDataTableModel(WLData *data,QObject *parent):QAbstractTableModel (parent)
{
mdata=data;
defheaders=QStringList()<<"index,all";
headers=defheaders;
}

void WLDataTableModel::setHeaders(QStringList _headers)
{
if(_headers.isEmpty()){
  headers=defheaders;
  }
  else{
  headers=_headers;
  }

emit layoutChanged();
}

QStringList WLDataTableModel::getHeaders()
{
    return headers;
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
return mdata->count();
}

int WLDataTableModel::columnCount(const QModelIndex &parent) const
{
return headers.size();
}

QVariant WLDataTableModel::data(const QModelIndex &index, int role) const
{
if (!index.isValid()) return QVariant();

if (role == Qt::DisplayRole || role == Qt::EditRole) {    
  if(headers[index.column()]!="all")  {
     QVariant var=mdata->getValueAt(index.row(),headers[index.column()],"");
     double d;
     bool ok;
     d=var.toDouble(&ok);

     if(ok&&headers[index.column()]!="index")
        return QString::number(d,'f',3);
     else
        return mdata->getValueAt(index.row(),headers[index.column()],"").toString();
     }
     else {
     QStringList ret;
     WLEData edata=mdata->getDataAt(index.row());

     foreach(QString key,edata.keys())
       {
       if(headers.indexOf(key)==-1)
           ret+=key+":\""+edata.value(key).toString()+"\"";
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
   if(mdata->getValueAt(index.row(),headers[index.column()],"").toString().isEmpty()
    &&index.column()<(headers.size()-1))
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

int ikey = mdata->getKeyAt(index.row());

WLEData edata=mdata->getData(ikey);

bool   ok;
double dvalue=value.toString().replace(",",".").toDouble(&ok);

if(ok)
    edata.insert(headers[index.column()],dvalue);
else
    edata.insert(headers[index.column()],value);

mdata->setData(ikey,edata);
}

return true;
}

QVariant WLDataTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
if(role==Qt::DisplayRole) {
if(orientation==Qt::Vertical) { 
 }
 else {
 if(section>0)
 return headers.at(section);
 }

return QVariant();
}

return QAbstractItemModel::headerData(section,orientation,role);
}

Qt::ItemFlags WLDataTableModel::flags(const QModelIndex &index) const
{
if (!index.isValid()) return NULL;

return QAbstractTableModel::flags(index) | (0 < index.column() &&  index.column() < headers.size()-1 ? Qt::ItemIsEditable : Qt::NoItemFlags);
}


