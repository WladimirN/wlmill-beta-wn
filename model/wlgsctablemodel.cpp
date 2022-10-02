#include "wlgsctablemodel.h"
#include <QColor>
#include <QFont>

WLGSCTableModel::WLGSCTableModel(WLGCode *GCode,QObject *parent):WLDataTableModel(&GCode->data()->dataSC,parent)
{
mGCode=GCode;

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
    else {
     return QVariant();
    }
  }

return WLDataTableModel::data(index,role);
}

void WLGSCTableModel::setHeaders(QStringList headers)
{
if(headers.isEmpty())
   headers=QString("MAC,GCode,X,Y,Z,all").split(",");

WLDataTableModel::setHeaders(headers);
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

