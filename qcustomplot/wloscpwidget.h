#ifndef WLOSCPWIDGET_H
#define WLOSCPWIDGET_H

#include <QWidget>
#include "qcustomplot.h"
#include "wlmoduleoscp.h"
#include "wlioput.h"
#include "wlaxis.h"
#include "wlchoscpwidget.h"

namespace Ui {
class WLOscpWidget;
}

class WLOscpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WLOscpWidget(WLModuleOscp *MOscp,QWidget *parent = nullptr);
    ~WLOscpWidget();

private:
    Ui::WLOscpWidget *ui;

    WLModuleOscp *mMOscp;

    QList <QPointer<QCPGraph>> mGraphs;
    QList <WLChOscpWidget*> chWidgets;

    double lastTime=0;    
    double period=.100;

    QButtonGroup *selectChannel;

private:

   void initSelectorChannel();

private slots:
   void addData(double time,QList <double> values);
   void onPBRun(bool press);

   void horzScrollBarChanged(int value);
   void xAxisChanged(QCPRange range);
};

#endif // WLOSCPWIDGET_H
