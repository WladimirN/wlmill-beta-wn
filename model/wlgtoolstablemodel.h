#ifndef WLGTOOLSTABLEMODEL_H
#define WLGTOOLSTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "wlgcode.h"

class WLGToolsTableModel : public QAbstractTableModel
{
Q_OBJECT

public:
    explicit WLGToolsTableModel(WLGCode *GCode,QObject *parent = nullptr);

    Q_INVOKABLE void setHeaders(QStringList);
    QStringList getHeaders();

      void setScaleFont(double scale);
    double getScaleFont() const;

public:
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;


private:
    WLGCode *GCode=nullptr;
 QStringList    headers;
 QStringList defheaders;
 double     m_scaleFont=1.5;
};

#endif // WLGTOOLSTABLEMODEL_H
