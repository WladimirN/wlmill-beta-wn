#ifndef WLSPINDLEWIDGET_H
#define WLSPINDLEWIDGET_H

#include <QDialog>

#include "wlspindle.h"

namespace Ui {
class WLSpindleWidget;
}

class WLSpindleWidget : public QDialog
{
    Q_OBJECT

public:
    explicit WLSpindleWidget(WLSpindle *_Spindle,QWidget *parent = nullptr);
    ~WLSpindleWidget();

private:
    Ui::WLSpindleWidget *ui;

private:
    WLSpindle *Spindle;

private:
   void initTableCalcSout();
   QList <WLSpindleData> getSpindleDataList();

   // QDialog interface
public slots:
   void accept();
};

#endif // WLSPINDLEWIDGET_H
