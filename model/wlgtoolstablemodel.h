#ifndef WLGTOOLSTABLEMODEL_H
#define WLGTOOLSTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "wlgcode.h"
#include "wldatatablemodel.h"

class WLGToolsTableModel : public WLDataTableModel
{
Q_OBJECT

public:
    explicit WLGToolsTableModel(WLGCode *GCode,QObject *parent = nullptr);

public:
     void setHeaders(QStringList m_headers);

private:
    WLGCode *mGCode=nullptr;

    // QAbstractItemModel interface
public:
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
};

#endif // WLGTOOLSTABLEMODEL_H
