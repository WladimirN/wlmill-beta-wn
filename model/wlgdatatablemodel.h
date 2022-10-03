#ifndef WLGDATATABLEMODEL_H
#define WLGDATATABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "wlgcode.h"
#include "wldatatablemodel.h"

class WLGDataTableModel : public WLDataTableModel
{
Q_OBJECT

public:
    explicit WLGDataTableModel(WLGCode *GCode,WLData *data,QObject *parent = nullptr);

protected:
    WLGCode *mGCode=nullptr;
    QList <int> selectList;

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const;

public slots:
    void setSelectList(QList<int>);
};



#endif // WLGDATATABLEMODEL_H
