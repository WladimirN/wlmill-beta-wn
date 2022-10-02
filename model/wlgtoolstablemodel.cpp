#include "wlgtoolstablemodel.h"
#include <QColor>
#include <QFont>

WLGToolsTableModel::WLGToolsTableModel(WLGCode *GCode,QObject *parent):WLDataTableModel(&GCode->data()->dataTools,parent)
{
mGCode=GCode;

connect(mGCode,&WLGCode::changedTool,[=](int number){
if(number<0) {
  emit layoutChanged();
  }
  else{
  emit dataChanged(index(number-1,0),index(number-1,2));
  }
});
}


QVariant WLGToolsTableModel::data(const QModelIndex &index, int role) const
{
if (!index.isValid()) return QVariant();

if (role == Qt::BackgroundRole
   &&(m_headers[index.column()]=="index"||m_headers.at(index.column())=="GCode")){
    if(mGCode->getT()==mGCode->getDataTool()->getValueAt(index.row(),"index","-1").toInt()){
      return QColor(250,100,100);
      }
      else {
      return QVariant();
      }
  }

if (role == Qt::BackgroundRole
  && (m_headers[index.column()]=="Xg"
    ||m_headers[index.column()]=="Yg"
    ||m_headers[index.column()]=="Zg")
  &&mGCode->getOfstTool()==mGCode->getDataTool()->getValueAt(index.row(),"index","-1").toInt()){
   return QColor(100,200,100);
   }

return WLDataTableModel::data(index,role);
}

void WLGToolsTableModel::setHeaders(QStringList headers)
{
if(headers.isEmpty())
   headers=QString("GCode,Diam,D,H,Xg,Yg,Zg,all").split(",");

WLDataTableModel::setHeaders(headers);
}

Qt::ItemFlags WLGToolsTableModel::flags(const QModelIndex &index) const
{
Qt::ItemFlags ret=WLDataTableModel::flags(index);

if((ret|Qt::ItemIsEditable)
 &&(m_headers.at(index.column())=="GCode")){
  ret&=~Qt::ItemIsEditable;
  }

return ret;
}


