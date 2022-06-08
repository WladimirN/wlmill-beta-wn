#ifndef HEIGHTMAPTABLEMODEL_H
#define HEIGHTMAPTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include "wlheightmap.h"

class WLHeightMapTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit WLHeightMapTableModel(WLHeightMap *_HeightMap,QObject *parent = 0);

  //  void resize(int cols, int rows);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
   // bool insertRow(int row, const QModelIndex &parent = QModelIndex());
    //bool removeRow(int row, const QModelIndex &parent = QModelIndex());
    void clear();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

//signals:
//    void dataChangedByUserInput();

private:
//    QList<QList<double>> m_data;
    WLHeightMap *m_HeightMap=nullptr;
    QList<QString> m_headers;
};

#endif // HEIGHTMAPTABLEMODEL_H
