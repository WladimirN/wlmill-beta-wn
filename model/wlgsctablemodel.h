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
    QVariant data(const QModelIndex &index, int role) const;

    void setHeaders(QStringList headers);

private:
    WLGCode *mGCode=nullptr;

};

#endif // WLGSCTABLEMODEL_H
