#ifndef WLGSCTABLEMODEL_H
#define WLGSCTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "wlgcode.h"
#include "wlgdatatablemodel.h"

class WLGSCTableModel : public WLGDataTableModel
{
Q_OBJECT

public:
    explicit WLGSCTableModel(WLGCode *GCode,QObject *parent = nullptr);

public:
   void setHeaders(QString strheaders);

    // QAbstractItemModel interface
public:
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;

};

#endif // WLGSCTABLEMODEL_H
