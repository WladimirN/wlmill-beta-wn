#include "wlgsctablemodel.h"
#include <QColor>
#include <QFont>

WLGSCTableModel::WLGSCTableModel(WLGCode *GCode,QObject *parent):WLDataTableModel(&GCode->data()->dataSC,parent)
{
mGCode=GCode;

connect(mGCode,&WLGCode::changedSC,[=](int number){
  if(number<0)
    emit layoutChanged();
  else
    emit dataChanged(index(number-1,0),index(number-1,2));
});
}


QVariant WLGSCTableModel::data(const QModelIndex &index, int role) const
{
if (role == Qt::BackgroundRole){
   if(mGCode->data()->dataTools.getValueAt(index.row(),headers[index.column()],"").toString().isEmpty()
    &&index.column()<(headers.size()-1))
   return QColor(Qt::lightGray);
   }

if (role == Qt::BackgroundRole
  &&headers[index.column()]=="index"
  &&mGCode->getSC()==mGCode->getDataSC()->getValueAt(index.row(),"index","-1").toInt()){
   return QColor(250,100,100);
   }

return WLDataTableModel::data(index,role);
}

void WLGSCTableModel::setHeaders(QStringList headers)
{
if(headers.isEmpty())
   headers=QString("index,X,Y,Z,all").split(",");

WLDataTableModel::setHeaders(headers);
}


