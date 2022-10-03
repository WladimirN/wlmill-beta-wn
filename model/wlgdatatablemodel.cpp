#include "wlgdatatablemodel.h"
#include <QColor>
#include <QFont>

WLGDataTableModel::WLGDataTableModel(WLGCode *GCode,WLData *data,QObject *parent):WLDataTableModel(data,parent)
{
mGCode=GCode;
}


QVariant WLGDataTableModel::data(const QModelIndex &index, int role) const
{
if (role == Qt::BackgroundRole){
  if(m_headers[index.column()]=="index"||m_headers.at(index.column())=="GCode"){
    if(mGCode->getSC()==mGCode->getDataSC()->getValueAt(index.row(),"index","-1").toInt()){
      return QColor(250,100,100);
    }
    else{
     return QVariant();
    }
  }
  else if(selectList.indexOf(m_data->getKeyAt(index.row()))!=-1){
         return QColor(100,250,100);
         }
}

return WLDataTableModel::data(index,role);
}

void WLGDataTableModel::setSelectList(QList<int> newList)
{
QList <int> lastList=selectList;

selectList.clear();

foreach(int number,lastList){
number=m_data->getKeyAt(number);
emit dataChanged(index(number-1,0),index(number-1,2));
}

selectList=newList;

foreach(int number,selectList){
number=m_data->getKeyAt(number);
emit dataChanged(index(number-1,0),index(number-1,2));
}

}


