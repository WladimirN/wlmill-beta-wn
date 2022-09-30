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
    QVariant data(const QModelIndex &index, int role) const;

     void setHeaders(QStringList headers);
private:
    WLGCode *mGCode=nullptr;

};

#endif // WLGTOOLSTABLEMODEL_H
