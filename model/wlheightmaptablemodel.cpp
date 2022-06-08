#include "wlheightmaptablemodel.h"

#include <QtDebug>

WLHeightMapTableModel::WLHeightMapTableModel(WLHeightMap *_HeightMap,QObject *parent) : QAbstractTableModel(parent)
{
m_HeightMap=_HeightMap;
connect(m_HeightMap,&WLHeightMap::changed,this,[=](){
  beginResetModel();
  endResetModel();}
  );
}


/*
void HeightMapTableModel::resize(int cols, int rows)
{
    foreach (QVector<double> row, m_data) row.clear();

    m_data.clear();

    for (int i = 0; i < rows; i++) {
        QVector<double> row;
        for (int j = 0; j < cols; j++) {
            row.append(qQNaN());
        }
        m_data.append(row);
    }
}
*/
QVariant WLHeightMapTableModel::data(const QModelIndex &index, int role) const
{
 if (!index.isValid()) return QVariant();
/*
 if (index.row() >= m_data.count() || index.column() >= m_data[0].count()) return QVariant();
*/
 if (role == Qt::DisplayRole || role == Qt::EditRole) {
     return QString::number(m_HeightMap->getData().map[index.column()][m_HeightMap->countY()-1-index.row()]);
 }
/*
 if (role == Qt::UserRole) {
     return m_data[index.row()][index.column()];
 }*/

 if (role == Qt::TextAlignmentRole) {
     return Qt::AlignCenter;
 }

 return QVariant();
}

bool WLHeightMapTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
 m_HeightMap->setValue(index.column(),m_HeightMap->countY()-index.row()-1,value.toString().replace(",",".").toDouble());

 //if (role == Qt::EditRole) emit dataChangedByUserInput();

 return true;
}
/*
bool HeightMapTableModel::insertRow(int row, const QModelIndex &parent)
{
    Q_UNUSED(parent)

    m_data.insert(row, QVector<double>());
    return true;
}

bool HeightMapTableModel::removeRow(int row, const QModelIndex &parent)
{
    Q_UNUSED(parent)

    m_data.remove(row);
    return true;
}

void HeightMapTableModel::clear()
{
    m_data.clear();
}*/

int WLHeightMapTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_HeightMap->countY();
}

int WLHeightMapTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_HeightMap->countX();
}

QVariant WLHeightMapTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();

    return QString::number(section);

    if(orientation==Qt::Horizontal) {
     return QString::number(m_HeightMap->getP0().x()
                          +(m_HeightMap->getP1().x()-m_HeightMap->getP0().x())/(m_HeightMap->countX()-1)*section);
     }
    else {
    return QString::number(m_HeightMap->getP0().y()
                         +(m_HeightMap->getP1().y()-m_HeightMap->getP0().y())/(m_HeightMap->countY()-1)*(m_HeightMap->countY()-1-section));

    }

}

Qt::ItemFlags WLHeightMapTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return NULL;
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

