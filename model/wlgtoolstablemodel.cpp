#include "wlgtoolstablemodel.h"
#include <QColor>
#include <QFont>

WLGToolsTableModel::WLGToolsTableModel(WLGCode *GCode,QObject *parent):WLDataTableModel(&GCode->data()->dataTools,parent)
{
mGCode=GCode;

connect(mGCode,&WLGCode::changedTool,[=](int number){
if(number<0)
  emit layoutChanged();
else
  emit dataChanged(index(number-1,0),index(number-1,2));
});
}


QVariant WLGToolsTableModel::data(const QModelIndex &index, int role) const
{
if (!index.isValid()) return QVariant();

if (role == Qt::DisplayRole || role == Qt::EditRole) {
     if(headers[index.column()]=="index")
      return QString("T%1").arg(mGCode->getDataTool()->getValueAt(index.row(),headers[index.column()],"").toInt());
     }

if (role == Qt::BackgroundRole){
   if(mGCode->data()->dataTools.getValueAt(index.row(),headers[index.column()],"").toString().isEmpty()
    &&index.column()<(headers.size()-1))
   return QColor(Qt::lightGray);
   }

if (role == Qt::BackgroundRole
  &&headers[index.column()]=="index"
  &&mGCode->getT()==mGCode->getDataTool()->getValueAt(index.row(),"index","-1").toInt()){
   return QColor(250,100,100);
   }

if (role == Qt::BackgroundRole
  && (headers[index.column()]=="Xg"
    ||headers[index.column()]=="Yg"
    ||headers[index.column()]=="Zg")
  &&mGCode->getOfstTool()==mGCode->getDataTool()->getValueAt(index.row(),"index","-1").toInt()){
   return QColor(100,200,100);
   }

return WLDataTableModel::data(index,role);
}

void WLGToolsTableModel::setHeaders(QStringList headers)
{
if(headers.isEmpty())
   headers=QString("index,Diam,D,H,Xg,Yg,Zg,all").split(",");

WLDataTableModel::setHeaders(headers);
}


