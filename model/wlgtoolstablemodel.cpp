#include "wlgtoolstablemodel.h"
#include <QColor>
#include <QFont>

WLGToolsTableModel::WLGToolsTableModel(WLGCode *GCode,QObject *parent)
                    :WLGDataTableModel(GCode,&GCode->data()->dataTools,parent)
{
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
  && (m_headers[index.column()]=="Xo"
    ||m_headers[index.column()]=="Yo"
    ||m_headers[index.column()]=="Zo")
  &&mGCode->getOfstTool()==mGCode->getDataTool()->getValueAt(index.row(),"index","-1").toInt()){
   return QColor(100,200,100);
   }

return WLGDataTableModel::data(index,role);
}

void WLGToolsTableModel::setHeaders(QString strheaders)
{
if(strheaders.isEmpty())
   strheaders=QString("GCode,Diam,D,H,Xo,Yo,Zo,all");

WLGDataTableModel::setHeaders(strheaders);

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


