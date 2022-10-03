#ifndef WLDATATABLEMODEL_H
#define WLDATATABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include "wldata.h"

class WLDataTableModel : public QAbstractTableModel
{
Q_OBJECT

public:
    explicit WLDataTableModel(WLData *data,QObject *parent = nullptr);

    QStringList getHeaders();

      void setScaleFont(double scale);
    double getScaleFont() const;

public:
virtual  Q_INVOKABLE void setHeaders(QStringList);

public:
    WLData const *getData(){return  m_data;}

public:
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

protected:
    QStringList m_headers;
    QStringList m_defheaders;
        WLData *m_data;

private:
    double  m_scaleFont=1.5;
};

#endif // WLDATATABLEMODE_H
