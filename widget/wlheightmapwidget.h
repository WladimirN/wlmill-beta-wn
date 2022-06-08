#ifndef WLHEIGHTMAPWIDGET_H
#define WLHEIGHTMAPWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QAbstractTableModel>

#include "wlheightmaptablemodel.h"
#include "wlheightmap.h"


namespace Ui {
class WLHeightMapWidget;
}

class WLHeightMapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WLHeightMapWidget(WLHeightMap *_heightMap,QWidget *parent = nullptr);
    ~WLHeightMapWidget();


private slots:
   void on_pbLoad_clicked();

   void on_cbEnable_stateChanged(int arg1);

   void on_cbShow_stateChanged(int arg1);

   void on_cbShowGrid_stateChanged(int arg1);

   void on_sbStepInter_valueChanged(double arg1);

private:
    Ui::WLHeightMapWidget *ui;

    WLHeightMap *m_heightMap;
    WLHeightMapTableModel *m_heightMapTModel;
};

#endif // WLHEIGHTMAPWIDGET_H
