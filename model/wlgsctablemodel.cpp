#include "wlgsctablemodel.h"
#include <QColor>
#include <QFont>

WLGSCTableModel::WLGSCTableModel(WLGCode *GCode,QObject *parent)
              :WLGDataTableModel(GCode,&GCode->data()->dataSC,parent)
{
connect(mGCode,&WLGCode::changedSC,[=](int number){
  if(number<0){
    emit layoutChanged();
    }
    else {
    emit dataChanged(index(number-1,0),index(number-1,2));    
    }
});
}


QVariant WLGSCTableModel::data(const QModelIndex &index, int role) const
{
if (!index.isValid()) return QVariant();

if (role == Qt::BackgroundRole
   &&(m_headers[index.column()]=="index"||m_headers.at(index.column())=="GCode")){
    if(mGCode->getSC()==mGCode->getDataSC()->getValueAt(index.row(),"index","-1").toInt()){
      return QColor(250,100,100);
    }
    else{
     return QVariant();
    }
 }

return WLGDataTableModel::data(index,role);
}

void WLGSCTableModel::setHeaders(QString strheaders)
{
if(strheaders.isEmpty())
   strheaders=QString("MAC,GCode,X,Y,Z,all");

WLGDataTableModel::setHeaders(strheaders);
}

Qt::ItemFlags WLGSCTableModel::flags(const QModelIndex &index) const
{
Qt::ItemFlags ret=WLDataTableModel::flags(index);

if((ret|Qt::ItemIsEditable)
 &&(m_headers.at(index.column())=="GCode")){
  ret&=~Qt::ItemIsEditable;
  }

return ret;
}

