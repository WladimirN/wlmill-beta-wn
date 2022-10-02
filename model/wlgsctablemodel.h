#ifndef WLGSCTABLEMODEL_H
#define WLGSCTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "wlgcode.h"
#include "wldatatablemodel.h"

class WLGSCTableModel : public WLDataTableModel
{
Q_OBJECT

public:
    explicit WLGSCTableModel(WLGCode *GCode,QObject *parent = nullptr);

public:
   void setHeaders(QStringList m_headers);

private:
    WLGCode *mGCode=nullptr;


    // QAbstractItemModel interface
public:
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
};

#endif // WLGSCTABLEMODEL_H
