#ifndef WLGTOOLSTABLEMODEL_H
#define WLGTOOLSTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "wlgcode.h"
#include "wlgdatatablemodel.h"

class WLGToolsTableModel : public WLGDataTableModel
{
Q_OBJECT

public:
    explicit WLGToolsTableModel(WLGCode *GCode,QObject *parent = nullptr);

public:
    void setHeaders(QString strheaders);

    // QAbstractItemModel interface
public:
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
};

#endif // WLGTOOLSTABLEMODEL_H
