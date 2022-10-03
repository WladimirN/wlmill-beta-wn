#ifndef WLOSCPWIDGET_H
#define WLOSCPWIDGET_H

#include <QWidget>
#include "qcustomplot.h"
#include "wlmoduleoscp.h"
#include "wlioput.h"
#include "wlaxis.h"

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

    QPointer<QCPGraph> mGraph1;
    QPointer<QCPGraph> mGraph2;
    QPointer<QCPGraph> mGraph3;
    QPointer<QCPGraph> mGraph4;

    double lastTime=0;

private slots:
   void addData(double time,QList <double> values);
   void onPBRun(bool press);

   void horzScrollBarChanged(int value);
   void xAxisChanged(QCPRange range);
};

#endif // WLOSCPWIDGET_H
